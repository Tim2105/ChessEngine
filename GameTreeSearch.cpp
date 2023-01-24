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
    // Wenn die maximale Suchtiefe erreicht wurde, wird die Stellung bewertet
    // TODO: Stattdessen eine Quiezenzsuche durchführen
    if(depth == 0) {
        return evaluator.evaluate();
    }

    int32_t alpha = MIN_SCORE;
    int32_t beta = MAX_SCORE;

    Array<Move, 256> moves = board->generateLegalMoves();

    // Wenn keine legalen Züge möglich sind, ist das Spiel vorbei
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
            // Die allererste Variation muss vollständig durchsucht werden
            score = -pvSearch(depth - 1, -beta, -alpha);

        } else {
            // Die restlichen Variationen werden mit einer Nullfenstersuche durchsucht
            score = -nwSearch(depth - 1, -alpha);

            if(score > alpha) {
                // Wenn die Nullfenstersuche den Zug nicht wiederlegen konnte, wird die
                // Variation vollständig durchsucht
                // Dieser Zug kommt eventuell in die Hauptvariation
                score = -pvSearch(depth - 1, -beta, -alpha);
            }
        }

        board->undoMove();

        if(score > alpha) {
            // Der Zug ist der beste Zug, der bisher gefunden wurde
            // Er gehört ab jetzt zur Hauptvariante bis ein besserer Zug gefunden wird
            // oder die Suche stoppt
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

    // Der beste Zug wird in die Transpositionstabelle eingetragen,
    // damit zukünftige Suchen mit gleicher oder niedrigerer Tiefe
    // nicht dieselben Berechnungen durchführen müssen
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

    // Wenn diese Position mit gleicher oder höherer Tiefe schon einmal
    // durchsucht wurde, können die Ergebnisse aus der letzten Suche
    // wiederverwendet werden

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

    // Wenn die maximale Suchtiefe erreicht wurde, wird die Stellung bewertet
    // TODO: Stattdessen eine Quiezenzsuche durchführen
    if(depth == 0) {
        return evaluator.evaluate();
    }

    Array<Move, 256> moves = board->generateLegalMoves();

    // Wenn keine legalen Züge möglich sind, ist das Spiel vorbei
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
            // Die allererste Variation muss vollständig durchsucht werden
            score = -pvSearch(depth - 1, -beta, -alpha);
        } else {
            // Die restlichen Variationen werden mit einer Nullfenstersuche durchsucht
            score = -nwSearch(depth - 1, -alpha);

            if(score > alpha) {
                // Wenn die Nullfenstersuche den Zug nicht wiederlegen konnte, wird die
                // Variation vollständig durchsucht
                // Dieser Zug kommt eventuell in die Hauptvariation
                score = -pvSearch(depth - 1, -beta, -alpha);
            }
        }

        board->undoMove();

        // Wenn der Zug den Gegenr in eine schlechtere Stellung bringt,
        // als er bereits garantieren kann, wird die Suche abgebrochen
        if(score >= beta) {
            // Die Ergebnisse der Suche werden in die Transpositionstabelle eingetragen
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

        // Wenn der Zug besser ist als der bisher beste Zug, wird er zur Hauptvariante
        // und die Suche wird fortgesetzt
        if(score > alpha) {
            bestMove = m;
            alpha = score;
            ttFlags = PV_NODE;

            // Trage den Zug in die Hauptvariante ein
            pvTable[plyFromRoot].clear();
            pvTable[plyFromRoot].push_back(m);
            pvTable[plyFromRoot].push_back(pvTable[plyFromRoot + 1]);
        }

        searchPV = false;
    }

    // Die Ergebnisse der Suche werden in die Transpositionstabelle eingetragen
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
    
    // Wenn diese Position mit gleicher oder höherer Tiefe schon einmal
    // durchsucht wurde, können die Ergebnisse aus der letzten Suche
    // wiederverwendet werden

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
                if(ttResult.score <= beta - 1) {
                    return beta - 1;
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

    // Wenn die maximale Suchtiefe erreicht wurde, wird die Stellung bewertet
    // TODO: Stattdessen eine Quiezenzsuche durchführen
    if(depth == 0) {
        return evaluator.evaluate();
    }
    Array<Move, 256> moves = board->generateLegalMoves();

    // Wenn keine legalen Züge möglich sind, ist das Spiel vorbei
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
        // In der Nullfenstersuche werden auch alle weiteren Knoten
        // mit einem Nullfenster durchsucht
        score = -nwSearch(depth - 1, -beta + 1);
        board->undoMove();

        // Wenn der Zug den Gegner in eine schlechtere Stellung bringt,
        // als er bereits garantieren kann, wird die Suche abgebrochen
        if(score >= beta) {
            // Die Ergebnisse der Suche werden in die Transpositionstabelle eingetragen
            // Ergebnisse auf der Nullfenstersuche werden als Nullfensterknoten markiert,
            // weil die Bewertung nicht genau ist
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