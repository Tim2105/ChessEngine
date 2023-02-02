#include "SearchTree.h"
#include <thread>
#include <algorithm>
#include "EvaluationDefinitions.h"
#include <cmath>

struct MoveScorePair {
    Move move;
    int32_t score;
};

template<>
struct std::greater<MoveScorePair> {
    bool operator()(const MoveScorePair& lhs, const MoveScorePair& rhs) {
        return lhs.score > rhs.score;
    }
};

template<>
struct std::less<MoveScorePair> {
    bool operator()(const MoveScorePair& lhs, const MoveScorePair& rhs) {
        return lhs.score < rhs.score;
    }
};

SearchTree::SearchTree(Board& b) {
    board = &b;
    evaluator = BoardEvaluator(b);

    currentMaxDepth = 0;
    nodesSearched = 0;

    searching = false;
}

std::vector<Move> SearchTree::findPrincipalVariation() {
    std::vector<Move> pv;

    uint64_t hash = board->getHashValue();
    TranspositionTableEntry ttEntry;

    bool moveFound = transpositionTable.probe(hash, ttEntry);
    Move hashMove = ttEntry.hashMove;

    pv.push_back(hashMove);

    return pv;
}

void SearchTree::searchTimer(uint32_t searchTime) {
    std::this_thread::sleep_for(std::chrono::milliseconds(searchTime));
    searching = false;
}

int16_t SearchTree::search(uint32_t searchTime) {
    searching = true;
    currentMaxDepth = 0;
    nodesSearched = 0;

    int16_t lastScore;

    transpositionTable.clear();

    std::thread timer(std::bind(&SearchTree::searchTimer, this, searchTime));

    int16_t score = evaluator.evaluate();

    auto start = std::chrono::high_resolution_clock::now();

    for(int8_t depth = ONE_PLY / 2; searching; depth += ONE_PLY) {
        currentMaxDepth = depth;
        score = rootSearch(depth, score);

        auto now = std::chrono::high_resolution_clock::now();

        std::cout << "(" << (now - start).count() / 1000000 << "ms)" << "Depth: " << (int32_t)ceil((float)depth / ONE_PLY) << " Score: " << score
            << " Nodes: " << nodesSearched << " PV:";
        
        if(searching) {
            lastScore = score;
            principalVariation = findPrincipalVariation();
            for(Move move : principalVariation)
                std::cout << " " << move;
        }
        
        std::cout << std::endl;
    }

    timer.join();

    return lastScore;
}

void SearchTree::sortMoves(Array<Move, 256>& moves, int8_t depth, int32_t moveEvalFunc) {
    Array<MoveScorePair, 256> msp;

    TranspositionTableEntry ttEntry;
    bool hashHit = transpositionTable.probe(board->getHashValue(), ttEntry);

    for(Move move : moves) {
        if(move == ttEntry.hashMove)
            msp.push_back({move, 30000});
        else if(move.isCapture() || move.isPromotion()) {
            int32_t moveScore = 0;

            switch(moveEvalFunc) {
                case MVVLVA:
                    moveScore += evaluator.evaluateMoveMVVLVA(move);
                    break;
                case SEE:
                    moveScore += evaluator.evaluateMoveSEE(move);
                    break;
            }

            msp.push_back({move, moveScore});
        } else if(move.isQuiet()) {
            int8_t ply = currentMaxDepth - depth;
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

            msp.push_back({move, moveScore});
        }
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
                moveScore += evaluator.evaluateMoveSEE(move);
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

void SearchTree::sortAndCutMoves(Array<Move, 256>& moves, int8_t depth, int32_t minScore, int32_t moveEvalFunc) {
    Array<MoveScorePair, 256> msp;

    TranspositionTableEntry ttEntry;
    bool hashHit = transpositionTable.probe(board->getHashValue(), ttEntry);

    for(Move move : moves) {
        if(move == ttEntry.hashMove)
            msp.push_back({move, 30000});
        else if(move.isCapture() || move.isPromotion()) {
            int32_t moveScore = 0;

            switch(moveEvalFunc) {
                case MVVLVA:
                    moveScore += evaluator.evaluateMoveMVVLVA(move);
                    break;
                case SEE:
                    moveScore += evaluator.evaluateMoveSEE(move);
                    break;
            }
            if(moveScore >= minScore)
                msp.push_back({move, moveScore});
        } else if(move.isQuiet()) {
            int8_t ply = currentMaxDepth - depth;
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

            if(moveScore >= minScore)
                msp.push_back({move, moveScore});
        }
    }

    std::sort(msp.begin(), msp.end(), std::greater<MoveScorePair>());

    moves.clear();

    for(MoveScorePair pair : msp)
        moves.push_back(pair.move);
}

int16_t SearchTree::rootSearch(int8_t depth, int16_t expectedScore) {
    int16_t alpha = expectedScore - ASP_WINDOW;
    int16_t beta = expectedScore + ASP_WINDOW;

    int16_t score = pvSearch(depth, alpha, beta);
    int32_t aspAlphaReduction = ASP_WINDOW, numAlphaReduction = 1;
    int32_t aspBetaReduction = ASP_WINDOW, numBetaReduction = 1;

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

        score = pvSearch(depth, alpha, beta);
    }
    
    return score;
}

int16_t SearchTree::pvSearch(int8_t depth, int16_t alpha, int16_t beta) {
    if(!searching)
        return 0;

    if(evaluator.isDraw())
        return 0;

    if(depth <= 0) {
        int32_t lastMovedSquare = board->getLastMove().getDestination();
        int16_t score = quiescence(alpha, beta, lastMovedSquare);
        //int16_t score = evaluator.evaluate();
        return score;
    }

    TranspositionTableEntry ttEntry;
    bool tableHit = transpositionTable.probe(board->getHashValue(), ttEntry);

    if(tableHit) {
        if(ttEntry.depth >= depth && IS_REGULAR_NODE(ttEntry.type)) {
            switch(NODE_TYPE(ttEntry.type)) {
                case EXACT_NODE:
                    return ttEntry.score;
                case CUT_NODE:
                    if(ttEntry.score >= beta)
                        return ttEntry.score;
            }
        }
    }
    
    bool searchPv = true;
    int16_t score, bestScore = MIN_SCORE;
    Move bestMove;

    Array<Move, 256> moves = board->generateLegalMoves();

    if(moves.size() == 0) {
        if(board->isCheck())
            return -MATE_SCORE;
        else
            return 0;
    }

    int32_t moveNumber = 1;

    sortMoves(moves, depth, SEE);

    for(Move move : moves) {
        if(!searching)
            return 0;
        
        nodesSearched++;

        board->makeMove(move);

        int8_t extension = determineExtension(depth, move);

        if(searchPv) {
            score = -pvSearch(depth - ONE_PLY + extension, -beta, -alpha);
        } else {
            int8_t reduction = determineReduction(depth, move, moveNumber);

            score = -nwSearch(depth - ONE_PLY + extension - reduction, -alpha - 1, -alpha);
            if(score > alpha && score < beta)
                score = -pvSearch(depth - ONE_PLY, + extension -beta, -alpha);
        }

        board->undoMove();

        if(!searching)
            return 0;

        if(score >= beta) {
            tableHit = transpositionTable.probe(board->getHashValue(), ttEntry);

            if(!tableHit || (depth > ttEntry.depth) &&
                             ttEntry.type != PV_NODE | EXACT_NODE)
                transpositionTable.put(board->getHashValue(), {
                    depth, score, CUT_NODE, move
                });
            
            int8_t ply = currentMaxDepth - depth;
            if(move.isQuiet()) {
                if(killerMoves[ply][0] != move) {
                    killerMoves[ply][1] = killerMoves[ply][0];
                    killerMoves[ply][0] = move;
                }
            }

            return score;
        }
        
        if(score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
        
        if(score > alpha)
            alpha = score;

        searchPv = false;
        moveNumber++;
    }

    tableHit = transpositionTable.probe(board->getHashValue(), ttEntry);

    if(!tableHit || (depth > ttEntry.depth))
        transpositionTable.put(board->getHashValue(), {
            depth, bestScore, EXACT_NODE, bestMove
        });

    return bestScore;
}

int16_t SearchTree::nwSearch(int8_t depth, int16_t alpha, int16_t beta) {
    if(!searching)
        return 0;

    if(evaluator.isDraw())
        return 0;

    if(depth <= 0) {
        int32_t lastMovedSquare = board->getLastMove().getDestination();
        int16_t score = quiescence(alpha, beta, lastMovedSquare);
        //int16_t score = evaluator.evaluate();
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
            return -MATE_SCORE;
        else
            return 0;
    }

    int8_t ply = currentMaxDepth - depth;
    int32_t moveNumber = 1;

    sortMoves(moves, depth, SEE);

    for(Move move : moves) {
        if(!searching)
            return 0;
        
        nodesSearched++;

        board->makeMove(move);

        int8_t extension = determineExtension(depth, move);
        int8_t reduction = determineReduction(depth, move, moveNumber++);

        int16_t score = -nwSearch(depth - ONE_PLY + extension - reduction, -beta, -alpha);

        board->undoMove();

        if(!searching)
            return 0;

        if(score >= beta) {
            bool tableHit = transpositionTable.probe(board->getHashValue(), ttEntry);

            if(!tableHit || (depth > ttEntry.depth &&
                               ttEntry.type == NW_NODE | CUT_NODE))
                transpositionTable.put(board->getHashValue(), {
                    depth, score, NW_NODE | CUT_NODE, move
                });
            
            int8_t ply = currentMaxDepth - depth;
            if(move.isQuiet()) {
                if(killerMoves[ply][0] != move) {
                    killerMoves[ply][1] = killerMoves[ply][0];
                    killerMoves[ply][0] = move;
                }
            }

            return score;
        }
        
        if(score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
    }

    tableHit = transpositionTable.probe(board->getHashValue(), ttEntry);

    if(!tableHit || (depth > ttEntry.depth &&
                        !IS_REGULAR_NODE(ttEntry.type)))
        transpositionTable.put(board->getHashValue(), {
            depth, bestScore, NW_NODE | EXACT_NODE, bestMove
        });

    return bestScore;
}

int8_t SearchTree::determineExtension(int8_t depth, Move& m) {
    int8_t ply = currentMaxDepth - depth;

    int8_t extension = 0;

    int32_t movedPieceType = TYPEOF(board->pieceAt(m.getDestination()));
    int32_t capturedPieceType = TYPEOF(board->getLastMoveHistoryEntry().capturedPiece);

    bool isCheck = board->isCheck();
    bool isCheckEvasion = false;
    int32_t kingPos = board->getPieceList((board->getSideToMove() ^ COLOR_MASK) | KING).front();
    if(board->squareAttacked(kingPos, board->getSideToMove()) ||
       (movedPieceType == KING && board->squareAttacked(m.getOrigin(), board->getSideToMove())))
        isCheckEvasion = true;

    // Extensions
    if(isCheck || isCheckEvasion)
        extension += FIVE_SIXTHS_PLY;
    else if(m.isCapture())
        extension += TWO_THIRDS_PLY;
    else if(m.isPromotion())
        extension += ONE_HALF_PLY;
    else if(movedPieceType == PAWN) {
        extension += ONE_SIXTH_PLY;
    }
    
    return extension;
}

int8_t SearchTree::determineReduction(int8_t depth, Move& m, int32_t moveNumber) {
    int8_t ply = currentMaxDepth - depth;

    int8_t reduction = 0;

    int32_t movedPieceType = TYPEOF(board->pieceAt(m.getDestination()));
    int32_t capturedPieceType = TYPEOF(board->getLastMoveHistoryEntry().capturedPiece);

    bool isCheck = board->isCheck();
    bool isCheckEvasion = false;
    int32_t kingPos = board->getPieceList((board->getSideToMove() ^ COLOR_MASK) | KING).front();
    if(board->squareAttacked(kingPos, board->getSideToMove()) ||
       (movedPieceType == KING && board->squareAttacked(m.getOrigin(), board->getSideToMove())))
        isCheckEvasion = true;

    // Reductions
    if(ply >= FULL_MOVE_DEPTH && moveNumber > UNREDUCED_MOVES && m.isQuiet() && !isCheck && !isCheckEvasion) {
        reduction += (int8_t)floor((moveNumber - UNREDUCED_MOVES) * 0.3 * ONE_PLY);
    }
    
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