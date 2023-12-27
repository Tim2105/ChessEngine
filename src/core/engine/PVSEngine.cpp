#include "core/engine/PVSEngine.h"
#include "core/chess/Referee.h"

#include <math.h>

int16_t PVSEngine::pvs(int16_t depth, uint16_t ply, int16_t alpha, int16_t beta, bool allowNullMove, bool isPVNode) {
    if(nodesSearched % NODES_PER_CHECKUP == 0 && isCheckupTime())
        checkup();

    if(stopFlag)
        return 0;

    nodesSearched++;

    if(boardCopy.repetitionCount() >= 3)
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

    if(depth <= 0 || ply >= (maxDepthReached + 1) * 6)
        return quiescence(ply, alpha, beta);

    int16_t staticEvaluation = evaluator.evaluate();
    bool isThreat = false;

    if(allowNullMove && !boardCopy.isCheck() &&
       depth > ONE_PLY && !deactivateNullMove()) {
        
        Move nullMove = Move::nullMove();
        boardCopy.makeMove(nullMove);

        int16_t score = -pvs(depth - calculateNullMoveReduction(depth), ply + 1, -alpha - 1, -alpha, false, false);

        boardCopy.undoMove();

        if(stopFlag)
            return 0;

        if(!isPVNode) {
            if(score >= beta)
                return score;

            if(score > alpha)
                alpha = score;
        }

        if(score < staticEvaluation - NULL_MOVE_EXTENSION_MARGIN) {
            isThreat = true;
            if(depth <= NULL_MOVE_MAX_EXTENSION_DEPTH)
                depth += ONE_HALF_PLY;
            
        }
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
        if(!isPVNode && extension == 0 && moveCount > 3 &&
           !boardCopy.isCheck() && !isThreat)
            reduction = determineReduction(moveCount);

        int16_t score;

        if(moveCount == 0) {
            score = -pvs(depth - ONE_PLY + extension, ply + 1, -beta, -alpha, true, true);
        } else {
            score = -pvs(depth - ONE_PLY + extension - reduction, ply + 1, -alpha - 1, -alpha, true, false);

            if(score > alpha && (score < beta || reduction > 0))
                score = -pvs(depth - ONE_PLY + extension, ply + 1, -beta, -alpha, true, true);
        }

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

    if(boardCopy.repetitionCount() >= 3)
        return DRAW_SCORE;

    int16_t standPat = evaluator.evaluate();

    if(standPat >= beta)
        return beta;

    if(standPat > alpha)
        alpha = standPat;

    int16_t bestScore = MIN_SCORE;

    if(!boardCopy.isCheck())
        bestScore = standPat;

    clearMoveStack(ply);

    int16_t moveCount = 0;
    Move move;

    while((move = selectNextMoveInQuiescence(ply, NEUTRAL_SCORE)).exists()) {
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

    // Erweiterung, wenn eine Figur geschlagen wird,
    // ein Bauer promoviert oder ein Freibauer gezogen wird.
    Move lastMove = boardCopy.getLastMove();
    if(lastMove.isCapture() || lastMove.isPromotion())
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

    int16_t reduction = ONE_HALF_PLY;

    Move lastMove = boardCopy.getLastMove();
    reduction -= getHistoryScore(lastMove, boardCopy.getSideToMove() ^ COLOR_MASK) * ONE_PLY / 16384;

    return std::clamp(reduction, ONE_HALF_PLY, (int16_t)(2 * ONE_PLY));
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

void PVSEngine::scoreMoves(Array<Move, 256>& moves, uint16_t ply) {
    for(Move move : moves) {
        int16_t score = 0;

        if(move.isCapture())
            score = evaluator.evaluateMoveSEE(move);
        else {
            if(isKillerMove(ply, move))
                score += KILLER_MOVE_SCORE;

            score += std::clamp(getHistoryScore(move), (int16_t)-KILLER_MOVE_SCORE, KILLER_MOVE_SCORE);
        }

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

        Array<Move, 256> moves = boardCopy.generateLegalMoves();
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
            moves = boardCopy.generateLegalMoves();
        else
            moves = boardCopy.generateLegalCaptures();

        scoreMoves(moves, ply);
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

    moveStack[ply].moveScorePairs[bestIndex].score = MIN_SCORE;

    return bestMove;
}

void PVSEngine::search(uint32_t time, bool treatAsTimeControl) {
    stopFlag = false;
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

        if(!extendSearch(treatAsTimeControl))
            break;

        std::chrono::milliseconds timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime);
        std::cout << "info depth " << depth << " score cp " << getBestMoveScore() << " pv ";
        for(Move move : variations[0].moves)
            std::cout << move.toString() << " ";

        std::cout << "nodes " << nodesSearched << " time " << timeElapsed.count() << " nps " << (uint64_t)(nodesSearched / (timeElapsed.count() / 1000.0)) << std::endl;
    }

    evaluator.setBoard(*board);

    std::chrono::milliseconds timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime);
    std::cout << "info score cp " << getBestMoveScore() << " pv ";
    for(Move move : variations[0].moves)
        std::cout << move.toString() << " ";

    std::cout << "nodes " << nodesSearched << " time " << timeElapsed.count() << " nps " << (uint64_t)(nodesSearched / (timeElapsed.count() / 1000.0)) << std::endl;
}

void PVSEngine::calculateTimeLimits(uint32_t time, bool treatAsTimeControl) {
    if(!treatAsTimeControl) {
        timeMin = std::chrono::milliseconds(time);
        timeMax = std::chrono::milliseconds(time);
        return;
    }

    // Berechne die minimale und maximale Zeit, die die Suche dauern soll
    int32_t numLegalMoves = board->generateLegalMoves().size();

    double oneThirtiethOfTime = time * 0.0333;
    double oneFourthOfTime = time * 0.25;

    uint64_t minTime = oneThirtiethOfTime - oneThirtiethOfTime * std::exp(-0.05  * numLegalMoves);
    uint64_t maxTime = oneFourthOfTime - oneFourthOfTime * std::exp(-0.05  * numLegalMoves);

    timeMin = std::chrono::milliseconds(minTime);
    timeMax = std::chrono::milliseconds(maxTime);
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