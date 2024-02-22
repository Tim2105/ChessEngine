#ifndef HANDCRAFTED_EVALUATOR_H
#define HANDCRAFTED_EVALUATOR_H

#include "core/engine/evaluation/EvaluationDefinitons.h"
#include "core/engine/evaluation/PSQT.h"
#include "core/engine/evaluation/Evaluator.h"
#include "core/engine/search/SearchDefinitions.h"

#include <vector>

class HandcraftedEvaluator: public Evaluator {
    private:
        struct EvaluationVariables {
            Score materialScore; // Bewertung des Materials und der Figurentabellen
            double phase; // Aktuelle Spielphase (0 = Mittelspiel, 1 = Endspiel)
            int32_t phaseWeight; // Materialgewichtung für die Spielphase
        };

        EvaluationVariables evaluationVars;
        bool isDraw = false;

        std::vector<EvaluationVariables> evaluationHistory;

        void calculateScore();
        void calculateGamePhase();

    public:
        HandcraftedEvaluator(Board& b) : Evaluator(b) {
            evaluationHistory.reserve(MAX_PLY);

            calculateScore();
            calculateGamePhase();
        };

        constexpr int32_t evaluate() override {
            return ((1.0 - evaluationVars.phase) * evaluationVars.materialScore.mg + evaluationVars.phase * evaluationVars.materialScore.eg) *
                   (board.getSideToMove() == WHITE ? 1 : -1);
        }

        constexpr double getGamePhase() const {
            return evaluationVars.phase;
        }

        constexpr int16_t getTaperedPSQTValue(int32_t piece, int32_t square) const {
            if(piece & BLACK) {
                // Für schwarze Figuren muss der Rang gespiegelt werden
                square = Square::flipY(square);
            }

            piece = TYPEOF(piece);

            return (1.0 - evaluationVars.phase) * MG_PSQT[piece][square] + evaluationVars.phase * EG_PSQT[piece][square];
        }

        void updateBeforeMove(Move m) override;
        void updateBeforeUndo() override;

        inline void setBoard(Board& b) override {
            Evaluator::setBoard(b);

            evaluationHistory.clear();

            calculateScore();
            calculateGamePhase();
        }

    private:
        static constexpr int16_t MG_PIECE_VALUE[7] = {
            0, // Empty
            110, // Pawn
            350, // Knight
            360, // Bishop
            520, // Rook
            980, // Queen
            0 // King
        };

        static constexpr int16_t EG_PIECE_VALUE[7] = {
            0, // Empty
            150, // Pawn
            350, // Knight
            360, // Bishop
            530, // Rook
            950, // Queen
            0 // King
        };

        /**
         * @brief Konstanten zur Berechnung der Spielphase.
         * Eine Phase von 0 bedeutet, Midgame und eine Phase von 1 bedeutet Endgame.
         * Diese Phase wird benutzt um zwischen Midgame- und Endgameevaluation zu interpolieren.
         */

        // Figurengewichte
        static constexpr int32_t PAWN_WEIGHT = 0;
        static constexpr int32_t KNIGHT_WEIGHT = 1;
        static constexpr int32_t BISHOP_WEIGHT = 1;
        static constexpr int32_t ROOK_WEIGHT = 2;
        static constexpr int32_t QUEEN_WEIGHT = 4;

        static constexpr int32_t PIECE_WEIGHT[7] = {
            0, // Empty
            PAWN_WEIGHT,
            KNIGHT_WEIGHT,
            BISHOP_WEIGHT,
            ROOK_WEIGHT,
            QUEEN_WEIGHT,
            0 // King
        };

        static constexpr int32_t TOTAL_WEIGHT = PAWN_WEIGHT * 16 + KNIGHT_WEIGHT * 4 + BISHOP_WEIGHT * 4 + ROOK_WEIGHT * 4 + QUEEN_WEIGHT * 2;

        // Phasengrenzen, können unter 0 oder über 1 sein,
        // die berechnete Phase wird aber zwischen 0 und 1 eingeschränkt
        static constexpr double MIN_PHASE = -0.5;
        static constexpr double MAX_PHASE = 1.25;
};

#endif