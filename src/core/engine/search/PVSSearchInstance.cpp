#include "core/engine/search/PVSSearchInstance.h"

int16_t PVSSearchInstance::pvs(int16_t depth, uint16_t ply, int16_t alpha, int16_t beta, bool allowNullMove, uint8_t nodeType) {
    if(locallySearchedNodes >= NODES_PER_CHECKUP && checkupFunction) {
        locallySearchedNodes = 0;
        checkupFunction();
    }

    if(stopFlag.load())
        return 0;

    if(depth <= 0)
        return quiescence(ply, alpha, beta);

    nodesSearched.fetch_add(1);
    locallySearchedNodes++;

    uint16_t repetitionCount = board.repetitionCount();
    if(repetitionCount >= 3 || board.getFiftyMoveCounter() >= 100)
        return DRAW_SCORE;

    TranspositionTableEntry entry;
    if(nodeType != PV_NODE && transpositionTable.probe(board.getHashValue(), entry)) {
        if(entry.depth * ONE_PLY >= depth) {
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

    if(allowNullMove && !board.isCheck() &&
       depth > ONE_PLY && !deactivateNullMove()) {
        
        Move nullMove = Move::nullMove();
        board.makeMove(nullMove);

        int16_t score = -pvs(depth - calculateNullMoveReduction(depth), ply + 1, -beta, -alpha, false, CUT_NODE);

        board.undoMove();

        if(stopFlag.load())
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
    bool isCheckEvasion = board.isCheck();

    while((pair = selectNextMove(ply, nodeType != ALL_NODE && depth >= 6 * ONE_PLY, depth)).move.exists()) {
        move = pair.move;

        evaluator.updateBeforeMove(move);
        board.makeMove(move);
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
                    board.undoMove();
                    evaluator.updateAfterUndo(move);

                    nodesSearched.fetch_add(1);
                    locallySearchedNodes++;
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
        int32_t fiftyMoveCounter = board.getFiftyMoveCounter();
        if(fiftyMoveCounter > 20 && !isMateScore(score))
            score = (int32_t)score * (100 - fiftyMoveCounter) / 80;

        evaluator.updateBeforeUndo();
        board.undoMove();
        evaluator.updateAfterUndo(move);

        if(stopFlag.load())
            return 0;

        if(score >= beta) {
            TranspositionTableEntry entry(
                move,
                score,
                board.getPly(),
                (uint8_t)(depth / ONE_PLY),
                CUT_NODE
            );

            transpositionTable.put(board.getHashValue(), entry);

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
            }
        }

        moveCount++;
    }

    // Es gab keine legalen Züge, das Spiel ist vorbei.
    if(moveCount == 0) {
        clearPVTable(ply);

        if(board.isCheck())
            return -MATE_SCORE + ply; // Matt
        else
            return DRAW_SCORE; // Remis
    }

    entry = {
        bestMove,
        bestScore,
        board.getPly(),
        (uint8_t)(depth / ONE_PLY),
        actualNodeType
    };

    transpositionTable.put(board.getHashValue(), entry);

    incrementHistoryScore(bestMove, depth);

    return bestScore;
}

int16_t PVSSearchInstance::quiescence(int16_t ply, int16_t alpha, int16_t beta) {
    if(locallySearchedNodes >= NODES_PER_CHECKUP && checkupFunction) {
        locallySearchedNodes = 0;
        checkupFunction();
    }

    nodesSearched.fetch_add(1);
    locallySearchedNodes++;

    if(board.repetitionCount() >= 3 || board.getFiftyMoveCounter() >= 100)
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

    if(!board.isCheck()) {
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
        board.makeMove(move);
        evaluator.updateAfterMove();

        int16_t score = -quiescence(ply + 1, -beta, -alpha);

        evaluator.updateBeforeUndo();
        board.undoMove();
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
    if(moveCount == 0 && board.isCheck())
        return -MATE_SCORE + ply; // Matt

    return bestScore;
}

int16_t PVSSearchInstance::determineExtension() {
    int16_t extension = 0;

    // Erweiterung, wenn ein Spieler im Schach steht.
    if(board.isCheck())
        extension += ONE_PLY;

    return extension;
}

int16_t PVSSearchInstance::determineReduction(int16_t moveCount, uint8_t nodeType) {
    if(moveCount == 1)
        return 0;

    Move lastMove = board.getLastMove();
    if(lastMove.isCapture() || lastMove.isPromotion())
        return 0;
    else {
        int32_t movedPieceType = TYPEOF(board.pieceAt(lastMove.getDestination()));
        if(movedPieceType == PAWN) {
            int32_t side = board.getSideToMove() ^ COLOR_MASK;
            int32_t otherSide = side ^ COLOR_MASK;
            if(!(sentryMasks[side / COLOR_MASK][lastMove.getDestination()]
                & board.getPieceBitboard(otherSide | PAWN)))
                return 0;
        }
    }

    int32_t reduction = ONE_PLY;

    int32_t historyScore = getHistoryScore(lastMove, board.getSideToMove() ^ COLOR_MASK);
    reduction -= historyScore / 8192 * ONE_PLY;
    reduction = std::max(reduction, 0);

    if(nodeType == CUT_NODE)
        reduction += 2 * ONE_PLY;
    else if(nodeType == PV_NODE)
        reduction = std::min(reduction, 2 * ONE_PLY);

    return reduction;
}

bool PVSSearchInstance::deactivateNullMove() {
    // Deaktiviere den Null-Zug, wenn der Spieler nur noch
    // einen König und Bauern hat.
    int32_t side = board.getSideToMove();

    Bitboard ownPieces = side == WHITE ? board.getWhiteOccupiedBitboard() : board.getBlackOccupiedBitboard();

    return ownPieces == board.getPieceBitboard(side | PAWN);
}

void PVSSearchInstance::scoreMoves(const Array<Move, 256>& moves, uint16_t ply) {
    for(Move move : moves) {
        if(!move.exists())
            continue;

        int16_t score;
        int16_t scoreDistortion = 0;
        if(!isMainThread)
            scoreDistortion = mersenneTwister() % (2 * MAX_MOVE_SCORE_DISTORTION + 1) - MAX_MOVE_SCORE_DISTORTION;

        if(move.isCapture() || move.isPromotion()) {
            uint64_t nodesSearchedBySEE = 0;
            int16_t seeEvaluation = evaluator.evaluateMoveSEE(move, nodesSearchedBySEE);
            locallySearchedNodes += nodesSearchedBySEE;
            nodesSearched.fetch_add(nodesSearchedBySEE);

            if(seeEvaluation >= 0) {
                // Schlagzüge mit SEE >= 0
                score = std::clamp(GOOD_CAPTURE_MOVES_NEUTRAL + seeEvaluation + scoreDistortion,
                                   GOOD_CAPTURE_MOVES_MIN,
                                   GOOD_CAPTURE_MOVES_MAX);
            } else {
                // Schlagzüge mit SEE < 0
                score = std::clamp(BAD_CAPTURE_MOVES_NEUTRAL + seeEvaluation + scoreDistortion,
                                   BAD_CAPTURE_MOVES_MIN,
                                   BAD_CAPTURE_MOVES_MAX);
            }
        } else {
            if(isKillerMove(ply, move)) {
                // Killerzüge
                score = KILLER_MOVE_SCORE;
            } else {
                // Ruhige Züge
                score = std::clamp(QUIET_MOVES_NEUTRAL + getHistoryScore(move) / currentSearchDepth + scoreDistortion,
                                   QUIET_MOVES_MIN,
                                   QUIET_MOVES_MAX);
            }
        }

        moveStack[ply].moveScorePairs.push_back(MoveScorePair(move, score));
    }
}

void PVSSearchInstance::scoreMovesForQuiescence(const Array<Move, 256>& moves, uint16_t ply) {
    for(Move move : moves) {
        uint64_t nodesSearchedBySEE = 0;
        int16_t score = evaluator.evaluateMoveSEE(move, nodesSearchedBySEE);
        locallySearchedNodes += nodesSearchedBySEE;
        nodesSearched.fetch_add(nodesSearchedBySEE);
    
        moveStack[ply].moveScorePairs.push_back(MoveScorePair(move, score));
    }
}

MoveScorePair PVSSearchInstance::selectNextMove(uint16_t ply, bool useIID, int16_t depth) {
    if(moveStack[ply].moveScorePairs.size() == 0) {
        if(!moveStack[ply].hashMove.exists()) {
            Move hashMove = Move::nullMove();
            TranspositionTableEntry entry;
            if(transpositionTable.probe(board.getHashValue(), entry))
                hashMove = entry.hashMove;

            if(hashMove.exists() && (ply != 0 || searchMoves.size() == 0 || searchMoves.contains(hashMove))) {
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
                    board.makeMove(move);
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
                    board.undoMove();
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
        if(ply > 0 || searchMoves.size() == 0)
            board.generateLegalMoves(moves);
        else
            moves = searchMoves;
        
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

MoveScorePair PVSSearchInstance::selectNextMoveInQuiescence(uint16_t ply, int16_t minScore) {
    if(moveStack[ply].moveScorePairs.size() == 0) {
        Array<Move, 256> moves;
        if(board.isCheck())
            board.generateLegalMoves(moves);
        else
            board.generateLegalCaptures(moves);

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