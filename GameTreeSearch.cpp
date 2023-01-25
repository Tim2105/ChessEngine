#include "GameTreeSearch.h"
#include <algorithm>
#include <cmath>

GameTreeSearch::GameTreeSearch(Board& board) {
    this->board = &board;
    this->evaluator = BoardEvaluator(board);

    clearKillerTable();
    clearHistoryTable();
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

void GameTreeSearch::clearKillerTable() {
    for(int i = 0; i < MAX_DEPTH; i++) {
        for(int j = 0; j < NUM_KILLER_MOVES; j++) {
            killerTable[i][j] = Move();
        }
    }
}

void GameTreeSearch::clearHistoryTable() {
    for(int i = 0; i < 15; i++) {
        for(int j = 0; j < 120; j++) {
            historyTable[i][j] = 0;
        }
    }
}

void GameTreeSearch::clearPVTable() {
    for(int i = 0; i < MAX_DEPTH; i++) {
        pvTable[i].clear();
    }
}

void GameTreeSearch::sortMoves(Array<Move, 256>& moves, int32_t plyFromRoot) {
    TranspositionTableEntry ttResult;
    bool ttHit = transpositionTable.probe(board->getHashValue(), ttResult);

    Move hashMove;

    if(ttHit)
        hashMove = ttResult.hashMove;

    Array<MoveScorePair, 256> moveScorePairs;

    for(Move m : moves) {
        int32_t moveScore = 0;

        if(m == hashMove)
            moveScore += HASH_MOVE_SCORE;

        if(std::find(killerTable[plyFromRoot], killerTable[plyFromRoot] + NUM_KILLER_MOVES, m) != killerTable[plyFromRoot] + NUM_KILLER_MOVES)
            moveScore += KILLER_MOVE_SCORE;
        
        int32_t pieceMoved = board->pieceAt(m.getOrigin());
        moveScore += historyTable[pieceMoved][m.getDestination()];

        moveScore += evaluator.evaluateMove(m);
            
        moveScorePairs.push_back({m, moveScore});
    }

    std::sort(moveScorePairs.begin(), moveScorePairs.end(), std::greater<MoveScorePair>());

    moves.clear();

    for(MoveScorePair msp : moveScorePairs) {
        moves.push_back(msp.move);
    }
}

void GameTreeSearch::sortMovesQuiescence(Array<Move, 256>& moves) {

    Array<MoveScorePair, 256> moveScorePairs;

    for(Move m : moves) {
        int32_t moveScore = evaluator.evaluateMove(m);

        if(moveScore >= 0)
            moveScorePairs.push_back({m, moveScore});
    }

    std::sort(moveScorePairs.begin(), moveScorePairs.end(), std::greater<MoveScorePair>());

    moves.clear();

    for(MoveScorePair msp : moveScorePairs) {
        moves.push_back(msp.move);
    }
}

int32_t GameTreeSearch::pvSearchInit(uint8_t depth, bool enableAspirationWindow, int32_t expectedScore) {
    if(!enableAspirationWindow) {
        return pvSearchRoot(depth, MIN_SCORE, MAX_SCORE);
    } else {
        int32_t alpha = expectedScore - ASPIRATION_WINDOW_SIZE;
        int32_t beta = expectedScore + ASPIRATION_WINDOW_SIZE;

        int32_t score = pvSearchRoot(depth, alpha, beta);

        int32_t failHigh = 0;
        int32_t failLow = 0;

        while(score <= alpha || score >= beta) {
            if(score <= alpha) {
                failHigh++;
                if(failHigh > MAX_ASPIRATION_WINDOW_STEPS)
                    alpha = MIN_SCORE;
                else
                    alpha = expectedScore - ASPIRATION_WINDOW_SIZE * pow(ASPIRATION_WINDOW_STEP_BASE, failHigh);
            } else {
                failLow++;
                if(failLow > MAX_ASPIRATION_WINDOW_STEPS)
                    beta = MAX_SCORE;
                else
                    beta = expectedScore + ASPIRATION_WINDOW_SIZE * pow(ASPIRATION_WINDOW_STEP_BASE, failLow);
            }

            score = pvSearchRoot(depth, alpha, beta);
        }

        return score;
    }
}

int32_t GameTreeSearch::pvSearchRoot(uint8_t depth, int32_t alpha, int32_t beta) {
    // Wenn die maximale Suchtiefe erreicht wurde, wird die Stellung bewertet
    // TODO: Stattdessen eine Quiezenzsuche durchführen
    if(depth == 0) {
        return quiescence(alpha, beta);
    }

    Array<Move, 256> moves = board->generateLegalMoves();

    // Wenn keine legalen Züge möglich sind, ist das Spiel vorbei
    if(moves.size() == 0) {
        if(board->isCheck())
            return -MATE_SCORE;
        else
            return 0;
    }

    sortMoves(moves, 0);

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
            score = -nwSearch(depth - 1, -alpha - 1, -alpha);

            if(score > alpha) {
                // Wenn die Nullfenstersuche den Zug nicht wiederlegen konnte, wird die
                // Variation vollständig durchsucht
                // Dieser Zug kommt eventuell in die Hauptvariation
                score = -pvSearch(depth - 1, -beta, -alpha);
            }
        }

        board->undoMove();

        if(score >= beta) {
            return beta;
        }

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
        switch(TYPEOF_NODE(ttResult.flags)) {
            case PV_NODE:
                return ttResult.score;
            case ALL_NODE:
                if(ttResult.score <= alpha) {
                    return ttResult.score;
                }
                break;
            case CUT_NODE:
                if(ttResult.score >= beta) {
                    return ttResult.score;
                }
                break;
        }
    }

    // Wenn die maximale Suchtiefe erreicht wurde, wird die Stellung bewertet
    // TODO: Stattdessen eine Quiezenzsuche durchführen
    if(depth == 0) {
        return quiescence(alpha, beta);
    }

    Array<Move, 256> moves = board->generateLegalMoves();

    // Wenn keine legalen Züge möglich sind, ist das Spiel vorbei
    if(moves.size() == 0) {
        if(board->isCheck())
            return -MATE_SCORE;
        else
            return 0;
    }

    sortMoves(moves, plyFromRoot);

    bool searchPV = false;

    int32_t score = MIN_SCORE;

    uint8_t ttFlags = ALL_NODE;

    Move bestMove;

    for(Move m : moves) {
        board->makeMove(m);

        if(searchPV) {
            // Die allererste Variation muss vollständig durchsucht werden
            score = -pvSearch(depth - 1, -beta, -alpha);
        } else {
            // Die restlichen Variationen werden mit einer Nullfenstersuche durchsucht
            score = -nwSearch(depth - 1, -alpha - 1, -alpha);

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
                ttEntry.score = score;
                ttEntry.depth = depth;
                ttEntry.flags = CUT_NODE;
                ttEntry.hashMove = m;

                transpositionTable.put(board->getHashValue(), ttEntry);
            }

            if(m.isQuiet()) {
                // Der Zug ist ein Killerzug
                memmove(&killerTable[plyFromRoot][1], &killerTable[plyFromRoot][0], sizeof(Move) * (NUM_KILLER_MOVES - 1));
                killerTable[plyFromRoot][0] = m;

                // Aktualisiere den Zug für die History-Heuristik
                int32_t movedPiece = board->pieceAt(m.getOrigin());
                historyTable[movedPiece][m.getDestination()] += depth * depth;
            }

            return score;
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

int32_t GameTreeSearch::nwSearch(uint8_t depth, int32_t alpha, int32_t beta) {
    int32_t plyFromRoot = currentDepth - depth;
    
    // Wenn diese Position mit gleicher oder höherer Tiefe schon einmal
    // durchsucht wurde, können die Ergebnisse aus der letzten Suche
    // wiederverwendet werden

    TranspositionTableEntry ttResult;

    bool ttHit = transpositionTable.probe(board->getHashValue(), ttResult);

    if(ttHit && ttResult.depth >= depth) {
        switch(TYPEOF_NODE(ttResult.flags)) {
            case PV_NODE:
                return ttResult.score;
            case ALL_NODE:
                if(ttResult.score <= alpha) {
                    return ttResult.score;
                }
                break;
            case CUT_NODE:
                if(ttResult.score >= beta) {
                    return ttResult.score;
                }
                break;
        }
    }

    // Wenn die maximale Suchtiefe erreicht wurde, wird die Stellung bewertet
    // TODO: Stattdessen eine Quiezenzsuche durchführen
    if(depth == 0) {
        return quiescence(alpha, beta);
    }

    int32_t score = MIN_SCORE;

    uint8_t ttFlags = NULL_WINDOW_NODE | ALL_NODE;

    Move hashMove = ttHit ? ttResult.hashMove : Move();

    Move bestMove;

    if(hashMove.exists()) {
        // Wenn der Hashzug gültig ist, wird er als erstes ausprobiert
        // Wenn der Hashzug einen Beta-Schnitt verursacht,
        // wird sowohl die Zuggeneration als auch die Vorsortierung übersprungen
        board->makeMove(hashMove);
        score = -nwSearch(depth - 1, -beta, -alpha);
        board->undoMove();

        if(score >= beta) {
            // Die Ergebnisse der Suche werden in die Transpositionstabelle eingetragen
            // Ergebnisse auf der Nullfenstersuche werden als Nullfensterknoten markiert,
            // weil die Bewertung nicht genau ist
            if((ttHit && ttResult.depth < depth && !IS_REGULAR_NODE(ttResult.flags)) || !ttHit) {
                TranspositionTableEntry ttEntry;
                ttEntry.score = score;
                ttEntry.depth = depth;
                ttEntry.flags = NULL_WINDOW_NODE | CUT_NODE;
                ttEntry.hashMove = hashMove;

                transpositionTable.put(board->getHashValue(), ttEntry);
            }
            
            if(hashMove.isQuiet()) {
                // Der Zug ist ein Killerzug
                memmove(&killerTable[plyFromRoot][1], &killerTable[plyFromRoot][0], sizeof(Move) * (NUM_KILLER_MOVES - 1));
                killerTable[plyFromRoot][0] = hashMove;
            }
            return score;
        }

        if(score > alpha) {
            alpha = score;
            bestMove = hashMove;
            ttFlags = NULL_WINDOW_NODE | PV_NODE;
        }
    }

    Array<Move, 256> moves = board->generateLegalMoves();

    // Wenn keine legalen Züge möglich sind, ist das Spiel vorbei
    if(moves.size() == 0) {
        if(board->isCheck())
            return -MATE_SCORE;
        else
            return 0;
    }

    sortMoves(moves, plyFromRoot);

    int32_t searchNumMoves = moves.size();

    if(!board->isCheck() && plyFromRoot >= FULL_MOVE_SEARCH_DEPTH_FUNCTION(currentDepth)) {
        searchNumMoves = LMR_MOVE_COUNT_FUNCTION(depth, searchNumMoves);
    }
    
    if(hashMove.exists())
        searchNumMoves--;

    for(Move m : moves) {
        if(searchNumMoves <= 0)
            break;

        // Wenn der Zug der Hashzug ist, wurde er bereits ausprobiert
        if(m == hashMove)
            continue;

        board->makeMove(m);
        // In der Nullfenstersuche werden auch alle weiteren Knoten
        // mit einem Nullfenster durchsucht
        score = -nwSearch(depth - 1, -beta, -alpha);
        board->undoMove();

        // Wenn der Zug den Gegner in eine schlechtere Stellung bringt,
        // als er bereits garantieren kann, wird die Suche abgebrochen
        if(score >= beta) {
            // Die Ergebnisse der Suche werden in die Transpositionstabelle eingetragen
            // Ergebnisse auf der Nullfenstersuche werden als Nullfensterknoten markiert,
            // weil die Bewertung nicht genau ist
            if((ttHit && ttResult.depth < depth && !IS_REGULAR_NODE(ttResult.flags)) || !ttHit) {
                TranspositionTableEntry ttEntry;
                ttEntry.score = score;
                ttEntry.depth = depth;
                ttEntry.flags = NULL_WINDOW_NODE | CUT_NODE;
                ttEntry.hashMove = m;

                transpositionTable.put(board->getHashValue(), ttEntry);
            }

            if(m.isQuiet()) {
                // Der Zug ist ein Killerzug
                memmove(&killerTable[plyFromRoot][1], &killerTable[plyFromRoot][0], sizeof(Move) * (NUM_KILLER_MOVES - 1));
                killerTable[plyFromRoot][0] = m;

                // In der Nullfenstersuche werden keine Züge für die History-Heuristik
                // gespeichert, weil Beta-Schnitte passieren und die History-Tabelle verfälscht wird
            }
            
            return score;
        }

        if(score > alpha) {
            alpha = score;
            ttFlags = NULL_WINDOW_NODE | PV_NODE;
            bestMove = m;
        }

        searchNumMoves--;
    }

    // Die Ergebnisse der Suche werden in die Transpositionstabelle eingetragen
    // Ergebnisse auf der Nullfenstersuche werden als Nullfensterknoten markiert,
    // weil die Bewertung nicht genau ist

    if(ttHit && ttResult.depth < depth && !IS_REGULAR_NODE(ttResult.flags)) {
        TranspositionTableEntry ttEntry;
        ttEntry.score = alpha;
        ttEntry.depth = depth;
        ttEntry.flags = ttFlags;
        ttEntry.hashMove = bestMove;

        transpositionTable.put(board->getHashValue(), ttEntry);
    }

    return alpha;
}

int32_t GameTreeSearch::quiescence(int32_t alpha, int32_t beta) {
    int32_t score = evaluator.evaluate();

    if(score >= beta) {
        return score;
    }
    
    if(score > alpha) {
        alpha = score;
    }
    
    Array<Move, 256> moves;
    if(board->isCheck()) { 
        moves = board->generateLegalMoves();

        // Wenn keine legalen Züge möglich sind, ist das Spiel vorbei
        if(moves.size() == 0)
            return -MATE_SCORE;
    }
    else
        moves = board->generateLegalCaptures();
    
    sortMovesQuiescence(moves);

    for(Move m : moves) {
        board->makeMove(m);
        score = -quiescence(-beta, -alpha);
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

int32_t GameTreeSearch::search(uint8_t depth, std::vector<Move>& pv) {
    int32_t score = 0;

    clearHistoryTable();

    bool firstSearch = true;

    for(uint8_t i = 1; i <= depth; i++) {
        clearPVTable();

        currentDepth = i;

        score = pvSearchInit(i, !firstSearch, score);

        std::cout << "Depth: " << (int) i << " Score: " << score << std::endl;

        for(Move m : pvTable[0]) {
            std::cout << m.toString() << " ";
        }

        std::cout << std::endl;

        if(abs(score) >= MATE_SCORE - MAX_DEPTH)
            break;

        firstSearch = false;
    }

    pv.clear();

    for(Move m : pvTable[0]) {
        pv.push_back(m);
    }

    return score;
}