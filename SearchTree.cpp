#include "SearchTree.h"
#include <thread>
#include <algorithm>
#include "EvaluationDefinitions.h"
#include <cmath>
#include "MoveNotations.h"

SearchTree::SearchTree(Board& b) {
    board = &b;
    evaluator = BoardEvaluator(b);

    currentMaxDepth = 0;
    currentAge = b.getPly();
    nodesSearched = 0;

    searching = false;
}

void SearchTree::searchTimer(uint32_t searchTime) {
    std::this_thread::sleep_for(std::chrono::milliseconds(searchTime));
    searching = false;
}

void SearchTree::clearRelativeHistory() {
    for(int i = 0; i < 2; i++) {
        for(int j = 0; j < 64; j++) {
            for(int k = 0; k < 64; k++) {
                relativeHistory[i][j][k] = 0;
            }
        }
    }
}

void SearchTree::clearPvTable() {
    for(int i = 0; i < 32; i++) {
        pvTable[i].clear();
    }
}

int16_t SearchTree::search(uint32_t searchTime) {
    searching = true;
    currentMaxDepth = 0;
    currentAge = board->getPly();
    nodesSearched = 0;

    int16_t lastScore;
    clearRelativeHistory();

    std::thread timer(std::bind(&SearchTree::searchTimer, this, searchTime));

    int16_t score = evaluator.evaluate();

    auto start = std::chrono::high_resolution_clock::now();

    for(int16_t depth = ONE_PLY; searching; depth += ONE_PLY) {
        currentMaxDepth = depth;

        score = rootSearch(depth, score);

        auto now = std::chrono::high_resolution_clock::now();

        std::cout << "(" << (now - start).count() / 1000000 << "ms)" << "Depth: " << (int32_t)ceil((float)depth / ONE_PLY) << " Score: " << score
            << " Nodes: " << nodesSearched << " PV: ";
        
        for(std::string move : variationToFigurineAlgebraicNotation(principalVariation, *board))
            std::cout << move << " ";

        std::cout << std::endl;
        
        if(searching) {
            lastScore = score;
        }
    }

    timer.join();

    return lastScore;
}

void SearchTree::sortMoves(Array<Move, 256>& moves, int16_t ply, int32_t moveEvalFunc) {
    Array<MoveScorePair, 256> msp;

    Move hashMove;

    TranspositionTableEntry ttEntry;
    bool hashHit = transpositionTable.probe(board->getHashValue(), ttEntry);

    for(Move move : moves) {
        int32_t moveScore = 0;

        if(move == ttEntry.hashMove) {
            moveScore += 30000;
        } else if(move.isCapture() || move.isPromotion()) {
            switch(moveEvalFunc) {
                case MVVLVA:
                    moveScore += evaluator.evaluateMoveMVVLVA(move);
                    break;
                case SEE:
                    int32_t seeScore = evaluator.evaluateMoveSEE(move);
                    seeCache.put(move, seeScore);
                    moveScore += seeScore;
                    break;
            }
        } else if(move.isQuiet()) {
            int32_t moveScore = 0;

            if(killerMoves[ply][0] == move)
                moveScore += 80;
            else if(killerMoves[ply][1] == move)
                moveScore += 70;  
            else if(ply >= 2) {
                if(killerMoves[ply - 2][0] == move)
                    moveScore += 60;
                else if(killerMoves[ply - 2][1] == move)
                    moveScore += 50;
            }
        }

        int32_t movedPieceType = TYPEOF(board->pieceAt(move.getOrigin()));
        int32_t side = board->getSideToMove();
        int32_t psqtOrigin64 = board->sq120To64(move.getOrigin());
        int32_t psqtDestination64 = board->sq120To64(move.getDestination());

        if(side == BLACK) {
            int32_t rank = psqtOrigin64 / 8;
            int32_t file = psqtOrigin64 % 8;

            psqtOrigin64 = (RANK_8 - rank) * 8 + file;

            rank = psqtDestination64 / 8;
            file = psqtDestination64 % 8;

            psqtDestination64 = (RANK_8 - rank) * 8 + file;
        }

        moveScore += MOVE_ORDERING_PSQT[movedPieceType][psqtDestination64] - MOVE_ORDERING_PSQT[movedPieceType][psqtOrigin64];

        moveScore += std::clamp(relativeHistory[board->getSideToMove() / COLOR_MASK]
                            [board->sq120To64(move.getOrigin())]
                            [board->sq120To64(move.getDestination())] / (1 << (currentMaxDepth / ONE_PLY)),
                            -99, 69);

        msp.push_back({move, moveScore});
    }

    std::sort(msp.begin(), msp.end(), std::greater<MoveScorePair>());

    moves.clear();

    for(MoveScorePair pair : msp)
        moves.push_back(pair.move);
}

void SearchTree::sortAndCutMoves(Array<Move, 256>& moves, int32_t minScore, int32_t moveEvalFunc) {
    Array<MoveScorePair, 256> msp;

    for(Move move : moves) {
        int32_t moveScore = 0;

        switch(moveEvalFunc) {
            case MVVLVA:
                moveScore += evaluator.evaluateMoveMVVLVA(move);
                break;
            case SEE:
                int32_t seeScore = evaluator.evaluateMoveSEE(move);
                seeCache.put(move, seeScore);
                moveScore += seeScore;
                break;
        }

        if(moveScore >= minScore)
            msp.push_back({move, moveScore});
    }

    std::sort(msp.begin(), msp.end(), std::greater<MoveScorePair>());

    moves.clear();

    for(MoveScorePair msPair : msp)
        moves.push_back(msPair.move);
}

void SearchTree::sortAndCutMoves(Array<Move, 256>& moves, int16_t ply, int32_t minScore, int32_t moveEvalFunc) {
    Array<MoveScorePair, 256> msp;

    TranspositionTableEntry ttEntry;
    bool hashHit = transpositionTable.probe(board->getHashValue(), ttEntry);

    for(Move move : moves) {
        int32_t moveScore = 0;

        if(move == ttEntry.hashMove) {
            moveScore += 30000;
        }  else if(move.isCapture() || move.isPromotion()) {
            switch(moveEvalFunc) {
                case MVVLVA:
                    moveScore += evaluator.evaluateMoveMVVLVA(move);
                    break;
                case SEE:
                    int32_t seeScore = evaluator.evaluateMoveSEE(move);
                    seeCache.put(move, seeScore);
                    moveScore += seeScore;
                    break;
            }
        } else if(move.isQuiet()) {
            int32_t moveScore = 0;

            if(killerMoves[ply][0] == move)
                moveScore += 80;
            else if(killerMoves[ply][1] == move)
                moveScore += 70;  
            else if(ply >= 2) {
                if(killerMoves[ply - 2][0] == move)
                    moveScore += 60;
                else if(killerMoves[ply - 2][1] == move)
                    moveScore += 50;
            }
        }

        int32_t movedPieceType = TYPEOF(board->pieceAt(move.getOrigin()));
        int32_t side = board->getSideToMove();
        int32_t psqtOrigin64 = board->sq120To64(move.getOrigin());
        int32_t psqtDestination64 = board->sq120To64(move.getDestination());

        if(side == BLACK) {
            int32_t rank = psqtOrigin64 / 8;
            int32_t file = psqtOrigin64 % 8;

            psqtOrigin64 = (RANK_8 - rank) * 8 + file;

            rank = psqtDestination64 / 8;
            file = psqtDestination64 % 8;

            psqtDestination64 = (RANK_8 - rank) * 8 + file;
        }

        moveScore += MOVE_ORDERING_PSQT[movedPieceType][psqtDestination64] - MOVE_ORDERING_PSQT[movedPieceType][psqtOrigin64];

        moveScore += std::clamp(relativeHistory[board->getSideToMove() / COLOR_MASK]
                            [board->sq120To64(move.getOrigin())]
                            [board->sq120To64(move.getDestination())] / (1 << (currentMaxDepth / ONE_PLY)),
                            -99, 69);
                                    
        if(moveScore >= minScore)
            msp.push_back({move, moveScore});
    }

    std::sort(msp.begin(), msp.end(), std::greater<MoveScorePair>());

    moves.clear();

    for(MoveScorePair pair : msp)
        moves.push_back(pair.move);
}

int16_t SearchTree::rootSearch(int16_t depth, int16_t expectedScore) {
    int32_t aspAlphaReduction = ASP_WINDOW, numAlphaReduction = 1;
    int32_t aspBetaReduction = ASP_WINDOW, numBetaReduction = 1;

    int16_t alpha = expectedScore - aspAlphaReduction;
    int16_t beta = expectedScore + aspBetaReduction;

    int16_t score = pvSearchRoot(depth, alpha, beta);

    while((score <= alpha || score >= beta)) {
        if(score <= alpha) {
            if(numAlphaReduction >= ASP_MAX_DEPTH)
                alpha = MIN_SCORE;
            else {
                aspAlphaReduction *= ASP_STEP_FACTOR;
                alpha = expectedScore - aspAlphaReduction;
            }

            numAlphaReduction++;
        } else if(score >= beta) {
            if(numBetaReduction >= ASP_MAX_DEPTH)
                beta = MAX_SCORE;
            else {
                aspBetaReduction *= ASP_STEP_FACTOR;
                beta = expectedScore + aspBetaReduction;
            }

            numBetaReduction++;
        }

        score = pvSearchRoot(depth, alpha, beta);
    }
    
    return score;
}

int16_t SearchTree::pvSearchRoot(int16_t depth, int16_t alpha, int16_t beta) {
    clearPvTable();
    seeCache.clear();

    bool searchPv = true;
    int16_t score, bestScore = MIN_SCORE;
    Move bestMove;

    int32_t moveNumber = 1;
    bool isCheckEvasion = board->isCheck();

    Array<Move, 256> moves;

    moves = board->generateLegalMoves();
    sortMoves(moves, 0, SEE);

    for(Move move : moves) {
        if(!searching)
            return 0;
        
        nodesSearched++;

        board->makeMove(move);

        int16_t extension = determineExtension(depth, move, moveNumber, moves.size(), isCheckEvasion);
        int16_t nwReduction = determineReduction(depth, move, moveNumber, moves.size(), isCheckEvasion);

        int16_t nwDepthDelta = -ONE_PLY - nwReduction + extension;
        nwDepthDelta = std::min(nwDepthDelta, (int16_t)(-ONE_PLY));

        if(searchPv) {
            score = -pvSearch(depth - ONE_PLY, 1, -beta, -alpha);
        } else {
            score = -nwSearch(depth + nwDepthDelta, 1, -alpha - 1, -alpha);
            if(score > alpha)
                score = -pvSearch(depth - ONE_PLY, 1, -beta, -alpha);
        }

        board->undoMove();

        if(!searching)
            return 0;
        
        relativeHistory[board->getSideToMove() / COLOR_MASK]
                           [board->sq120To64(move.getOrigin())]
                           [board->sq120To64(move.getDestination())] -= depth / ONE_PLY;
        

        if(score >= beta) {
            transpositionTable.put(board->getHashValue(), {
                currentAge, depth, score, CUT_NODE, move
            });

            if(move.isQuiet()) {
                if(killerMoves[0][0] != move) {
                    killerMoves[0][1] = killerMoves[0][0];
                    killerMoves[0][0] = move;
                }
            }

            relativeHistory[board->getSideToMove() / COLOR_MASK]
                           [board->sq120To64(move.getOrigin())]
                           [board->sq120To64(move.getDestination())] += 1 << (depth / ONE_PLY);

            return score;
        }

        if(score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
        
        if(score > alpha) {
            alpha = score;
            principalVariation.clear();
            principalVariation.push_back(move);
            principalVariation.insert(principalVariation.end(), pvTable[1].begin(), pvTable[1].end());
        }

        searchPv = false;
        moveNumber++;
    }

    transpositionTable.put(board->getHashValue(), {
        currentAge, depth, bestScore, EXACT_NODE, bestMove
    });

    relativeHistory[board->getSideToMove() / COLOR_MASK]
                    [board->sq120To64(bestMove.getOrigin())]
                    [board->sq120To64(bestMove.getDestination())] += 1 << (depth / ONE_PLY);

    return bestScore;
}

int16_t SearchTree::pvSearch(int16_t depth, int16_t ply, int16_t alpha, int16_t beta) {
    if(!searching)
        return 0;

    if(evaluator.isDraw()) {
        pvTable[ply].clear();
        return 0;
    }

    if(depth <= 0) {
        int32_t lastMovedSquare = board->getLastMove().getDestination();
        int16_t score = quiescence(alpha, beta, lastMovedSquare);
        return score;
    }

    pvTable[ply + 1].clear();

    TranspositionTableEntry ttEntry;
    
    bool searchPv = true;
    int16_t score, bestScore = MIN_SCORE;
    Move bestMove;
    
    Array<Move, 256> moves = board->generateLegalMoves();

    if(moves.size() == 0) {
        pvTable[ply].clear();

        if(board->isCheck())
            return -MATE_SCORE + ply;
        else
            return 0;
    }

    int32_t moveNumber = 1;
    bool isCheckEvasion = board->isCheck();

    sortMoves(moves, ply, SEE);

    for(Move move : moves) {
        if(!searching)
            return 0;
        
        nodesSearched++;

        board->makeMove(move);

        int16_t extension = determineExtension(depth, move, moveNumber, moves.size(), isCheckEvasion);
        int16_t nwReduction = determineReduction(depth, move, moveNumber, moves.size(), isCheckEvasion);

        int16_t nwDepthDelta = -ONE_PLY - nwReduction + extension;
        nwDepthDelta = std::min(nwDepthDelta, (int16_t)(-ONE_PLY));

        if(searchPv) {
            score = -pvSearch(depth - ONE_PLY, ply + 1, -beta, -alpha);
        } else {
            score = -nwSearch(depth + nwDepthDelta, ply + 1, -alpha - 1, -alpha);
            if(score > alpha)
                score = -pvSearch(depth - ONE_PLY, ply + 1, -beta, -alpha);
        }

        board->undoMove();

        if(!searching)
            return 0;
        
        relativeHistory[board->getSideToMove() / COLOR_MASK]
                           [board->sq120To64(move.getOrigin())]
                           [board->sq120To64(move.getDestination())] -= depth / ONE_PLY;
        

        if(score >= beta) {
            bool tableHit = transpositionTable.probe(board->getHashValue(), ttEntry);

            if(!tableHit || (depth > ttEntry.depth) &&
                             ttEntry.type != PV_NODE | EXACT_NODE)
                transpositionTable.put(board->getHashValue(), {
                    currentAge, depth, score, CUT_NODE, move
                });

            if(move.isQuiet()) {
                if(killerMoves[ply][0] != move) {
                    killerMoves[ply][1] = killerMoves[ply][0];
                    killerMoves[ply][0] = move;
                }
            }

            relativeHistory[board->getSideToMove() / COLOR_MASK]
                           [board->sq120To64(move.getOrigin())]
                           [board->sq120To64(move.getDestination())] += 1 << (depth / ONE_PLY);

            return score;
        }

        if(score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
        
        if(score > alpha) {
            alpha = score;
            pvTable[ply].clear();
            pvTable[ply].push_back(move);
            pvTable[ply].push_back(pvTable[ply + 1]);
        }

        searchPv = false;
        moveNumber++;
    }

    bool tableHit = transpositionTable.probe(board->getHashValue(), ttEntry);

    if(!tableHit || (depth > ttEntry.depth))
        transpositionTable.put(board->getHashValue(), {
            currentAge, depth, bestScore, EXACT_NODE, bestMove
        });

    relativeHistory[board->getSideToMove() / COLOR_MASK]
                    [board->sq120To64(bestMove.getOrigin())]
                    [board->sq120To64(bestMove.getDestination())] += 1 << (depth / ONE_PLY);

    return bestScore;
}

int16_t SearchTree::nwSearch(int16_t depth, int16_t ply, int16_t alpha, int16_t beta) {
    if(!searching)
        return 0;

    if(evaluator.isDraw())
        return 0;

    if(depth <= 0) {
        int32_t lastMovedSquare = board->getLastMove().getDestination();
        int16_t score = quiescence(alpha, beta, lastMovedSquare);
        return score;
    }

    TranspositionTableEntry ttEntry;
    bool tableHit = transpositionTable.probe(board->getHashValue(), ttEntry);

    if(tableHit) {
        if(ttEntry.depth >= depth) {
            switch(NODE_TYPE(ttEntry.type)) {
                case EXACT_NODE:
                    return ttEntry.score;
                case CUT_NODE:
                    if(ttEntry.score >= beta)
                        return ttEntry.score;
            }
        }
    }
    
    int16_t bestScore = MIN_SCORE;
    Move bestMove;
    
    Array<Move, 256> moves = board->generateLegalMoves();

    if(moves.size() == 0) {
        if(board->isCheck())
            return -MATE_SCORE + ply;
        else
            return 0;
    }

    sortMoves(moves, ply, SEE);

    int32_t moveNumber = 1;
    bool isCheckEvasion = board->isCheck();

    for(Move move : moves) {
        if(!searching)
            return 0;
        
        nodesSearched++;

        board->makeMove(move);

        int16_t extension = determineExtension(depth, move, moveNumber, moves.size(), isCheckEvasion);
        int16_t nwReduction = determineReduction(depth, move, moveNumber, moves.size(), isCheckEvasion);

        int16_t nwDepthDelta = -ONE_PLY - nwReduction + extension;
        nwDepthDelta = std::min(nwDepthDelta, (int16_t)(-ONE_PLY));

        int16_t score = -nwSearch(depth + nwDepthDelta, ply + 1, -beta, -alpha);

        // Wenn eine stark reduzierte Suche eine Bewertung > alpha liefert, dann
        // wird eine vollständige Suche durchgeführt.
        if(nwDepthDelta < -ONE_PLY && score > alpha)
            score = -nwSearch(depth - ONE_PLY, ply + 1, -beta, -alpha);

        board->undoMove();

        if(!searching)
            return 0;
        
        relativeHistory[board->getSideToMove() / COLOR_MASK]
                        [board->sq120To64(move.getOrigin())]
                        [board->sq120To64(move.getDestination())] -= depth / ONE_PLY;

        if(score >= beta) {
            bool tableHit = transpositionTable.probe(board->getHashValue(), ttEntry);

            if(!tableHit || (depth > ttEntry.depth &&
                               ttEntry.type == NW_NODE | CUT_NODE))
                transpositionTable.put(board->getHashValue(), {
                    currentAge, depth, score, NW_NODE | CUT_NODE, move
                });
            
            if(move.isQuiet()) {
                if(killerMoves[ply][0] != move) {
                    killerMoves[ply][1] = killerMoves[ply][0];
                    killerMoves[ply][0] = move;
                }
            }

            relativeHistory[board->getSideToMove() / COLOR_MASK]
                           [board->sq120To64(move.getOrigin())]
                           [board->sq120To64(move.getDestination())] += 1 << (depth / ONE_PLY);

            return score;
        }
        
        if(score > bestScore) {
            bestScore = score;
            bestMove = move;
        }

        moveNumber++;
    }

    if(!tableHit || (depth > ttEntry.depth &&
                        !IS_REGULAR_NODE(ttEntry.type)))
        transpositionTable.put(board->getHashValue(), {
            currentAge, depth, bestScore, NW_NODE | EXACT_NODE, bestMove
        });
    
    relativeHistory[board->getSideToMove() / COLOR_MASK]
                [board->sq120To64(bestMove.getOrigin())]
                [board->sq120To64(bestMove.getDestination())] += 1 << (depth / ONE_PLY);

    return bestScore;
}

int16_t SearchTree::determineExtension(int16_t depth, Move& m, int32_t moveCount, int32_t legalMoveCount, bool isCheckEvasion) {
    int16_t extension = 0;

    int32_t movedPieceType = TYPEOF(board->pieceAt(m.getDestination()));
    int32_t capturedPieceType = TYPEOF(board->getLastMoveHistoryEntry().capturedPiece);

    int32_t origin64 = board->sq120To64(m.getOrigin());
    int32_t destination64 = board->sq120To64(m.getDestination());

    bool isCheck = board->isCheck();

    // Erweiterungen

    // Schach
    if(isCheck || isCheckEvasion)
        extension += ONE_PLY * 2;
    
    // Schlagzug oder Bauernumwandlung
    if(!m.isQuiet()) {
        int32_t seeScore = MIN_SCORE;
        bool seeCacheHit = seeCache.probe(m, seeScore);

        if(!seeCacheHit || seeScore >= 0)
            extension += ONE_PLY; // Höhere Erweiterung wenn der Schlagzug eine gute SEE-Bewertung hat
        else
            extension += ONE_HALF_PLY;
    }

    // Freibauerzüge
    if(movedPieceType == PAWN) {
        int32_t side = board->getSideToMove() ^ COLOR_MASK;
        int32_t otherSide = board->getSideToMove();

        if(!(sentryMasks[side / COLOR_MASK][board->sq120To64(m.getDestination())]
            & board->getPieceBitboard(otherSide | PAWN)))
            extension += TWO_THIRDS_PLY;
    }

    // Wenn eine Figur sich in Sicherheit bewegt 
    if(board->squareAttacked(origin64, board->getSideToMove()) &&
                !board->squareAttacked(destination64, board->getSideToMove())) {
        extension += ONE_THIRD_PLY;
    }
    
    return extension;
}

int16_t SearchTree::determineReduction(int16_t depth, Move& m, int32_t moveCount, int32_t legalMoveCount, bool isCheckEvasion) {
    int16_t reduction = 0;

    bool isCheck = board->isCheck();

    int32_t movedPieceType = TYPEOF(board->pieceAt(m.getDestination()));
    int32_t capturedPieceType = TYPEOF(board->getLastMoveHistoryEntry().capturedPiece);

    int32_t origin64 = board->sq120To64(m.getOrigin());
    int32_t destination64 = board->sq120To64(m.getDestination());
    
    if(moveCount <= 3 || isCheck || isCheckEvasion)
        return 0;

    reduction += (int16_t)(sqrt(depth) + sqrt(moveCount - 1) * ONE_PLY);
    
    return reduction;
}

int16_t SearchTree::quiescence(int16_t alpha, int16_t beta, int32_t captureSquare) {
    if(!searching)
        return 0;

    int16_t score = (int16_t)evaluator.evaluate();

    if(score >= beta)
        return score;
    
    if(score > alpha)
        alpha = score;
    
    int16_t bestScore = score;
    
    Array<Move, 256> moves;

    if(board->isCheck()) {
        moves = board->generateLegalMoves();
        if(moves.size() == 0)
            return -MATE_SCORE;

        sortAndCutMoves(moves, MIN_SCORE, MVVLVA);
    }
    else {
        moves = board->generateLegalCaptures();

        std::remove_if(moves.begin(), moves.end(), [captureSquare](Move m) {
            return m.getDestination() != captureSquare;
        });

        sortAndCutMoves(moves, QUIESCENCE_SCORE_CUTOFF, MVVLVA);
    }
    
    for(Move move : moves) {
        if(!searching)
            return 0;
        
        nodesSearched++;

        board->makeMove(move);

        score = -quiescence(-beta, -alpha, captureSquare);

        board->undoMove();

        if(!searching)
            return 0;

        if(score >= beta)
            return score;
        
        if(score > bestScore)
            bestScore = score;
        
        if(score > alpha)
            alpha = score;
    }

    return bestScore;
}