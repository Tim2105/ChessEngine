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
            100, // Pawn
            310, // Knight
            330, // Bishop
            490, // Rook
            950, // Queen
            0 // King
        };

        static constexpr int16_t EG_PIECE_VALUE[7] = {
            0, // Empty
            140, // Pawn
            300, // Knight
            335, // Bishop
            525, // Rook
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
                0x101010101010100ULL,0x202020202020200ULL,0x404040404040400ULL,0x808080808080800ULL,0x1010101010101000ULL,0x2020202020202000ULL,0x4040404040404000ULL,0x8080808080808000ULL,
                0x101010101010000ULL,0x202020202020000ULL,0x404040404040000ULL,0x808080808080000ULL,0x1010101010100000ULL,0x2020202020200000ULL,0x4040404040400000ULL,0x8080808080800000ULL,
                0x101010101000000ULL,0x202020202000000ULL,0x404040404000000ULL,0x808080808000000ULL,0x1010101010000000ULL,0x2020202020000000ULL,0x4040404040000000ULL,0x8080808080000000ULL,
                0x101010100000000ULL,0x202020200000000ULL,0x404040400000000ULL,0x808080800000000ULL,0x1010101000000000ULL,0x2020202000000000ULL,0x4040404000000000ULL,0x8080808000000000ULL,
                0x101010000000000ULL,0x202020000000000ULL,0x404040000000000ULL,0x808080000000000ULL,0x1010100000000000ULL,0x2020200000000000ULL,0x4040400000000000ULL,0x8080800000000000ULL,
                0x101000000000000ULL,0x202000000000000ULL,0x404000000000000ULL,0x808000000000000ULL,0x1010000000000000ULL,0x2020000000000000ULL,0x4040000000000000ULL,0x8080000000000000ULL,
                0x100000000000000ULL,0x200000000000000ULL,0x400000000000000ULL,0x800000000000000ULL,0x1000000000000000ULL,0x2000000000000000ULL,0x4000000000000000ULL,0x8000000000000000ULL,
                0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,
            },
            // Black
            {
                0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,
                0x1ULL,0x2ULL,0x4ULL,0x8ULL,0x10ULL,0x20ULL,0x40ULL,0x80ULL,
                0x101ULL,0x202ULL,0x404ULL,0x808ULL,0x1010ULL,0x2020ULL,0x4040ULL,0x8080ULL,
                0x10101ULL,0x20202ULL,0x40404ULL,0x80808ULL,0x101010ULL,0x202020ULL,0x404040ULL,0x808080ULL,
                0x1010101ULL,0x2020202ULL,0x4040404ULL,0x8080808ULL,0x10101010ULL,0x20202020ULL,0x40404040ULL,0x80808080ULL,
                0x101010101ULL,0x202020202ULL,0x404040404ULL,0x808080808ULL,0x1010101010ULL,0x2020202020ULL,0x4040404040ULL,0x8080808080ULL,
                0x10101010101ULL,0x20202020202ULL,0x40404040404ULL,0x80808080808ULL,0x101010101010ULL,0x202020202020ULL,0x404040404040ULL,0x808080808080ULL,
                0x1010101010101ULL,0x2020202020202ULL,0x4040404040404ULL,0x8080808080808ULL,0x10101010101010ULL,0x20202020202020ULL,0x40404040404040ULL,0x80808080808080ULL,
            }
        };

        static constexpr Bitboard neighboringFiles[8] = {
            0x101010101010101ULL, // A
            0x505050505050505ULL, // B
            0xA0A0A0A0A0A0A0AULL, // C
            0x1414141414141414ULL, // D
            0x2828282828282828ULL, // E
            0x5050505050505050ULL, // F
            0xA0A0A0A0A0A0A0A0ULL, // G
            0x4040404040404040ULL // H
        };

        static constexpr Bitboard sentryMasks[2][64] = {
            // White
            {
                    0x303030303030300ULL,0x707070707070700ULL,0xE0E0E0E0E0E0E00ULL,0x1C1C1C1C1C1C1C00ULL,0x3838383838383800ULL,0x7070707070707000ULL,0xE0E0E0E0E0E0E000ULL,0xC0C0C0C0C0C0C000ULL,
                    0x303030303030000ULL,0x707070707070000ULL,0xE0E0E0E0E0E0000ULL,0x1C1C1C1C1C1C0000ULL,0x3838383838380000ULL,0x7070707070700000ULL,0xE0E0E0E0E0E00000ULL,0xC0C0C0C0C0C00000ULL,
                    0x303030303000000ULL,0x707070707000000ULL,0xE0E0E0E0E000000ULL,0x1C1C1C1C1C000000ULL,0x3838383838000000ULL,0x7070707070000000ULL,0xE0E0E0E0E0000000ULL,0xC0C0C0C0C0000000ULL,
                    0x303030300000000ULL,0x707070700000000ULL,0xE0E0E0E00000000ULL,0x1C1C1C1C00000000ULL,0x3838383800000000ULL,0x7070707000000000ULL,0xE0E0E0E000000000ULL,0xC0C0C0C000000000ULL,
                    0x303030000000000ULL,0x707070000000000ULL,0xE0E0E0000000000ULL,0x1C1C1C0000000000ULL,0x3838380000000000ULL,0x7070700000000000ULL,0xE0E0E00000000000ULL,0xC0C0C00000000000ULL,
                    0x303000000000000ULL,0x707000000000000ULL,0xE0E000000000000ULL,0x1C1C000000000000ULL,0x3838000000000000ULL,0x7070000000000000ULL,0xE0E0000000000000ULL,0xC0C0000000000000ULL,
                    0x300000000000000ULL,0x700000000000000ULL,0xE00000000000000ULL,0x1C00000000000000ULL,0x3800000000000000ULL,0x7000000000000000ULL,0xE000000000000000ULL,0xC000000000000000ULL,
                    0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,
            },
            // Black
            {
                    0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,
                    0x3ULL,0x7ULL,0xEULL,0x1CULL,0x38ULL,0x70ULL,0xE0ULL,0xC0ULL,
                    0x303ULL,0x707ULL,0xE0EULL,0x1C1CULL,0x3838ULL,0x7070ULL,0xE0E0ULL,0xC0C0ULL,
                    0x30303ULL,0x70707ULL,0xE0E0EULL,0x1C1C1CULL,0x383838ULL,0x707070ULL,0xE0E0E0ULL,0xC0C0C0ULL,
                    0x3030303ULL,0x7070707ULL,0xE0E0E0EULL,0x1C1C1C1CULL,0x38383838ULL,0x70707070ULL,0xE0E0E0E0ULL,0xC0C0C0C0ULL,
                    0x303030303ULL,0x707070707ULL,0xE0E0E0E0EULL,0x1C1C1C1C1CULL,0x3838383838ULL,0x7070707070ULL,0xE0E0E0E0E0ULL,0xC0C0C0C0C0ULL,
                    0x30303030303ULL,0x70707070707ULL,0xE0E0E0E0E0EULL,0x1C1C1C1C1C1CULL,0x383838383838ULL,0x707070707070ULL,0xE0E0E0E0E0E0ULL,0xC0C0C0C0C0C0ULL,
                    0x3030303030303ULL,0x7070707070707ULL,0xE0E0E0E0E0E0EULL,0x1C1C1C1C1C1C1CULL,0x38383838383838ULL,0x70707070707070ULL,0xE0E0E0E0E0E0E0ULL,0xC0C0C0C0C0C0C0ULL,
            }
        };

        static constexpr int32_t NUM_ATTACKER_WEIGHT[6] = {
            0, 0, 50, 75, 90, 100
        };

        static constexpr size_t NUM_ATTACKER_WEIGHT_SIZE = sizeof(NUM_ATTACKER_WEIGHT) / sizeof(NUM_ATTACKER_WEIGHT[0]);

        static constexpr int32_t KNIGHT_UNDEFENDED_ATTACK_WEIGHT = 28;
        static constexpr int32_t BISHOP_UNDEFENDED_ATTACK_WEIGHT = 28;
        static constexpr int32_t ROOK_UNDEFENDED_ATTACK_WEIGHT = 52;
        static constexpr int32_t QUEEN_UNDEFENDED_ATTACK_WEIGHT = 96;

        static constexpr int32_t KNIGHT_DEFENDED_ATTACK_WEIGHT = 17;
        static constexpr int32_t BISHOP_DEFENDED_ATTACK_WEIGHT = 17;
        static constexpr int32_t ROOK_DEFENDED_ATTACK_WEIGHT = 29;
        static constexpr int32_t QUEEN_DEFENDED_ATTACK_WEIGHT = 40;

        static constexpr Bitboard kingAttackZone[64] = {
            0x30707ULL,0x70f0fULL,0xe0e0eULL,0x1c1c1cULL,0x383838ULL,0x707070ULL,0xe0f0f0ULL,0xc0e0e0ULL,
            0x3070707ULL,0x70f0f0fULL,0xe0e0e0eULL,0x1c1c1c1cULL,0x38383838ULL,0x70707070ULL,0xe0f0f0f0ULL,0xc0e0e0e0ULL,
            0x307070700ULL,0x70f0f0f00ULL,0xe0e0e0e00ULL,0x1c1c1c1c00ULL,0x3838383800ULL,0x7070707000ULL,0xe0f0f0f000ULL,0xc0e0e0e000ULL,
            0x707070000ULL,0xf0f0f0000ULL,0xe0e0e0000ULL,0x1c1c1c0000ULL,0x3838380000ULL,0x7070700000ULL,0xf0f0f00000ULL,0xe0e0e00000ULL,
            0x70707000000ULL,0xf0f0f000000ULL,0xe0e0e000000ULL,0x1c1c1c000000ULL,0x383838000000ULL,0x707070000000ULL,0xf0f0f0000000ULL,0xe0e0e0000000ULL,
            0x7070703000000ULL,0xf0f0f07000000ULL,0xe0e0e0e000000ULL,0x1c1c1c1c000000ULL,0x38383838000000ULL,0x70707070000000ULL,0xf0f0f0e0000000ULL,0xe0e0e0c0000000ULL,
            0x707070300000000ULL,0xf0f0f0700000000ULL,0xe0e0e0e00000000ULL,0x1c1c1c1c00000000ULL,0x3838383800000000ULL,0x7070707000000000ULL,0xf0f0f0e000000000ULL,0xe0e0e0c000000000ULL,
            0x707030000000000ULL,0xf0f070000000000ULL,0xe0e0e0000000000ULL,0x1c1c1c0000000000ULL,0x3838380000000000ULL,0x7070700000000000ULL,0xf0f0e00000000000ULL,0xe0e0c00000000000ULL,
        };
};

#endif