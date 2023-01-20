#include "GameTreeSearch.h"

GameTreeSearch::GameTreeSearch(Board& board) {
    this->board = &board;
    this->evaluator = BoardEvaluator();
}

GameTreeSearch::~GameTreeSearch() {
    
}

int32_t GameTreeSearch::pvSearch(int depth, int32_t alpha, int32_t beta, Array<Move, MAX_DEPTH>& pv) {
    if(depth == 0) {
        return evaluator.evaluate(*board);
    }

    Array<Move, 256> moves = board->generateLegalMoves();
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
            return score;
        }

        if(score > alpha) {
            alpha = score;

            pv.clear();
            pv.push_back(m);
            pv.push_back(childPV);
        }

        searchPV = false;
    }

    return alpha;
}

int32_t GameTreeSearch::nwSearch(int depth, int32_t alpha, int32_t beta) {
    if(depth == 0) {
        return evaluator.evaluate(*board);
    }

    Array<Move, 256> moves = board->generateLegalMoves();
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

Array<Move, MAX_DEPTH> GameTreeSearch::search(int32_t depth) {
    Array<Move, MAX_DEPTH> pv;
    int32_t score = pvSearch(depth, MIN_SCORE, MAX_SCORE, pv);
    return pv;
}