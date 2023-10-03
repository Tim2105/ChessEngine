#ifndef REFEREE_H
#define REFEREE_H

#include "core/chess/Board.h"
#include "core/engine/Evaluator.h"

/**
 * @brief Evaluator für den Referee.
 * Die Methode evaluate() gibt immer 0 zurück, da die Bewertung der Positionen
 * für den Referee nicht relevant ist.
 * Stattdessen ist diese Klasse nur zur Überprüfung auf Unentschieden zuständig.
 */
class RefereeEvaluator : public Evaluator {
    public:
        RefereeEvaluator(Board& board) : Evaluator(board) {};
        ~RefereeEvaluator() = default;

        int32_t evaluate() override {
            return 0;
        }
};

inline bool isDraw(Board& board) {
    return (RefereeEvaluator(board).isDraw() ||
            (board.generateLegalMoves().size() == 0 &&
             !board.isCheck()));
}

inline bool isCheckmate(Board& board) {
    return board.isCheck() && board.generateLegalMoves().size() == 0;
}

inline bool isGameOver(Board& board) {
    return isDraw(board) || isCheckmate(board);
}

#endif