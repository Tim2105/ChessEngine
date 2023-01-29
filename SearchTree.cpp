#include "SearchTree.h"
#include <thread>
#include <algorithm>

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

std::vector<Move> SearchTree::getPrincipalVariation() {
    std::vector<Move> pv;

    uint64_t hash = board->getHashValue();
    TranspositionTableEntry ttEntry;

    bool moveFound = transpositionTable.probe(hash, ttEntry);
    Move hashMove = ttEntry.hashMove;

    while(moveFound) {
        pv.push_back(hashMove);

        board->makeMove(hashMove);
        hash = board->getHashValue();

        moveFound = transpositionTable.probe(hash, ttEntry);
        hashMove = ttEntry.hashMove;
    }

    for(int i = 0; i < pv.size(); i++)
        board->undoMove();

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

    std::thread timer(std::bind(&SearchTree::searchTimer, this, searchTime));

    int16_t score = evaluator.evaluate();

    auto start = std::chrono::high_resolution_clock::now();

    for(int8_t depth = 1; searching; depth++) {
        currentMaxDepth = depth;
        score = rootSearch(depth, score);

        auto now = std::chrono::high_resolution_clock::now();

        std::cout << "(" << (now - start).count() / 1000000 << "ms)" << "Depth: " << (int)depth << " Score: " << score
            << " Nodes: " << nodesSearched << " PV:";
        
        std::vector<Move> pv = getPrincipalVariation();
        for(Move move : pv)
            std::cout << " " << move;
        
        std::cout << std::endl;
    }

    timer.join();

    return score;
}

void SearchTree::sortMoves(Array<Move, 256>& moves, int8_t depth) {
    Array<MoveScorePair, 256> msp;

    TranspositionTableEntry ttEntry;
    transpositionTable.probe(board->getHashValue(), ttEntry);

    for(Move move : moves) {
        if(move == ttEntry.hashMove)
            msp.push_back({move, 20000});
        else {
            int32_t moveScore = evaluator.evaluateMove(move);

            if(move.isQuiet()) {
                int8_t ply = currentMaxDepth - depth;

                if(killerMoves[ply][0] == move || killerMoves[ply][1] == move)
                    moveScore += 80;
                else if(ply >= 2) {
                    if(killerMoves[ply - 2][0] == move || killerMoves[ply - 2][1] == move)
                        moveScore += 80;
                }
            }

            msp.push_back({move, moveScore});
        }
    }

    std::sort(msp.begin(), msp.end(), std::greater<MoveScorePair>());

    moves.clear();

    for(MoveScorePair msPair : msp) {
        moves.push_back(msPair.move);
    }
}

void SearchTree::sortAndCutMoves(Array<Move, 256>& moves, int16_t minScore) {
    Array<MoveScorePair, 256> msp;

    for(Move move : moves) {
        int32_t moveScore = evaluator.evaluateMove(move);
        if(moveScore >= minScore)
            msp.push_back({move, moveScore});
    }

    std::sort(msp.begin(), msp.end(), std::greater<MoveScorePair>());

    moves.clear();

    for(MoveScorePair msPair : msp) {
        moves.push_back(msPair.move);
    }
}

int16_t SearchTree::rootSearch(int8_t depth, int16_t expectedScore) {

    int16_t alpha = expectedScore - 50;
    int16_t beta = expectedScore + 50;

    int16_t score = pvSearch(depth, alpha, beta);

    if(score <= alpha || score >= beta)
        score = pvSearch(depth, MIN_SCORE, MAX_SCORE);
    
    return score;
}

int16_t SearchTree::pvSearch(int8_t depth, int16_t alpha, int16_t beta) {
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

    if(depth <= 0)
        return quiescence(alpha, beta);
    
    bool searchPv = true;
    int16_t score, bestScore = MIN_SCORE;
    Move bestMove;

    Array<Move, 256> moves = board->generateLegalMoves();

    if(moves.size() == 0) {
        if(board->isCheck())
            return MIN_SCORE;
        else
            return 0;
    }

    sortMoves(moves, depth);

    for(Move move : moves) {
        if(!searching)
            return 0;
        
        nodesSearched++;

        board->makeMove(move);

        if(searchPv) {
            score = -pvSearch(depth - 1, -beta, -alpha);
        } else {
            score = -nwSearch(depth - 1, -alpha - 1, -alpha);
            if(score > alpha && score < beta)
                score = -pvSearch(depth - 1, -beta, -alpha);
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
    }

    tableHit = transpositionTable.probe(board->getHashValue(), ttEntry);

    if(!tableHit || (depth > ttEntry.depth))
        transpositionTable.put(board->getHashValue(), {
            depth, bestScore, EXACT_NODE, bestMove
        });

    return bestScore;
}

int16_t SearchTree::nwSearch(int8_t depth, int16_t alpha, int16_t beta) {
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

    if(depth <= 0)
        return quiescence(alpha, beta);
    
    int16_t bestScore = MIN_SCORE;
    Move bestMove;
    
    Array<Move, 256> moves = board->generateLegalMoves();

    if(moves.size() == 0) {
        if(board->isCheck())
            return MIN_SCORE;
        else
            return 0;
    }

    sortMoves(moves, depth);

    for(Move move : moves) {
        if(!searching)
            return 0;
        
        nodesSearched++;

        board->makeMove(move);

        int16_t score = -nwSearch(depth - 1, -beta, -alpha);

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
            if(move.isQuiet() && killerMoves[ply][0] != move) {
                killerMoves[ply][1] = killerMoves[ply][0];
                killerMoves[ply][0] = move;
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

int16_t SearchTree::quiescence(int16_t alpha, int16_t beta) {
    int16_t score = (int16_t)evaluator.evaluate();

    if(score >= beta)
        return score;
    
    if(score > alpha)
        alpha = score;
    
    int16_t bestScore = score;
    
    Array<Move, 256> moves;

    if(board->isCheck())
        moves = board->generateLegalMoves();
    else
        moves = board->generateLegalCaptures();
    
    sortAndCutMoves(moves, 0);
    
    for(Move move : moves) {
        if(!searching)
            return 0;
        
        nodesSearched++;

        board->makeMove(move);

        score = -quiescence(-beta, -alpha);

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