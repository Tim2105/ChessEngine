#ifndef TOURNAMENT_H
#define TOURNAMENT_H

#include "core/chess/Board.h"
#include "core/engine/SearchTree.h"

#include <vector>
#include <string>

/**
 * @brief Evaluator für das Turnier.
 * Die Methode evaluate() gibt immer 0 zurück, da die Bewertung der Positionen
 * im Turnier nicht relevant ist.
 * Stattdessen ist diese Klasse nur zur Überprüfung auf Unentschieden zuständig.
 */
class TournamentEvaluator : public Evaluator {
    public:
        TournamentEvaluator(Board& board) : Evaluator(board) {};
        ~TournamentEvaluator() = default;

        int32_t evaluate() override {
            return 0;
        }
};

class Tournament {

    private:
        SearchTree& st1;
        SearchTree& st2;
        std::string engineName1;
        std::string engineName2;

        static const std::vector<std::string> openings;
        static const std::vector<int32_t> timeControls;
        static const std::vector<int32_t> numGames;

        int32_t runGame(Board& board, SearchTree& st1, SearchTree& st2, int32_t time);

    public:
        Tournament(SearchTree& st1, SearchTree& st2, std::string engineName1, std::string engineName2) : st1(st1), st2(st2), engineName1(engineName1), engineName2(engineName2) {};
        ~Tournament() = default;

        void run();

};

#endif