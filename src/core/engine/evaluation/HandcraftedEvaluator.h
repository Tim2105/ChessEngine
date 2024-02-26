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
            Score pawnScore; // Bewertung der Bauernstruktur
            double phase; // Aktuelle Spielphase (0 = Mittelspiel, 1 = Endspiel)
            int32_t phaseWeight; // Materialgewichtung für die Spielphase
        };

        EvaluationVariables evaluationVars;

        std::vector<EvaluationVariables> evaluationHistory;

        void calculateMaterialScore();
        void calculatePawnScore();
        void calculateGamePhase();
        int32_t calculateKingSafetyScore();

        int32_t evaluateKNBKEndgame(int32_t ownBishopSq, int32_t oppKingSq);
        int32_t evaluateWinningNoPawnsEndgame(int32_t oppKingSq);

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
            140, // Pawn
            330, // Knight
            360, // Bishop
            530, // Rook
            1030, // Queen
            0 // King
        };

        static constexpr int16_t EG_WINNING_MATERIAL_ADVANTAGE = (EG_PIECE_VALUE[ROOK] - EG_PIECE_VALUE[BISHOP]) / 2 + EG_PIECE_VALUE[BISHOP];
        static constexpr int16_t EG_WINNING_BONUS = 800;
        static constexpr int16_t EG_SPECIAL_MATE_PROGRESS_BONUS = 150;

        static constexpr int32_t MG_TEMPO_BONUS = 12;
        static constexpr int32_t EG_TEMPO_BONUS = 4;

    public:
        HandcraftedEvaluator(Board& b) : Evaluator(b) {
            evaluationHistory.reserve(MAX_PLY);

            calculateMaterialScore();
            calculatePawnScore();
            calculateGamePhase();
        };

        inline int32_t evaluate() override {
            int32_t numPawns = board.getPieceBitboard(WHITE_PAWN).popcount() +
                               board.getPieceBitboard(BLACK_PAWN).popcount();

        	// Spezialfall: Keine Bauern mehr auf dem Spielfeld
            if(numPawns == 0) {
                // Wenn der Materialvorteil kleiner als das Gewicht eines Turms ist, dann ist es Unentschieden
                if(std::abs(evaluationVars.materialScore.eg) < EG_WINNING_MATERIAL_ADVANTAGE)
                    return DRAW_SCORE;

                int32_t whiteKingSq = board.getKingSquare(WHITE);
                int32_t blackKingSq = board.getKingSquare(BLACK);

                if(evaluationVars.materialScore.eg >= EG_WINNING_MATERIAL_ADVANTAGE) {
                    bool isKBNK = (board.getPieceBitboard(WHITE_KNIGHT).popcount() == 1 && board.getPieceBitboard(WHITE_BISHOP).popcount() == 1 &&
                                    board.getPieceBitboard(WHITE_ROOK).popcount() == 0 && board.getPieceBitboard(WHITE_QUEEN).popcount() == 0);

                    bool isKNNK = (board.getPieceBitboard(WHITE_KNIGHT).popcount() == 2 && board.getPieceBitboard(WHITE_BISHOP).popcount() == 0 &&
                                    board.getPieceBitboard(WHITE_ROOK).popcount() == 0 && board.getPieceBitboard(WHITE_QUEEN).popcount() == 0);

                    if(isKBNK) // Läufer und Springer gegen König -> Matt
                        return evaluateKNBKEndgame(board.getPieceBitboard(WHITE_BISHOP).getFSB(), blackKingSq) * (board.getSideToMove() == WHITE ? 1 : -1);
                    else if(isKNNK) // Zwei Springer gegen König -> Unentschieden
                        return DRAW_SCORE;
                    else // Jede andere Kombination -> Matt
                        return evaluateWinningNoPawnsEndgame(blackKingSq) * (board.getSideToMove() == WHITE ? 1 : -1);
                } else if(evaluationVars.materialScore.eg <= -EG_WINNING_MATERIAL_ADVANTAGE) {
                    bool isKBNK = (board.getPieceBitboard(BLACK_KNIGHT).popcount() == 1 && board.getPieceBitboard(BLACK_BISHOP).popcount() == 1 &&
                                    board.getPieceBitboard(BLACK_ROOK).popcount() == 0 && board.getPieceBitboard(BLACK_QUEEN).popcount() == 0);

                    bool isKNNK = (board.getPieceBitboard(BLACK_KNIGHT).popcount() == 2 && board.getPieceBitboard(BLACK_BISHOP).popcount() == 0 &&
                                    board.getPieceBitboard(BLACK_ROOK).popcount() == 0 && board.getPieceBitboard(BLACK_QUEEN).popcount() == 0);

                    if(isKBNK) // Läufer und Springer gegen König -> Matt
                        return evaluateKNBKEndgame(board.getPieceBitboard(BLACK_BISHOP).getFSB(), whiteKingSq) * (board.getSideToMove() == WHITE ? -1 : 1);
                    else if(isKNNK) // Zwei Springer gegen König -> Unentschieden
                        return DRAW_SCORE;
                    else // Jede andere Kombination -> Matt
                        return evaluateWinningNoPawnsEndgame(whiteKingSq) * (board.getSideToMove() == WHITE ? -1 : 1);
                }
            }

            Score score = evaluationVars.materialScore + evaluationVars.pawnScore;

            // Aktualisiere die Königssicherheitsbewertung
            int32_t kingSafetyScore = calculateKingSafetyScore();

            int32_t evaluation = ((1.0 - evaluationVars.phase) * (score.mg + kingSafetyScore) + evaluationVars.phase * score.eg) *
                                 (board.getSideToMove() == WHITE ? 1 : -1);

            evaluation += (1.0 - evaluationVars.phase) * MG_TEMPO_BONUS + evaluationVars.phase * EG_TEMPO_BONUS;

            // Skaliere die Bewertung in Richtung 0, wenn wir uns der 50-Züge-Regel annähern.
            // (Starte erst nach 10 Zügen, damit die Bewertung nicht zu früh verzerrt wird.)
            int32_t fiftyMoveCounter = board.getFiftyMoveCounter();
            if(fiftyMoveCounter > 20)
                evaluation = (int32_t)evaluation * (100 - fiftyMoveCounter) / 80;

            return evaluation;
        }

        constexpr double getGamePhase() const {
            return evaluationVars.phase;
        }

        void updateBeforeMove(Move m) override;
        void updateAfterMove() override;
        void updateBeforeUndo() override;

        inline void setBoard(Board& b) override {
            Evaluator::setBoard(b);

            evaluationHistory.clear();

            calculateMaterialScore();
            calculatePawnScore();
            calculateGamePhase();
        }

    private:
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

        // Bestrafung für Doppelbauern pro Linie im Mittelspiel
        static constexpr int16_t MG_DOUBLED_PAWN_PENALTY[8] = {
            -28, -35, -17, -31, -34, -26, -31, -30
        };

        // Bestrafung für Doppelbauern pro Linie im Endspiel
        static constexpr int16_t EG_DOUBLED_PAWN_PENALTY[8] = {
            -19, -22, -23, -30, -32, -25, -21, -19
        };

        // Bestrafung für isolierte Bauern pro Linie im Mittelspiel
        static constexpr int16_t MG_ISOLATED_PAWN_PENALTY[8] = {
            -34, -19, -16, -34, -35, -15, -8, -40
        };

        // Bestrafung für isolierte Bauern pro Linie im Endspiel
        static constexpr int16_t EG_ISOLATED_PAWN_PENALTY[8] = {
            -2, -9, -15, -19, -18, -13, -8, -3
        };

        // Bonus für Freibauern pro Rang im Mittelspiel
        static constexpr int16_t MG_PASSED_PAWN_BONUS[8] = {
            0, 7, 7, 13, 20, 31, 49, 0
        };

        // Bonus für Freibauern pro Rang im Endspiel
        static constexpr int16_t EG_PASSED_PAWN_BONUS[8] = {
            0, 35, 35, 46, 59, 72, 101, 0
        };

        static constexpr Bitboard fileFacingEnemy[2][64] = {
            // White
            {
                0x101010101010100,0x202020202020200,0x404040404040400,0x808080808080800,0x1010101010101000,0x2020202020202000,0x4040404040404000,0x8080808080808000,
                0x101010101010000,0x202020202020000,0x404040404040000,0x808080808080000,0x1010101010100000,0x2020202020200000,0x4040404040400000,0x8080808080800000,
                0x101010101000000,0x202020202000000,0x404040404000000,0x808080808000000,0x1010101010000000,0x2020202020000000,0x4040404040000000,0x8080808080000000,
                0x101010100000000,0x202020200000000,0x404040400000000,0x808080800000000,0x1010101000000000,0x2020202000000000,0x4040404000000000,0x8080808000000000,
                0x101010000000000,0x202020000000000,0x404040000000000,0x808080000000000,0x1010100000000000,0x2020200000000000,0x4040400000000000,0x8080800000000000,
                0x101000000000000,0x202000000000000,0x404000000000000,0x808000000000000,0x1010000000000000,0x2020000000000000,0x4040000000000000,0x8080000000000000,
                0x100000000000000,0x200000000000000,0x400000000000000,0x800000000000000,0x1000000000000000,0x2000000000000000,0x4000000000000000,0x8000000000000000,
                0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
            },
            // Black
            {
                0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80,
                0x101,0x202,0x404,0x808,0x1010,0x2020,0x4040,0x8080,
                0x10101,0x20202,0x40404,0x80808,0x101010,0x202020,0x404040,0x808080,
                0x1010101,0x2020202,0x4040404,0x8080808,0x10101010,0x20202020,0x40404040,0x80808080,
                0x101010101,0x202020202,0x404040404,0x808080808,0x1010101010,0x2020202020,0x4040404040,0x8080808080,
                0x10101010101,0x20202020202,0x40404040404,0x80808080808,0x101010101010,0x202020202020,0x404040404040,0x808080808080,
                0x1010101010101,0x2020202020202,0x4040404040404,0x8080808080808,0x10101010101010,0x20202020202020,0x40404040404040,0x80808080808080,
            }
        };

        static constexpr Bitboard neighboringFiles[8] = {
            0x101010101010101, // A
            0x505050505050505, // B
            0xA0A0A0A0A0A0A0A, // C
            0x1414141414141414, // D
            0x2828282828282828, // E
            0x5050505050505050, // F
            0xA0A0A0A0A0A0A0A0, // G
            0x4040404040404040 // H
        };

        static constexpr Bitboard sentryMasks[2][64] = {
            // White
            {
                    0x303030303030300,0x707070707070700,0xE0E0E0E0E0E0E00,0x1C1C1C1C1C1C1C00,0x3838383838383800,0x7070707070707000,0xE0E0E0E0E0E0E000,0xC0C0C0C0C0C0C000,
                    0x303030303030000,0x707070707070000,0xE0E0E0E0E0E0000,0x1C1C1C1C1C1C0000,0x3838383838380000,0x7070707070700000,0xE0E0E0E0E0E00000,0xC0C0C0C0C0C00000,
                    0x303030303000000,0x707070707000000,0xE0E0E0E0E000000,0x1C1C1C1C1C000000,0x3838383838000000,0x7070707070000000,0xE0E0E0E0E0000000,0xC0C0C0C0C0000000,
                    0x303030300000000,0x707070700000000,0xE0E0E0E00000000,0x1C1C1C1C00000000,0x3838383800000000,0x7070707000000000,0xE0E0E0E000000000,0xC0C0C0C000000000,
                    0x303030000000000,0x707070000000000,0xE0E0E0000000000,0x1C1C1C0000000000,0x3838380000000000,0x7070700000000000,0xE0E0E00000000000,0xC0C0C00000000000,
                    0x303000000000000,0x707000000000000,0xE0E000000000000,0x1C1C000000000000,0x3838000000000000,0x7070000000000000,0xE0E0000000000000,0xC0C0000000000000,
                    0x300000000000000,0x700000000000000,0xE00000000000000,0x1C00000000000000,0x3800000000000000,0x7000000000000000,0xE000000000000000,0xC000000000000000,
                    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
            },
            // Black
            {
                    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                    0x3,0x7,0xE,0x1C,0x38,0x70,0xE0,0xC0,
                    0x303,0x707,0xE0E,0x1C1C,0x3838,0x7070,0xE0E0,0xC0C0,
                    0x30303,0x70707,0xE0E0E,0x1C1C1C,0x383838,0x707070,0xE0E0E0,0xC0C0C0,
                    0x3030303,0x7070707,0xE0E0E0E,0x1C1C1C1C,0x38383838,0x70707070,0xE0E0E0E0,0xC0C0C0C0,
                    0x303030303,0x707070707,0xE0E0E0E0E,0x1C1C1C1C1C,0x3838383838,0x7070707070,0xE0E0E0E0E0,0xC0C0C0C0C0,
                    0x30303030303,0x70707070707,0xE0E0E0E0E0E,0x1C1C1C1C1C1C,0x383838383838,0x707070707070,0xE0E0E0E0E0E0,0xC0C0C0C0C0C0,
                    0x3030303030303,0x7070707070707,0xE0E0E0E0E0E0E,0x1C1C1C1C1C1C1C,0x38383838383838,0x70707070707070,0xE0E0E0E0E0E0E0,0xC0C0C0C0C0C0C0,
            }
        };

        static constexpr int32_t NUM_ATTACKER_WEIGHT[5] = {
            0, 50, 75, 90, 100
        };

        static constexpr size_t NUM_ATTACKER_WEIGHT_SIZE = sizeof(NUM_ATTACKER_WEIGHT) / sizeof(NUM_ATTACKER_WEIGHT[0]);

        static constexpr int32_t PIECE_UNDEFENDED_ATTACK_WEIGHT[7] = {
            0, // Empty
            0, // Pawn
            24, // Knight
            24, // Bishop
            35, // Rook
            62, // Queen
            0 // King
        };

        static constexpr int32_t PIECE_DEFENDED_ATTACK_WEIGHT[7] = {
            0, // Empty
            0, // Pawn
            14, // Knight
            14, // Bishop
            18, // Rook
            24, // Queen
            0 // King
        };

        static constexpr Bitboard kingAttackZone[2][64] = {
            // White
            {
                0x3030302,0x7070705,0xE0E0E0A,0x1C1C1C14,0x38383828,0x70707050,0xE0E0E0A0,0xC0C0C040,
                0x303030200,0x707070500,0xE0E0E0A00,0x1C1C1C1400,0x3838382800,0x7070705000,0xE0E0E0A000,0xC0C0C04000,
                0x30303020000,0x70707050000,0xE0E0E0A0000,0x1C1C1C140000,0x383838280000,0x707070500000,0xE0E0E0A00000,0xC0C0C0400000,
                0x3030302000000,0x7070705000000,0xE0E0E0A000000,0x1C1C1C14000000,0x38383828000000,0x70707050000000,0xE0E0E0A0000000,0xC0C0C040000000,
                0x303030200000000,0x707070500000000,0xE0E0E0A00000000,0x1C1C1C1400000000,0x3838382800000000,0x7070705000000000,0xE0E0E0A000000000,0xC0C0C04000000000,
                0x303020000000003,0x707050000000007,0xE0E0A000000000E,0x1C1C14000000001C,0x3838280000000038,0x7070500000000070,0xE0E0A000000000E0,0xC0C04000000000C0,
                0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
            },
            // Black
            {
                0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                0x300000000020303,0x700000000050707,0xE000000000A0E0E,0x1C00000000141C1C,0x3800000000283838,0x7000000000507070,0xE000000000A0E0E0,0xC00000000040C0C0,
                0x2030303,0x5070707,0xA0E0E0E,0x141C1C1C,0x28383838,0x50707070,0xA0E0E0E0,0x40C0C0C0,
                0x203030300,0x507070700,0xA0E0E0E00,0x141C1C1C00,0x2838383800,0x5070707000,0xA0E0E0E000,0x40C0C0C000,
                0x20303030000,0x50707070000,0xA0E0E0E0000,0x141C1C1C0000,0x283838380000,0x507070700000,0xA0E0E0E00000,0x40C0C0C00000,
                0x2030303000000,0x5070707000000,0xA0E0E0E000000,0x141C1C1C000000,0x28383838000000,0x50707070000000,0xA0E0E0E0000000,0x40C0C0C0000000,
                0x203030300000000,0x507070700000000,0xA0E0E0E00000000,0x141C1C1C00000000,0x2838383800000000,0x5070707000000000,0xA0E0E0E000000000,0x40C0C0C000000000,
            }
        };
};

#endif