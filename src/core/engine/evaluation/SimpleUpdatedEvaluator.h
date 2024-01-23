#ifndef SIMPLE_UPDATED_EVALUATOR_H
#define SIMPLE_UPDATED_EVALUATOR_H

#include "core/engine/evaluation/EvaluationDefinitons.h"
#include "core/engine/evaluation/PSQT.h"
#include "core/engine/evaluation/Evaluator.h"
#include "core/engine/search/SearchDefinitions.h"

class SimpleUpdatedEvaluator: public Evaluator {
    private:
        Score currentScore;
        double currentGamePhase;
        int32_t currentPhaseWeight;
        bool isDraw = false;

        struct EvaluationVariables {
            Score score;
            double phase;
            int32_t phaseWeight;
        };

        Array<EvaluationVariables, MAX_PLY> evaluationHistory;

        void calculateScore();
        void calculateGamePhase();

    public:
        SimpleUpdatedEvaluator(Board& b) : Evaluator(b) {
            calculateScore();
            calculateGamePhase();
        };

        constexpr int32_t evaluate() override {
            return ((1.0 - currentGamePhase) * currentScore.mg + currentGamePhase * currentScore.eg) *
                   (b->getSideToMove() == WHITE ? 1 : -1);
        }

        constexpr double getGamePhase() const {
            return currentGamePhase;
        }

        constexpr int16_t getTaperedPSQTValue(int32_t piece, int32_t square) const {
            if(piece & BLACK) {
                // Für schwarze Figuren muss der Rang gespiegelt werden
                int32_t rank = SQ2R(square);
                int32_t file = SQ2F(square);
                square = FR2SQ(file, RANK_8 - rank);
            }

            piece = TYPEOF(piece);

            return (1.0 - currentGamePhase) * MG_PSQT[piece][square] + currentGamePhase * EG_PSQT[piece][square];
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