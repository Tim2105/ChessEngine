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
            if(entry.type == PV_NODE || entry.type == ALL_NODE) {
                return entry.score;
            } else if(entry.type == CUT_NODE) {
                if(entry.score >= beta)
                    return entry.score;
            }
        }
    }

    if(allowNullMove && !board.isCheck() &&
       depth > ONE_PLY && !deactivateNullMove()) {

        int16_t staticEvaluation = evaluator.evaluate();
        if(staticEvaluation > beta) {
            Move nullMove = Move::nullMove();
            board.makeMove(nullMove);

            int16_t score = -pvs(depth - calculateNullMoveReduction(depth, staticEvaluation, beta),
                                ply + 1, -beta, -alpha, false, CUT_NODE);

            board.undoMove();

            if(stopFlag.load())
                return 0;

            if(score >= beta)
                return score;
        }
    }

    clearPVTable(ply + 1);
    prepareSearchStack(ply, nodeType != ALL_NODE && depth >= 6 * ONE_PLY, depth);

    Move move;
    int16_t moveCount = 0, moveScore;

    uint8_t actualNodeType = ALL_NODE;
    int16_t bestScore = MIN_SCORE;
    moveCount = 0;
    Move bestMove;
    bool isCheckEvasion = board.isCheck();

    for(MoveScorePair& pair : searchStack[ply].moveScorePairs) {
        move = pair.move;
        moveScore = pair.score;

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
                reduction = determineReduction(moveCount + 1, moveScore, nodeType);
            else if(depth == 2 * ONE_PLY && nodeType == ALL_NODE) {
                int16_t staticEvaluation = evaluator.evaluate();

                if(staticEvaluation <= alpha) {
                    evaluator.updateBeforeUndo();
                    board.undoMove();
                    evaluator.updateAfterUndo(move);

                    if(staticEvaluation > bestScore) {
                        bestScore = staticEvaluation;
                        bestMove = move;
                    }

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

            if(!move.isCapture() && !move.isPromotion())
                addKillerMove(ply, move);

            incrementHistoryScore(move, depth);

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

    prepareSearchStackForQuiescence(ply, minMoveScore, false);

    int16_t moveCount = 0;
    Move move;

    for(MoveScorePair& pair : searchStack[ply].moveScorePairs) {
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

int16_t PVSSearchInstance::determineReduction(int16_t moveCount, int16_t moveScore, uint8_t nodeType) {
    if(moveCount == 1)
        return 0;

    Move lastMove = board.getLastMove();
    if(moveScore >= GOOD_CAPTURE_MOVES_MIN || lastMove.isPromotion())
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
    reduction -= historyScore / 16384 * ONE_PLY;
    reduction = std::max(reduction, 0); 

    if(nodeType == CUT_NODE)
        reduction += ONE_PLY;

    return reduction;
}

bool PVSSearchInstance::deactivateNullMove() {
    // Deaktiviere den Null-Zug, wenn der Spieler nur noch
    // einen König und Bauern hat.
    int32_t side = board.getSideToMove();

    Bitboard ownPieces = side == WHITE ? board.getWhiteOccupiedBitboard() : board.getBlackOccupiedBitboard();

    return ownPieces == board.getPieceBitboard(side | PAWN);
}

void PVSSearchInstance::prepareSearchStack(uint16_t ply, bool useIID, int16_t depth) {
    clearSearchStack(ply);

    Move hashMove = Move::nullMove();
    TranspositionTableEntry entry;
    if(transpositionTable.probe(board.getHashValue(), entry))
        hashMove = entry.hashMove;

    if(hashMove.exists() && (ply != 0 || searchMoves.size() == 0 || searchMoves.contains(hashMove))) {
        searchStack[ply].hashMove = hashMove;
        searchStack[ply].moveScorePairs.push_back(MoveScorePair(hashMove, HASH_MOVE_SCORE));

        Array<Move, 256> moves;
        if(ply > 0 || searchMoves.size() == 0)
            board.generateLegalMoves(moves);
        else
            moves = searchMoves;

        moves.remove_first(hashMove);
        scoreMoves(moves, ply);
    } else if(useIID) {
        MoveScorePair pair;
        Move move, iidMove = Move::nullMove();
        int16_t iidScore = MIN_SCORE, moveCount = 0, moveScore;

        int16_t reducedDepth = (depth / (2 * ONE_PLY)) * ONE_PLY;

        prepareSearchStack(ply, reducedDepth >= 6 * ONE_PLY, reducedDepth);

        for(MoveScorePair& pair : searchStack[ply].moveScorePairs) {
            move = pair.move;
            moveScore = pair.score;

            evaluator.updateBeforeMove(move);
            board.makeMove(move);
            evaluator.updateAfterMove();

            int16_t reduction = determineReduction(moveCount + 1, moveScore, PV_NODE);
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

            moveCount++;
        }

        searchStack[ply].hashMove = iidMove;
        searchStack[ply].moveScorePairs.clear();

        if(iidMove.exists())
            searchStack[ply].moveScorePairs.push_back(MoveScorePair(iidMove, HASH_MOVE_SCORE));

        Array<Move, 256> moves;
        if(ply > 0 || searchMoves.size() == 0)
            board.generateLegalMoves(moves);
        else
            moves = searchMoves;

        moves.remove_first(iidMove);
        scoreMoves(moves, ply);
    } else {
        Array<Move, 256> moves;
        if(ply > 0 || searchMoves.size() == 0)
            board.generateLegalMoves(moves);
        else
            moves = searchMoves;

        scoreMoves(moves, ply);
    }
}

void PVSSearchInstance::prepareSearchStackForQuiescence(uint16_t ply, int16_t minMoveScore, bool includeHashMove) {
    clearSearchStack(ply);

    if(includeHashMove) {
        Move hashMove = Move::nullMove();
        TranspositionTableEntry entry;
        if(transpositionTable.probe(board.getHashValue(), entry))
            hashMove = entry.hashMove;

        if(hashMove.exists()) {
            searchStack[ply].hashMove = hashMove;
            searchStack[ply].moveScorePairs.push_back(MoveScorePair(hashMove, HASH_MOVE_SCORE));
        }

        Array<Move, 256> moves;
        if(board.isCheck())
            board.generateLegalMoves(moves);
        else
            board.generateLegalCaptures(moves);

        moves.remove_first(hashMove);

        scoreMovesForQuiescence(moves, ply, minMoveScore);
    } else {
        Array<Move, 256> moves;
        if(board.isCheck())
            board.generateLegalMoves(moves);
        else
            board.generateLegalCaptures(moves);

        scoreMovesForQuiescence(moves, ply, minMoveScore);
    }
}

void PVSSearchInstance::scoreMoves(const Array<Move, 256>& moves, uint16_t ply) {
    Array<MoveScorePair, 64> goodCaptures;
    Array<MoveScorePair, 64> badCaptures;
    Array<MoveScorePair, 256> quietMoves;
    Array<MoveScorePair, 2> killers;

    for(Move move : moves) {
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

                goodCaptures.insert_sorted(MoveScorePair(move, score), std::greater<MoveScorePair>());
            } else {
                // Schlagzüge mit SEE < 0
                score = std::clamp(BAD_CAPTURE_MOVES_NEUTRAL + seeEvaluation + scoreDistortion,
                                   BAD_CAPTURE_MOVES_MIN,
                                   BAD_CAPTURE_MOVES_MAX);

                badCaptures.insert_sorted(MoveScorePair(move, score), std::greater<MoveScorePair>());
            }
        } else {
            if(isKillerMove(ply, move)) {
                // Killerzüge
                score = KILLER_MOVE_SCORE;
                killers.push_back(MoveScorePair(move, score));
            } else {
                // Ruhige Züge
                score = std::clamp(QUIET_MOVES_NEUTRAL + getHistoryScore(move) / currentSearchDepth + scoreDistortion,
                                   QUIET_MOVES_MIN,
                                   QUIET_MOVES_MAX);

                quietMoves.insert_sorted(MoveScorePair(move, score), std::greater<MoveScorePair>());
            }
        }
    }

    searchStack[ply].moveScorePairs.push_back(goodCaptures);
    searchStack[ply].moveScorePairs.push_back(killers);
    searchStack[ply].moveScorePairs.push_back(badCaptures);
    searchStack[ply].moveScorePairs.push_back(quietMoves);
}

void PVSSearchInstance::scoreMovesForQuiescence(const Array<Move, 256>& moves, uint16_t ply, int16_t minMoveScore) {
    for(Move move : moves) {
        uint64_t nodesSearchedBySEE = 0;
        int16_t score = evaluator.evaluateMoveSEE(move, nodesSearchedBySEE);
        locallySearchedNodes += nodesSearchedBySEE;
        nodesSearched.fetch_add(nodesSearchedBySEE);
    
        if(score >= minMoveScore)
            searchStack[ply].moveScorePairs.insert_sorted(MoveScorePair(move, score), std::greater<MoveScorePair>());
    }
}