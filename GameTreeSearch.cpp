#include "GameTreeSearch.h"
#include <algorithm>

GameTreeSearch::GameTreeSearch(Board& board) {
    this->board = &board;
    this->evaluator = BoardEvaluator(board);
}

GameTreeSearch::~GameTreeSearch() {
    
}

struct MoveScorePair {
    Move move;
    int32_t score;

    bool operator<(const MoveScorePair& other) const {
        return score < other.score;
    }

    bool operator>(const MoveScorePair& other) const {
        return score > other.score;
    }

    bool operator==(const MoveScorePair& other) const {
        return score == other.score;
    }
};

void GameTreeSearch::sortMoves(Array<Move, 256>& moves) {
    TranspositionTableEntry ttResult;
    bool ttHit = transpositionTable.probe(board->getHashValue(), ttResult);

    Move hashMove;

    if(ttHit)
        hashMove = ttResult.hashMove;

    Array<MoveScorePair, 256> moveScorePairs;

    for(Move m : moves) {
        if(m == hashMove)
            moveScorePairs.push_back({m, HASH_MOVE_SCORE});
        else
            moveScorePairs.push_back({m, evaluator.evaluateMove(m)});
    }

    std::sort(moveScorePairs.begin(), moveScorePairs.end(), std::greater<MoveScorePair>());

    moves.clear();

    for(MoveScorePair msp : moveScorePairs) {
        moves.push_back(msp.move);
    }
}

int32_t GameTreeSearch::pvSearchInit(uint8_t depth) {
    if(depth == 0) {
        return evaluator.evaluate();
    }

    int32_t alpha = MIN_SCORE;
    int32_t beta = MAX_SCORE;

    Array<Move, 256> moves = board->generateLegalMoves();

    if(moves.size() == 0) {
        if(board->isCheck())
            return -MATE_SCORE;
        else
            return 0;
    }

    sortMoves(moves);

    uint8_t ttFlags = ALL_NODE;

    int32_t score = MIN_SCORE;

    bool searchPV = true;
    Move bestMove;

    for(Move m : moves) {
        board->makeMove(m);

        if(searchPV) {
            score = -pvSearch(depth - 1, -beta, -alpha);

        } else {
            score = -nwSearch(depth - 1, -alpha);

            if(score > alpha) {
                score = -pvSearch(depth - 1, -beta, -alpha);
            }
        }

        board->undoMove();

        if(score >= beta) {
            TranspositionTableEntry ttEntry;
            ttEntry.score = beta;
            ttEntry.depth = depth;
            ttEntry.flags = CUT_NODE;
            ttEntry.hashMove = m;

            transpositionTable.put(board->getHashValue(), ttEntry);

            return beta;
        }

        if(score > alpha) {
            bestMove = m;
            alpha = score;
            ttFlags = PV_NODE;

            int32_t plyFromRoot = currentDepth - depth;

            pvTable[plyFromRoot].clear();
            pvTable[plyFromRoot].push_back(m);
            pvTable[plyFromRoot].push_back(pvTable[plyFromRoot + 1]);
        }

        searchPV = false;
    }

    TranspositionTableEntry ttEntry;
    ttEntry.score = alpha;
    ttEntry.depth = depth;
    ttEntry.flags = ttFlags;

    if(ttFlags == PV_NODE)
        ttEntry.hashMove = bestMove;

    transpositionTable.put(board->getHashValue(), ttEntry);

    return alpha;
}

int32_t GameTreeSearch::pvSearch(uint8_t depth, int32_t alpha, int32_t beta) {
    int32_t plyFromRoot = currentDepth - depth;

    TranspositionTableEntry ttResult;

    bool ttHit = transpositionTable.probe(board->getHashValue(), ttResult);

    if(ttHit && ttResult.depth >= depth && IS_REGULAR_NODE(ttResult.flags)) {
        switch(ttResult.flags) {
            case PV_NODE:
                return ttResult.score;
            case ALL_NODE:
                if(ttResult.score <= alpha) {
                    return alpha;
                } else {
                    alpha = ttResult.score;
                }
                break;
            case CUT_NODE:
                if(ttResult.score >= beta) {
                    return beta;
                } else {
                    beta = ttResult.score;
                }
                break;
        }
    }

    if(depth == 0) {
        return evaluator.evaluate();
    }

    Array<Move, 256> moves = board->generateLegalMoves();

    if(moves.size() == 0) {
        if(board->isCheck())
            return -MATE_SCORE;
        else
            return 0;
    }

    sortMoves(moves);

    uint8_t ttFlags = ALL_NODE;

    int32_t score = MIN_SCORE;

    bool searchPV = true;
    Move bestMove;

    for(Move m : moves) {
        board->makeMove(m);

        if(searchPV) {
            score = -pvSearch(depth - 1, -beta, -alpha);
        } else {
            score = -nwSearch(depth - 1, -alpha);

            if(score > alpha) {
                score = -pvSearch(depth - 1, -beta, -alpha);
            }
        }

        board->undoMove();

        if(score >= beta) {
            if((ttHit && ttResult.depth < depth && ttResult.flags != PV_NODE) || !ttHit) {
                TranspositionTableEntry ttEntry;
                ttEntry.score = beta;
                ttEntry.depth = depth;
                ttEntry.flags = CUT_NODE;
                ttEntry.hashMove = m;

                transpositionTable.put(board->getHashValue(), ttEntry);
            }

            return beta;
        }

        if(score > alpha) {
            bestMove = m;
            alpha = score;
            ttFlags = PV_NODE;

            pvTable[plyFromRoot].clear();
            pvTable[plyFromRoot].push_back(m);
            pvTable[plyFromRoot].push_back(pvTable[plyFromRoot + 1]);
        }

        searchPV = false;
    }

    if((ttHit && ttResult.depth < depth && !(ttFlags == ALL_NODE && ttResult.flags == PV_NODE)) || !ttHit) {
        TranspositionTableEntry ttEntry;
        ttEntry.score = alpha;
        ttEntry.depth = depth;
        ttEntry.flags = ttFlags;

        if(ttFlags == PV_NODE)
            ttEntry.hashMove = bestMove;

        transpositionTable.put(board->getHashValue(), ttEntry);
    }

    return alpha;
}

int32_t GameTreeSearch::nwSearch(uint8_t depth, int32_t beta) {
    int32_t plyFromRoot = currentDepth - depth;

    TranspositionTableEntry ttResult;

    bool ttHit = transpositionTable.probe(board->getHashValue(), ttResult);

    if(ttHit && ttResult.depth >= depth) {
        switch(ttResult.flags) {
            case PV_NODE:
                if(ttResult.score >= beta) {
                    return beta;
                } else {
                    return beta - 1;
                }
            case ALL_NODE:
                return beta - 1;
            case CUT_NODE:
                return beta;
        }
    }

    if(depth == 0) {
        return evaluator.evaluate();
    }
    Array<Move, 256> moves = board->generateLegalMoves();

    if(moves.size() == 0) {
        if(board->isCheck())
            return -MATE_SCORE;
        else
            return 0;
    }

    sortMoves(moves);

    int32_t score = MIN_SCORE;

    for(Move m : moves) {
        board->makeMove(m);
        score = -nwSearch(depth - 1, -beta + 1);
        board->undoMove();

        if(score >= beta) {
            if((ttHit && ttResult.depth < depth && !IS_REGULAR_NODE(ttResult.flags)) || !ttHit) {
                TranspositionTableEntry ttEntry;
                ttEntry.score = beta;
                ttEntry.depth = depth;
                ttEntry.flags = NULL_WINDOW_NODE | CUT_NODE;
                ttEntry.hashMove = m;

                transpositionTable.put(board->getHashValue(), ttEntry);
            }
            
            return beta;
        }
    }

    return beta - 1;
}

int32_t GameTreeSearch::search(uint8_t depth, std::vector<Move>& pv) {
    int32_t score = 0;

    for(uint8_t i = (depth % 2 == 0 ? 2 : 1); i <= depth; i += 2) {
        currentDepth = i;
        score = pvSearchInit(i);
    }

    pv.clear();

    for(Move m : pvTable[0]) {
        pv.push_back(m);
    }

    return score;
}