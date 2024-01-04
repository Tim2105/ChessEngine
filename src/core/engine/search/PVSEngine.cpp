#include "core/engine/search/PVSEngine.h"
#include "core/chess/Referee.h"

#include <math.h>

int16_t PVSEngine::pvs(int16_t depth, uint16_t ply, int16_t alpha, int16_t beta, bool allowNullMove, bool isPVNode) {
    if(nodesSearched % NODES_PER_CHECKUP == 0 && isCheckupTime())
        checkup();

    if(stopFlag)
        return 0;

    nodesSearched++;

    uint16_t repetitionCount = boardCopy.repetitionCount();
    if(repetitionCount >= 3 || boardCopy.getFiftyMoveCounter() >= 100)
        return DRAW_SCORE;

    TranspositionTableEntry entry;
    if(!isPVNode && transpositionTable.probe(boardCopy.getHashValue(), entry)) {
        if(entry.depth >= depth) {
            if(entry.type == TT_TYPE_EXACT) {
                return entry.score;
            } else if(entry.type == TT_TYPE_CUT_NODE) {
                if(entry.score >= beta)
                    return entry.score;
                else
                    alpha = std::max(alpha, entry.score);
            }
        }
    }

    if(depth <= 0 || ply >= (maxDepthReached + 1) * 4)
        return quiescence(ply, alpha, beta);

    if(allowNullMove && !boardCopy.isCheck() &&
       depth > ONE_PLY && !deactivateNullMove()) {
        
        Move nullMove = Move::nullMove();
        boardCopy.makeMove(nullMove);

        int16_t score = -pvs(depth - calculateNullMoveReduction(depth), ply + 1, -alpha - 1, -alpha, false, false);

        boardCopy.undoMove();

        if(stopFlag)
            return 0;

        if(score >= beta)
            return score;

        if(score > alpha)
            alpha = score;

        int16_t staticEvaluation = evaluator.evaluate();

        if(score < staticEvaluation - NULL_MOVE_THREAT_MARGIN)
            depth += ONE_THIRD_PLY;
    }

    clearPVTable(ply + 1);
    clearMoveStack(ply);

    int16_t bestScore = MIN_SCORE, moveCount = 0;
    Move move, bestMove;
    bool isCheckEvasion = boardCopy.isCheck();

    while((move = selectNextMove(ply)).exists()) {
        evaluator.updateBeforeMove(move);
        boardCopy.makeMove(move);
        evaluator.updateAfterMove();

        int16_t extension = determineExtension(isCheckEvasion);
        int16_t reduction = 0;
        if(extension == 0)
            reduction = determineReduction(moveCount + 1);

        int16_t score;

        if(moveCount == 0) {
            score = -pvs(depth - ONE_PLY + extension, ply + 1, -beta, -alpha, !isPVNode, isPVNode);
        } else {
            score = -pvs(depth - ONE_PLY + extension - reduction, ply + 1, -alpha - 1, -alpha, true, false);

            if(score > alpha && (reduction > 0 || isPVNode))
                score = -pvs(depth - ONE_PLY + extension, ply + 1, -beta, -alpha, !isPVNode, isPVNode);
        }

        // Skaliere die Bewertung in Richtung 0,
        // wenn der Zug einmal wiederholt wurde.
        if(repetitionCount >= 2)
            score /= 2;

        // Skaliere die Bewertung in Richtung 0,
        // wenn sich der Fünfzig-Züge-Zähler erhöht.
        // (Erst nach 10 Zügen, da sonst die Bewertung
        // zu früh skaliert wird.)
        int32_t fiftyMoveCounter = boardCopy.getFiftyMoveCounter();
        if(fiftyMoveCounter > 20)
            score = (int32_t)score * (100 - fiftyMoveCounter) / 80;

        evaluator.updateBeforeUndo();
        boardCopy.undoMove();
        evaluator.updateAfterUndo(move);

        if(stopFlag)
            return 0;

        if(score >= beta) {
            TranspositionTableEntry entry {
                board->getPly(),
                depth,
                score,
                TT_TYPE_CUT_NODE,
                move
            };

            transpositionTable.put(boardCopy.getHashValue(), entry);

            if(!move.isCapture()) {
                addKillerMove(ply, move);
                incrementHistoryScore(move, depth);
            }

            return score;
        }

        decrementHistoryScore(move, depth);

        if(score > bestScore) {
            bestScore = score;
            bestMove = move;

            if(score > alpha) {
                alpha = score;
                addPVMove(ply, move);

                if(ply == 0)
                    collectPVLine(score);
            }
        }

        moveCount++;
    }

    // Es gab keine legalen Züge, das Spiel ist vorbei.
    if(moveCount == 0) {
        clearPVTable(ply);

        if(boardCopy.isCheck())
            return -MATE_SCORE + ply; // Matt
        else
            return DRAW_SCORE; // Remis
    }

    entry = {
        board->getPly(),
        depth,
        bestScore,
        TT_TYPE_EXACT,
        bestMove
    };

    transpositionTable.put(boardCopy.getHashValue(), entry);

    incrementHistoryScore(bestMove, depth);

    return bestScore;
}

int16_t PVSEngine::quiescence(int16_t ply, int16_t alpha, int16_t beta) {
    nodesSearched++;

    if(boardCopy.repetitionCount() >= 3 || boardCopy.getFiftyMoveCounter() >= 100)
        return DRAW_SCORE;

    int16_t standPat = evaluator.evaluate();

    if(standPat >= beta)
        return beta;

    // Delta Pruning
    if(standPat < alpha - DELTA_MARGIN)
        return standPat;

    if(standPat > alpha)
        alpha = standPat;

    int16_t bestScore = MIN_SCORE;
    int16_t minMoveScore = MIN_SCORE;

    if(!boardCopy.isCheck()) {
        bestScore = standPat;
        minMoveScore = NEUTRAL_SCORE;
    }

    clearMoveStack(ply);

    int16_t moveCount = 0;
    Move move;

    while((move = selectNextMoveInQuiescence(ply, minMoveScore)).exists()) {
        evaluator.updateBeforeMove(move);
        boardCopy.makeMove(move);
        evaluator.updateAfterMove();

        int16_t score = -quiescence(ply + 1, -beta, -alpha);

        evaluator.updateBeforeUndo();
        boardCopy.undoMove();
        evaluator.updateAfterUndo(move);

        if(score >= beta)
            return beta;

        if(score > bestScore) {
            bestScore = score;

            if(score > alpha)
                alpha = score;
        }

        moveCount++;
    }

    // Der König steht im Schach und es gibt keine legalen Züge,
    // das Spiel ist vorbei.
    if(moveCount == 0 && boardCopy.isCheck())
        return -MATE_SCORE + ply; // Matt

    return bestScore;
}

void PVSEngine::collectPVLine(int16_t score) {
    variations.clear();

    std::vector<Move> variation;
    for(Move move : getPVLine())
        variation.push_back(move);

    variations.push_back(Variation(variation, score));
}

int16_t PVSEngine::determineExtension(bool isCheckEvasion) {
    int16_t extension = 0;

    // Erweiterung, wenn ein Spieler im Schach steht
    // oder aus dem Schach zieht.
    if(boardCopy.isCheck() || isCheckEvasion)
        extension += ONE_HALF_PLY;

    // Erweiterung, wenn eine Figur zurückgeschlagen wird,
    // ein Bauer promoviert oder ein Freibauer gezogen wird.
    Move lastMove = boardCopy.getLastMove();
    std::vector<MoveHistoryEntry> moveHistory = boardCopy.getMoveHistory();
    Move secondLastMove = moveHistory.size() > 1 ? moveHistory[moveHistory.size() - 2].move : Move::nullMove();
    if(lastMove.isCapture() && secondLastMove.exists() && secondLastMove.isCapture())
        extension += ONE_THIRD_PLY;
    else if(lastMove.isPromotion())
        extension += ONE_THIRD_PLY;
    else {
        int32_t movedPieceType = TYPEOF(boardCopy.pieceAt(lastMove.getDestination()));
        if(movedPieceType == PAWN) {
            int32_t side = boardCopy.getSideToMove() ^ COLOR_MASK;
            int32_t otherSide = side ^ COLOR_MASK;
            if(!(sentryMasks[side / COLOR_MASK][lastMove.getDestination()]
                & boardCopy.getPieceBitboard(otherSide | PAWN)))
                extension += ONE_THIRD_PLY;
        }
    }

    return extension;
}

int16_t PVSEngine::determineReduction(int16_t moveCount) {
    UNUSED(moveCount);

    int32_t reduction = ONE_PLY;

    Move lastMove = boardCopy.getLastMove();
    int32_t historyScore = getHistoryScore(lastMove, boardCopy.getSideToMove() ^ COLOR_MASK);
    reduction -= historyScore * ONE_PLY / 16384;

    if(lastMove.isCapture())
        reduction -= ONE_PLY;

    return std::clamp(reduction, 0, (maxDepthReached + 1) * ONE_PLY / 2);
}

bool PVSEngine::deactivateNullMove() {
    // Deaktiviere den Null-Zug, wenn der Spieler nur noch
    // eine Leichtfigur oder weniger hat.
    int32_t side = boardCopy.getSideToMove();

    if(boardCopy.getPieceBitboard(side | QUEEN).getNumberOfSetBits() > 0)
        return false;

    if(boardCopy.getPieceBitboard(side | ROOK).getNumberOfSetBits() > 0)
        return false;

    if(boardCopy.getPieceBitboard(side | BISHOP).getNumberOfSetBits() +
       boardCopy.getPieceBitboard(side | KNIGHT).getNumberOfSetBits() > 1)
        return false;

    return true;
}

void PVSEngine::scoreMoves(const Array<Move, 256>& moves, uint16_t ply) {
    for(Move move : moves) {
        int16_t score = 0;

        if(move.isCapture())
            score = std::max(evaluator.evaluateMoveSEE(move), (int16_t)-(KILLER_MOVE_SCORE - 1)) + KILLER_MOVE_SCORE - 1;
        else{
            if(isKillerMove(ply, move))
                score += KILLER_MOVE_SCORE;

            score += std::clamp(getHistoryScore(move),
                                MIN_SCORE + 1,
                                KILLER_MOVE_SCORE - 1);
        }

        moveStack[ply].moveScorePairs.push_back(MoveScorePair(move, score));
    }
}

void PVSEngine::scoreMovesForQuiescence(const Array<Move, 256>& moves, uint16_t ply) {
    for(Move move : moves) {
        int16_t score = evaluator.evaluateMoveSEE(move);
        moveStack[ply].moveScorePairs.push_back(MoveScorePair(move, score));
    }
}

Move PVSEngine::selectNextMove(uint16_t ply) {
    if(moveStack[ply].moveScorePairs.size() == 0) {
        if(!moveStack[ply].hashMove.exists()) {
            Move hashMove = Move::nullMove();
            TranspositionTableEntry entry;
            if(transpositionTable.probe(boardCopy.getHashValue(), entry))
                hashMove = entry.hashMove;

            if(hashMove.exists()) {
                moveStack[ply].hashMove = hashMove;
                return hashMove;
            }
        }

        Array<Move, 256> moves;
        boardCopy.generateLegalMoves(moves);
        scoreMoves(moves, ply);

        if(moveStack[ply].hashMove.exists()) {
            for(MoveScorePair& pair : moveStack[ply].moveScorePairs)
                if(pair.move == moveStack[ply].hashMove) {
                    // Der Hashzug wurde bereits bewertet, daher wird er
                    // nicht noch einmal zurückgegeben.
                    pair.score = MIN_SCORE;
                    break;
                }
        }
    }

    Move bestMove = Move::nullMove();
    int16_t bestScore = MIN_SCORE;
    size_t bestIndex = 0;

    for(size_t i = 0; i < moveStack[ply].moveScorePairs.size(); i++)
        if(moveStack[ply].moveScorePairs[i].score > bestScore) {
            bestScore = moveStack[ply].moveScorePairs[i].score;
            bestMove = moveStack[ply].moveScorePairs[i].move;
            bestIndex = i;
        }

    moveStack[ply].moveScorePairs[bestIndex].score = MIN_SCORE;

    return bestMove;
}

Move PVSEngine::selectNextMoveInQuiescence(uint16_t ply, int16_t minScore) {
    if(moveStack[ply].moveScorePairs.size() == 0) {
        Array<Move, 256> moves;
        if(boardCopy.isCheck())
            boardCopy.generateLegalMoves(moves);
        else
            boardCopy.generateLegalCaptures(moves);

        scoreMovesForQuiescence(moves, ply);
    }

    Move bestMove = Move::nullMove();
    int16_t bestScore = minScore - 1;
    size_t bestIndex = 0;

    for(size_t i = 0; i < moveStack[ply].moveScorePairs.size(); i++)
        if(moveStack[ply].moveScorePairs[i].score > bestScore) {
            bestScore = moveStack[ply].moveScorePairs[i].score;
            bestMove = moveStack[ply].moveScorePairs[i].move;
            bestIndex = i;
        }

    moveStack[ply].moveScorePairs[bestIndex].score = MIN_SCORE - 1;

    return bestMove;
}

void PVSEngine::search(uint32_t time, bool treatAsTimeControl) {
    stopFlag = false;
    isTimeControlled = treatAsTimeControl;
    nodesSearched = 0;
    maxDepthReached = 0;

    boardCopy = Board(*board);
    evaluator.setBoard(boardCopy);

    clearKillerMoves();
    clearHistoryTable();
    clearPVTable();
    clearPVHistory();
    variations.clear();

    startTime = std::chrono::system_clock::now();
    calculateTimeLimits(time, treatAsTimeControl);

    int16_t alpha = MIN_SCORE, beta = MAX_SCORE;

    for(int16_t depth = 1; depth <= MAX_PLY; depth++) {
        pvs(depth * ONE_PLY, 0, alpha, beta, false, true);

        if(stopFlag)
            break;
        else {
            maxDepthReached = depth;

            if(pvHistory.size() == 5)
                pvHistory.shiftLeft(0);

            pvHistory.push_back({
                getBestMove(),
                getBestMoveScore()
            });
        }

        int16_t score = getBestMoveScore();
        std::string scoreStr;
        if(isMateScore(score)) {
            scoreStr = "mate ";
            if(score < 0)
                scoreStr += "-";
            scoreStr += std::to_string(isMateIn(score));
        } else {
            scoreStr = "cp " + std::to_string(score);
        }

        std::chrono::milliseconds timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime);
        std::cout << "info depth " << depth << " score " << scoreStr << " nodes " << nodesSearched <<
                    " time " << timeElapsed.count() << " nps " << (uint64_t)(nodesSearched / (timeElapsed.count() / 1000.0)) <<
                    " pv ";
        for(Move move : variations[0].moves)
            std::cout << move.toString() << " ";

        std::cout << std::endl;

        if(!extendSearch(isTimeControlled))
            break;
    }

    evaluator.setBoard(*board);
    stopFlag = false;

    int16_t score = getBestMoveScore();
    std::string scoreStr;
    if(isMateScore(score)) {
        scoreStr = "mate ";
        if(score < 0)
            scoreStr += "-";
        scoreStr += std::to_string(isMateIn(score));
    } else {
        scoreStr = "cp " + std::to_string(score);
    }

    std::chrono::milliseconds timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime);
    std::cout << "info depth " << maxDepthReached << " score " << scoreStr << " nodes " << nodesSearched <<
                " time " << timeElapsed.count() << " nps " << (uint64_t)(nodesSearched / (timeElapsed.count() / 1000.0)) <<
                " pv ";
    for(Move move : variations[0].moves)
        std::cout << move.toString() << " ";

    std::cout << "\n" << "bestmove " << getBestMove().toString();

    std::vector<Move> pv = getPrincipalVariation();
    if(pv.size() > 1)
        std::cout << " ponder " << pv[1].toString();

    std::cout << std::endl;
}

void PVSEngine::calculateTimeLimits(uint32_t time, bool treatAsTimeControl) {
    if(time == 0)
        time = std::numeric_limits<uint32_t>::max();

    std::chrono::milliseconds alreadySearched = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime);

    if(!treatAsTimeControl) {
        timeMin = std::chrono::milliseconds(time + alreadySearched.count());
        timeMax = std::chrono::milliseconds(time + alreadySearched.count());
        return;
    }

    // Berechne die minimale und maximale Zeit, die die Suche dauern soll
    int32_t numLegalMoves = board->generateLegalMoves().size();

    double oneThirtiethOfTime = time * 0.0333;
    double oneFourthOfTime = time * 0.25;

    uint64_t minTime = oneThirtiethOfTime - oneThirtiethOfTime * std::exp(-0.05  * numLegalMoves);
    uint64_t maxTime = oneFourthOfTime - oneFourthOfTime * std::exp(-0.05  * numLegalMoves);

    timeMin = std::chrono::milliseconds(minTime + alreadySearched.count());
    timeMax = std::chrono::milliseconds(maxTime + alreadySearched.count());
}

bool PVSEngine::extendSearch(bool isTimeControlled) {
    if(stopFlag)
        return false;

    std::chrono::milliseconds timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime);

    if(timeElapsed >= timeMax)
        return false;
    else if(timeElapsed < timeMin || !isTimeControlled)
        return true;

    int32_t meanScore = 0;
    for(MoveScorePair& pair : pvHistory)
        meanScore += pair.score;

    if(pvHistory.size() > 0)
        meanScore /= pvHistory.size();

    int32_t scoreVariance = 0;
    for(MoveScorePair& pair : pvHistory)
        scoreVariance += (pair.score - meanScore) *
                         (pair.score - meanScore);

    if(pvHistory.size() > 1)
        scoreVariance /= pvHistory.size() - 1;

    int32_t bestMoveChanges = 0;

    for(size_t i = 0; i < pvHistory.size() - 1; i++)
        if(pvHistory[i].move != pvHistory[i + 1].move)
            bestMoveChanges++;

    double timeFactor = ((double)timeElapsed.count() - (double)timeMin.count()) /
                        ((double)timeMax.count() - (double)timeMin.count());

    timeFactor = std::clamp(timeFactor, 0.0, 1.0);

    if(bestMoveChanges >= 4)
        return true;

    if(bestMoveChanges >= 3 && scoreVariance >= 40 * 40 * timeFactor)
        return true;

    if(bestMoveChanges >= 2 && scoreVariance >= 60 * 60 * timeFactor)
        return true;

    if(bestMoveChanges >= 1 && scoreVariance >= 80 * 80 * timeFactor)
        return true;

    return false;
}