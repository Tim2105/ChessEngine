#include "core/engine/search/PVSEngine.h"
#include "core/chess/Referee.h"

#include <algorithm>
#include <math.h>

int16_t PVSEngine::pvs(int16_t depth, uint16_t ply, int16_t alpha, int16_t beta, bool allowNullMove, uint8_t nodeType) {
    if(nodesSearched % NODES_PER_CHECKUP == 0 && isCheckupTime())
        checkup();

    if(stopFlag)
        return 0;

    if(depth <= 0)
        return quiescence(ply, alpha, beta);

    nodesSearched++;

    uint16_t repetitionCount = boardCopy.repetitionCount();
    if(repetitionCount >= 3 || boardCopy.getFiftyMoveCounter() >= 100)
        return DRAW_SCORE;

    TranspositionTableEntry entry;
    if(nodeType != PV_NODE && transpositionTable.probe(boardCopy.getHashValue(), entry)) {
        if(entry.depth >= depth) {
            if(entry.type == PV_NODE) {
                return entry.score;
            } else if(entry.type == CUT_NODE) {
                if(entry.score >= beta)
                    return entry.score;
            } else if(entry.type == ALL_NODE) {
                if(entry.score <= alpha)
                    return entry.score;
            }
        }
    }

    if(allowNullMove && !boardCopy.isCheck() &&
       depth > ONE_PLY && !deactivateNullMove()) {
        
        Move nullMove = Move::nullMove();
        boardCopy.makeMove(nullMove);

        int16_t score = -pvs(depth - calculateNullMoveReduction(depth), ply + 1, -beta, -alpha, false, CUT_NODE);

        boardCopy.undoMove();

        if(stopFlag)
            return 0;

        if(score >= beta)
            return score;
    }

    clearPVTable(ply + 1);
    clearMoveStack(ply);

    uint8_t actualNodeType = ALL_NODE;
    int16_t bestScore = MIN_SCORE, moveCount = 0;
    MoveScorePair pair;
    Move move, bestMove;
    bool isCheckEvasion = boardCopy.isCheck();

    while((pair = selectNextMove(ply, nodeType != ALL_NODE && depth >= 6 * ONE_PLY, depth)).move.exists()) {
        move = pair.move;

        evaluator.updateBeforeMove(move);
        boardCopy.makeMove(move);
        evaluator.updateAfterMove();

        int16_t score;
        uint8_t childType = CUT_NODE;

        if(nodeType == PV_NODE && moveCount == 0)
            childType = PV_NODE;
        else if(nodeType == CUT_NODE && moveCount == 0)
            childType = ALL_NODE;

        int16_t extension = determineExtension();
        int16_t reduction = 0;
        if(extension == 0 && !isCheckEvasion) {
            if(depth >= 3 * ONE_PLY)
                reduction = determineReduction(moveCount + 1, nodeType);
            else if(depth == 2 * ONE_PLY && nodeType == ALL_NODE) {
                int16_t staticEvaluation = evaluator.evaluate();

                if(staticEvaluation <= alpha) {
                    evaluator.updateBeforeUndo();
                    boardCopy.undoMove();
                    evaluator.updateAfterUndo(move);

                    nodesSearched++;
                    moveCount++;

                    continue;
                }
            }
        }

        if(moveCount == 0) {
            score = -pvs(depth - ONE_PLY + extension, ply + 1, -beta, -alpha, childType == CUT_NODE, childType);
        } else {
            score = -pvs(depth - ONE_PLY + extension - reduction, ply + 1, -alpha - 1, -alpha, childType == CUT_NODE, childType);

            if(score > alpha && (reduction > 0 || nodeType == PV_NODE)) {
                if(nodeType == PV_NODE)
                    childType = PV_NODE;
                else
                    childType = ALL_NODE;

                score = -pvs(depth - ONE_PLY + extension, ply + 1, -beta, -alpha, false, childType);
            }
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
        if(fiftyMoveCounter > 20 && !isMateScore(score))
            score = (int32_t)score * (100 - fiftyMoveCounter) / 80;

        evaluator.updateBeforeUndo();
        boardCopy.undoMove();
        evaluator.updateAfterUndo(move);

        if(stopFlag)
            return 0;

        if(score >= beta) {
            TranspositionTableEntry entry {
                boardCopy.getPly(),
                depth,
                score,
                CUT_NODE,
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
                actualNodeType = PV_NODE;
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
        boardCopy.getPly(),
        depth,
        bestScore,
        actualNodeType,
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
    MoveScorePair pair;
    Move move;

    while((pair = selectNextMoveInQuiescence(ply, minMoveScore)).move.exists()) {
        move = pair.move;

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

int16_t PVSEngine::determineExtension() {
    int16_t extension = 0;

    // Erweiterung, wenn ein Spieler im Schach steht.
    if(boardCopy.isCheck())
        extension += ONE_PLY;

    return extension;
}

int16_t PVSEngine::determineReduction(int16_t moveCount, uint8_t nodeType) {
    if(moveCount == 1)
        return 0;

    Move lastMove = boardCopy.getLastMove();
    if(lastMove.isCapture() || lastMove.isPromotion())
        return 0;
    else {
        int32_t movedPieceType = TYPEOF(boardCopy.pieceAt(lastMove.getDestination()));
        if(movedPieceType == PAWN) {
            int32_t side = boardCopy.getSideToMove() ^ COLOR_MASK;
            int32_t otherSide = side ^ COLOR_MASK;
            if(!(sentryMasks[side / COLOR_MASK][lastMove.getDestination()]
                & boardCopy.getPieceBitboard(otherSide | PAWN)))
                return 0;
        }
    }

    int32_t reduction = ONE_PLY;

    int32_t historyScore = getHistoryScore(lastMove, boardCopy.getSideToMove() ^ COLOR_MASK);
    reduction -= historyScore / 8192 * ONE_PLY;
    reduction = std::max(reduction, 0);

    if(nodeType == CUT_NODE)
        reduction += 2 * ONE_PLY;
    else if(nodeType == PV_NODE)
        reduction = std::min(reduction, 2 * ONE_PLY);

    return reduction;
}

bool PVSEngine::deactivateNullMove() {
    // Deaktiviere den Null-Zug, wenn der Spieler nur noch
    // einen König und Bauern hat.
    int32_t side = boardCopy.getSideToMove();

    Bitboard ownPieces = side == WHITE ? boardCopy.getWhiteOccupiedBitboard() : boardCopy.getBlackOccupiedBitboard();

    return ownPieces == boardCopy.getPieceBitboard(side | PAWN);
}

void PVSEngine::scoreMoves(const Array<Move, 256>& moves, uint16_t ply) {
    for(Move move : moves) {
        int16_t score = 0;

        if(move.isCapture()) {
            int16_t seeEvaluation = evaluator.evaluateMoveSEE(move);
            if(seeEvaluation >= 0)
                score += seeEvaluation + KILLER_MOVE_SCORE + 1;
            else
                score += KILLER_MOVE_SCORE - 1;
        } else {
            if(isKillerMove(ply, move))
                score += KILLER_MOVE_SCORE;
            else
                score += std::clamp(getHistoryScore(move) / (maxDepthReached + 1),
                                    MIN_SCORE + 1,
                                    KILLER_MOVE_SCORE - 2);
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

PVSEngine::MoveScorePair PVSEngine::selectNextMove(uint16_t ply, bool useIID, int16_t depth) {
    if(moveStack[ply].moveScorePairs.size() == 0) {
        if(!moveStack[ply].hashMove.exists()) {
            Move hashMove = Move::nullMove();
            TranspositionTableEntry entry;
            if(transpositionTable.probe(boardCopy.getHashValue(), entry))
                hashMove = entry.hashMove;

            if(hashMove.exists()) {
                moveStack[ply].hashMove = hashMove;
                return { hashMove, HASH_MOVE_SCORE };
            } else if(useIID) {
                Array<MoveScorePair, 256> moveScorePairs;

                MoveScorePair pair;
                Move move, iidMove;
                int16_t iidScore = MIN_SCORE, moveCount = 0;

                int16_t reducedDepth = (depth / (2 * ONE_PLY)) * ONE_PLY;

                while((pair = selectNextMove(ply, reducedDepth >= 6 * ONE_PLY, reducedDepth)).move.exists()) {
                    move = pair.move;

                    evaluator.updateBeforeMove(move);
                    boardCopy.makeMove(move);
                    evaluator.updateAfterMove();

                    int16_t reduction = determineReduction(moveCount + 1, PV_NODE);
                    int16_t score;

                    if(moveCount == 0) {
                        score = -pvs(reducedDepth, ply + 1, -MAX_SCORE, -iidScore, false, PV_NODE);
                    } else {
                        score = -pvs(reducedDepth - reduction, ply + 1, -iidScore - 1, -iidScore, true, CUT_NODE);

                        if(score > iidScore)
                            score = -pvs(reducedDepth, ply + 1, -MAX_SCORE, -iidScore, false, PV_NODE);
                    }

                    evaluator.updateBeforeUndo();
                    boardCopy.undoMove();
                    evaluator.updateAfterUndo(move);

                    if(score > iidScore) {
                        iidScore = score;
                        iidMove = move;
                    }

                    moveScorePairs.push_back(MoveScorePair(move, score));

                    moveCount++;
                }

                clearMoveStack(ply);

                if(iidMove.exists()) {
                    for(MoveScorePair& pair : moveScorePairs)
                        if(pair.move == iidMove)
                            pair.score = MIN_SCORE;

                    moveStack[ply].moveScorePairs = moveScorePairs;
                    moveStack[ply].hashMove = iidMove;
                    return { iidMove, HASH_MOVE_SCORE };
                }
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

    return { bestMove, bestScore };
}

PVSEngine::MoveScorePair PVSEngine::selectNextMoveInQuiescence(uint16_t ply, int16_t minScore) {
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

    return { bestMove, bestScore };
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
        int16_t score = pvs(depth * ONE_PLY, 0, alpha, beta, false, PV_NODE);

        bool alphaAlreadyWidened = false, betaAlreadyWidened = false;
        while(score <= alpha || score >= beta) {
            if(score <= alpha) {
                if(alphaAlreadyWidened)
                    alpha = MIN_SCORE;
                else {
                    alphaAlreadyWidened = true;
                    alpha -= 135;
                }
            } else {
                if(betaAlreadyWidened)
                    beta = MAX_SCORE;
                else {
                    betaAlreadyWidened = true;
                    beta += 135;
                }
            }

            score = pvs(depth * ONE_PLY, 0, alpha, beta, false, PV_NODE);
        }

        alpha = score - 15;
        beta = score + 15;

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

        score = getBestMoveScore();
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