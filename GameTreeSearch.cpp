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

int32_t GameTreeSearch::pvSearch(uint8_t depth, int32_t alpha, int32_t beta, Array<Move, MAX_DEPTH>& pv) {

    TranspositionTableEntry ttResult;

    bool ttHit = transpositionTable.probe(board->getHashValue(), ttResult);

    if(ttHit && ttResult.depth >= depth) {
        switch(ttResult.flags) {
            case PV_NODE:
                pv.clear();
                pv.push_back(ttResult.hashMove);

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
    sortMoves(moves);

    uint8_t ttFlags = ALL_NODE;

    int32_t score = MIN_SCORE;

    bool searchPV = true;
    Array<Move, MAX_DEPTH> childPV;

    for(Move m : moves) {
        board->makeMove(m);

        if(searchPV) {
            score = -pvSearch(depth - 1, -beta, -alpha, childPV);
        } else {
            score = -nwSearch(depth - 1, -alpha - 1, -alpha);

            if(score > alpha && score < beta) {
                score = -pvSearch(depth - 1, -beta, -alpha, childPV);
            }
        }

        board->undoMove();

        if(score >= beta) {
            if(ttHit && ttResult.depth < depth) {
                TranspositionTableEntry ttEntry;
                ttEntry.score = beta;
                ttEntry.depth = depth;
                ttEntry.flags = CUT_NODE;
                ttEntry.hashMove = m;

                transpositionTable.put(board->getHashValue(), ttEntry);
            }

            return score;
        }

        if(score > alpha) {
            alpha = score;
            ttFlags = PV_NODE;
            pv.clear();
            pv.push_back(m);
            pv.push_back(childPV);
        }

        searchPV = false;
    }

    if(ttHit && ttResult.depth < depth) {  
        TranspositionTableEntry ttEntry;
        ttEntry.score = alpha;
        ttEntry.depth = depth;
        ttEntry.flags = ttFlags;

        if(ttFlags == PV_NODE)
            ttEntry.hashMove = pv.front();

        transpositionTable.put(board->getHashValue(), ttEntry);
    }

    return alpha;
}

int32_t GameTreeSearch::nwSearch(uint8_t depth, int32_t alpha, int32_t beta) {   
    if(depth == 0) {
        return evaluator.evaluate();
    }

    Array<Move, 256> moves = board->generateLegalMoves();
    sortMoves(moves);

    int32_t score = MIN_SCORE;

    for(Move m : moves) {
        board->makeMove(m);
        score = -nwSearch(depth - 1, -beta, -alpha);
        board->undoMove();

        if(score >= beta) {
            return score;
        }

        if(score > alpha) {
            alpha = score;
        }
    }

    return alpha;
}

int32_t GameTreeSearch::search(uint8_t depth, Array<Move, MAX_DEPTH>& pv) {
    int32_t score = 0;

    Array<Move, MAX_DEPTH> currentPV;

    for(uint8_t i = 1; i <= depth; i++) {
        currentDepth = i;

        currentPV.clear();
        score = pvSearch(i, MIN_SCORE, MAX_SCORE, currentPV);
        pv = currentPV;
    }

    return score;
}