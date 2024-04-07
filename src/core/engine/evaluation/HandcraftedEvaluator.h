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
            Bitboard whiteBackwardPawns; // Rückständige weiße Bauern
            Bitboard blackBackwardPawns; // Rückständige schwarze Bauern
            Bitboard whitePassedPawns; // Weiße Freibauern
            Bitboard blackPassedPawns; // Schwarze Freibauern
            Bitboard whiteStrongSquares; // Starke Felder für weiße Figuren
            Bitboard blackStrongSquares; // Starke Felder für schwarze Figuren
            double phase; // Aktuelle Spielphase (0 = Mittelspiel, 1 = Endspiel)
            int32_t phaseWeight; // Materialgewichtung für die Spielphase
        };

        EvaluationVariables evaluationVars;

        std::vector<EvaluationVariables> evaluationHistory;

        void calculateMaterialScore();
        void calculatePawnScore();
        void calculateGamePhase();
        int32_t calculateKingSafetyScore();
        int32_t calculatePieceScore();

        int32_t evaluateKingAttackZone();
        int32_t evaluatePawnShield();
        int32_t evaluateOpenFiles();
        int32_t evaluatePawnStorm();

        int32_t evaluatePieceMobility();
        int32_t evaluateMinorPiecesOnStrongSquares();
        int32_t evaluateRooksOnOpenFiles();
        int32_t evaluateKingPawnProximity();

        int32_t evaluateKNBKEndgame(int32_t ownBishopSq, int32_t oppKingSq);
        int32_t evaluateWinningNoPawnsEndgame(int32_t oppKingSq);

        static constexpr int16_t MG_PIECE_VALUE[7] = {
            0, // Empty
            100, // Pawn
            400, // Knight
            410, // Bishop
            600, // Rook
            1200, // Queen
            0 // King
        };

        static constexpr int16_t EG_PIECE_VALUE[7] = {
            0, // Empty
            110, // Pawn
            380, // Knight
            430, // Bishop
            640, // Rook
            1250, // Queen
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

            // Aktualisiere die kontextsensitiven Figurenbewertungen
            int32_t pieceScore = calculatePieceScore();

            int32_t evaluation = ((1.0 - evaluationVars.phase) * score.mg + evaluationVars.phase * score.eg + kingSafetyScore + pieceScore) *
                                 (board.getSideToMove() == WHITE ? 1 : -1);

            evaluation += (1.0 - evaluationVars.phase) * MG_TEMPO_BONUS + evaluationVars.phase * EG_TEMPO_BONUS;

            // Skaliere die Bewertung in Richtung 0, wenn wir uns der 50-Züge-Regel annähern.
            // (Starte erst nach 10 Zügen, damit die Bewertung nicht zu früh verzerrt wird.)
            int32_t fiftyMoveCounter = board.getFiftyMoveCounter();
            if(fiftyMoveCounter > 20)
                evaluation = (int32_t)evaluation * (100 - fiftyMoveCounter) / 80;

            return evaluation;
        }

        inline double getGamePhase() const {
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
            -3, -4, -5, -6, -6, -5, -4, -3
        };

        // Bestrafung für Doppelbauern pro Linie im Endspiel
        static constexpr int16_t EG_DOUBLED_PAWN_PENALTY[8] = {
            -5, -7, -8, -10, -10, -8, -7, -5
        };

        // Bestrafung für isolierte Bauern pro Linie im Mittelspiel
        static constexpr int16_t MG_ISOLATED_PAWN_PENALTY[8] = {
            -7, -7, -8, -9, -9, -8, -7, -7
        };

        // Bestrafung für isolierte Bauern pro Linie im Endspiel
        static constexpr int16_t EG_ISOLATED_PAWN_PENALTY[8] = {
            -7, -9, -10, -12, -12, -10, -9, -7
        };

        // Bestrafung für rückständige Bauern pro Rang im Mittelspiel
        static constexpr int16_t MG_BACKWARD_PAWN_PENALTY[8] = {
            0, -15, -18, -25, -17, 0, 0, 0
        };

        // Bestrafung für rückständige Bauern pro Rang im Endspiel
        static constexpr int16_t EG_BACKWARD_PAWN_PENALTY[8] = {
            0, -18, -16, -5, 0, 0, 0, 0
        };

        // Bonus für Freibauern pro Rang im Mittelspiel
        static constexpr int16_t MG_PASSED_PAWN_BONUS[8] = {
            0, 5, 5, 8, 12, 17, 25, 0
        };

        // Bonus für Freibauern pro Rang im Endspiel
        static constexpr int16_t EG_PASSED_PAWN_BONUS[8] = {
            0, 28, 28, 37, 52, 70, 100, 0
        };

        // Bonus für starke Felder pro Feld im Mittelspiel
        static constexpr int16_t MG_STRONG_SQUARE_BONUS[64] = {
            0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,
            5, 7, 14, 21, 21, 14, 7, 5,
            9, 21, 22, 37, 37, 22, 21, 9,
            17, 26, 24, 21, 21, 24, 26, 17,
            0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0
        };

        // Bonus für starke Felder pro Feld im Endspiel
        static constexpr int16_t EG_STRONG_SQUARE_BONUS[64] = {
            0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,
            0, 1, 2, 6, 6, 2, 1, 0,
            2, 3, 4, 7, 7, 4, 3, 2,
            5, 6, 8, 10, 10, 8, 6, 5,
            0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0
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

        static constexpr Bitboard backwardPawnMask[2][64] = {
            // White
            {
                0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL,
                0x2ULL, 0x5ULL, 0xaULL, 0x14ULL, 0x28ULL, 0x50ULL, 0xa0ULL, 0x40ULL,
                0x202ULL, 0x505ULL, 0xa0aULL, 0x1414ULL, 0x2828ULL, 0x5050ULL, 0xa0a0ULL, 0x4040ULL,
                0x20202ULL, 0x50505ULL, 0xa0a0aULL, 0x141414ULL, 0x282828ULL, 0x505050ULL, 0xa0a0a0ULL, 0x404040ULL,
                0x2020202ULL, 0x5050505ULL, 0xa0a0a0aULL, 0x14141414ULL, 0x28282828ULL, 0x50505050ULL, 0xa0a0a0a0ULL, 0x40404040ULL,
                0x202020202ULL, 0x505050505ULL, 0xa0a0a0a0aULL, 0x1414141414ULL, 0x2828282828ULL, 0x5050505050ULL, 0xa0a0a0a0a0ULL, 0x4040404040ULL,
                0x20202020202ULL, 0x50505050505ULL, 0xa0a0a0a0a0aULL, 0x141414141414ULL, 0x282828282828ULL, 0x505050505050ULL, 0xa0a0a0a0a0a0ULL, 0x404040404040ULL,
                0x2020202020202ULL, 0x5050505050505ULL, 0xa0a0a0a0a0a0aULL, 0x14141414141414ULL, 0x28282828282828ULL, 0x50505050505050ULL, 0xa0a0a0a0a0a0a0ULL, 0x40404040404040ULL,
            },
            // Black
            {
                0x202020202020200ULL, 0x505050505050500ULL, 0xa0a0a0a0a0a0a00ULL, 0x1414141414141400ULL, 0x2828282828282800ULL, 0x5050505050505000ULL, 0xa0a0a0a0a0a0a000ULL, 0x4040404040404000ULL,
                0x202020202020000ULL, 0x505050505050000ULL, 0xa0a0a0a0a0a0000ULL, 0x1414141414140000ULL, 0x2828282828280000ULL, 0x5050505050500000ULL, 0xa0a0a0a0a0a00000ULL, 0x4040404040400000ULL,
                0x202020202000000ULL, 0x505050505000000ULL, 0xa0a0a0a0a000000ULL, 0x1414141414000000ULL, 0x2828282828000000ULL, 0x5050505050000000ULL, 0xa0a0a0a0a0000000ULL, 0x4040404040000000ULL,
                0x202020200000000ULL, 0x505050500000000ULL, 0xa0a0a0a00000000ULL, 0x1414141400000000ULL, 0x2828282800000000ULL, 0x5050505000000000ULL, 0xa0a0a0a000000000ULL, 0x4040404000000000ULL,
                0x202020000000000ULL, 0x505050000000000ULL, 0xa0a0a0000000000ULL, 0x1414140000000000ULL, 0x2828280000000000ULL, 0x5050500000000000ULL, 0xa0a0a00000000000ULL, 0x4040400000000000ULL,
                0x202000000000000ULL, 0x505000000000000ULL, 0xa0a000000000000ULL, 0x1414000000000000ULL, 0x2828000000000000ULL, 0x5050000000000000ULL, 0xa0a0000000000000ULL, 0x4040000000000000ULL,
                0x200000000000000ULL, 0x500000000000000ULL, 0xa00000000000000ULL, 0x1400000000000000ULL, 0x2800000000000000ULL, 0x5000000000000000ULL, 0xa000000000000000ULL, 0x4000000000000000ULL,
                0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL,
            }
        };

        static constexpr Bitboard sentryMask[2][64] = {
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

        static constexpr Bitboard strongSquareMask[2][64] = {
            // White
            {
                0x202020202020200ULL,0x505050505050500ULL,0xa0a0a0a0a0a0a00ULL,0x1414141414141400ULL,0x2828282828282800ULL,0x5050505050505000ULL,0xa0a0a0a0a0a0a000ULL,0x4040404040404000ULL,
                0x202020202020000ULL,0x505050505050000ULL,0xa0a0a0a0a0a0000ULL,0x1414141414140000ULL,0x2828282828280000ULL,0x5050505050500000ULL,0xa0a0a0a0a0a00000ULL,0x4040404040400000ULL,
                0x202020202000000ULL,0x505050505000000ULL,0xa0a0a0a0a000000ULL,0x1414141414000000ULL,0x2828282828000000ULL,0x5050505050000000ULL,0xa0a0a0a0a0000000ULL,0x4040404040000000ULL,
                0x202020200000000ULL,0x505050500000000ULL,0xa0a0a0a00000000ULL,0x1414141400000000ULL,0x2828282800000000ULL,0x5050505000000000ULL,0xa0a0a0a000000000ULL,0x4040404000000000ULL,
                0x202020000000000ULL,0x505050000000000ULL,0xa0a0a0000000000ULL,0x1414140000000000ULL,0x2828280000000000ULL,0x5050500000000000ULL,0xa0a0a00000000000ULL,0x4040400000000000ULL,
                0x202000000000000ULL,0x505000000000000ULL,0xa0a000000000000ULL,0x1414000000000000ULL,0x2828000000000000ULL,0x5050000000000000ULL,0xa0a0000000000000ULL,0x4040000000000000ULL,
                0x200000000000000ULL,0x500000000000000ULL,0xa00000000000000ULL,0x1400000000000000ULL,0x2800000000000000ULL,0x5000000000000000ULL,0xa000000000000000ULL,0x4000000000000000ULL,
                0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,
            },
            // Black
            {
                0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,
                0x2ULL,0x5ULL,0xaULL,0x14ULL,0x28ULL,0x50ULL,0xa0ULL,0x40ULL,
                0x202ULL,0x505ULL,0xa0aULL,0x1414ULL,0x2828ULL,0x5050ULL,0xa0a0ULL,0x4040ULL,
                0x20202ULL,0x50505ULL,0xa0a0aULL,0x141414ULL,0x282828ULL,0x505050ULL,0xa0a0a0ULL,0x404040ULL,
                0x2020202ULL,0x5050505ULL,0xa0a0a0aULL,0x14141414ULL,0x28282828ULL,0x50505050ULL,0xa0a0a0a0ULL,0x40404040ULL,
                0x202020202ULL,0x505050505ULL,0xa0a0a0a0aULL,0x1414141414ULL,0x2828282828ULL,0x5050505050ULL,0xa0a0a0a0a0ULL,0x4040404040ULL,
                0x20202020202ULL,0x50505050505ULL,0xa0a0a0a0a0aULL,0x141414141414ULL,0x282828282828ULL,0x505050505050ULL,0xa0a0a0a0a0a0ULL,0x404040404040ULL,
                0x2020202020202ULL,0x5050505050505ULL,0xa0a0a0a0a0a0aULL,0x14141414141414ULL,0x28282828282828ULL,0x50505050505050ULL,0xa0a0a0a0a0a0a0ULL,0x40404040404040ULL,
            }
        };

        static constexpr int32_t NUM_ATTACKER_WEIGHT[6] = {
            0, 0, 50, 75, 90, 100
        };

        static constexpr size_t NUM_ATTACKER_WEIGHT_SIZE = sizeof(NUM_ATTACKER_WEIGHT) / sizeof(NUM_ATTACKER_WEIGHT[0]);

        static constexpr int32_t KNIGHT_ATTACK_BONUS = 12;
        static constexpr int32_t BISHOP_ATTACK_BONUS = 12;
        static constexpr int32_t ROOK_ATTACK_BONUS = 38;
        static constexpr int32_t QUEEN_ATTACK_BONUS = 75;

        // Ein Bonus für jede Figur, die sich innerhalb der eigenen Königszone befindet
        static constexpr int32_t MINOR_PIECE_DEFENDER_BONUS = 17;

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

        static constexpr int32_t PAWN_SHIELD_SIZE_BONUS[4] = {
            0, 15, 52, 61
        };

        static constexpr size_t PAWN_SHIELD_SIZE_BONUS_SIZE = sizeof(PAWN_SHIELD_SIZE_BONUS) / sizeof(PAWN_SHIELD_SIZE_BONUS[0]);

        static constexpr Bitboard pawnShieldMask[2][64] = {
            // White
            {
                0x30302ULL,0x70705ULL,0xe0e0aULL,0x1c1c14ULL,0x383828ULL,0x707050ULL,0xe0e0a0ULL,0xc0c040ULL,
                0x3030200ULL,0x7070500ULL,0xe0e0a00ULL,0x1c1c1400ULL,0x38382800ULL,0x70705000ULL,0xe0e0a000ULL,0xc0c04000ULL,
                0x303020000ULL,0x707050000ULL,0xe0e0a0000ULL,0x1c1c140000ULL,0x3838280000ULL,0x7070500000ULL,0xe0e0a00000ULL,0xc0c0400000ULL,
                0x30302000000ULL,0x70705000000ULL,0xe0e0a000000ULL,0x1c1c14000000ULL,0x383828000000ULL,0x707050000000ULL,0xe0e0a0000000ULL,0xc0c040000000ULL,
                0x3030200000000ULL,0x7070500000000ULL,0xe0e0a00000000ULL,0x1c1c1400000000ULL,0x38382800000000ULL,0x70705000000000ULL,0xe0e0a000000000ULL,0xc0c04000000000ULL,
                0x303020000000000ULL,0x707050000000000ULL,0xe0e0a0000000000ULL,0x1c1c140000000000ULL,0x3838280000000000ULL,0x7070500000000000ULL,0xe0e0a00000000000ULL,0xc0c0400000000000ULL,
                0x302000000000000ULL,0x705000000000000ULL,0xe0a000000000000ULL,0x1c14000000000000ULL,0x3828000000000000ULL,0x7050000000000000ULL,0xe0a0000000000000ULL,0xc040000000000000ULL,
                0x200000000000000ULL,0x500000000000000ULL,0xa00000000000000ULL,0x1400000000000000ULL,0x2800000000000000ULL,0x5000000000000000ULL,0xa000000000000000ULL,0x4000000000000000ULL,
            },
            // Black
            {
                0x2ULL,0x5ULL,0xaULL,0x14ULL,0x28ULL,0x50ULL,0xa0ULL,0x40ULL,
                0x203ULL,0x507ULL,0xa0eULL,0x141cULL,0x2838ULL,0x5070ULL,0xa0e0ULL,0x40c0ULL,
                0x20303ULL,0x50707ULL,0xa0e0eULL,0x141c1cULL,0x283838ULL,0x507070ULL,0xa0e0e0ULL,0x40c0c0ULL,
                0x2030300ULL,0x5070700ULL,0xa0e0e00ULL,0x141c1c00ULL,0x28383800ULL,0x50707000ULL,0xa0e0e000ULL,0x40c0c000ULL,
                0x203030000ULL,0x507070000ULL,0xa0e0e0000ULL,0x141c1c0000ULL,0x2838380000ULL,0x5070700000ULL,0xa0e0e00000ULL,0x40c0c00000ULL,
                0x20303000000ULL,0x50707000000ULL,0xa0e0e000000ULL,0x141c1c000000ULL,0x283838000000ULL,0x507070000000ULL,0xa0e0e0000000ULL,0x40c0c0000000ULL,
                0x2030300000000ULL,0x5070700000000ULL,0xa0e0e00000000ULL,0x141c1c00000000ULL,0x28383800000000ULL,0x50707000000000ULL,0xa0e0e000000000ULL,0x40c0c000000000ULL,
                0x203030000000000ULL,0x507070000000000ULL,0xa0e0e0000000000ULL,0x141c1c0000000000ULL,0x2838380000000000ULL,0x5070700000000000ULL,0xa0e0e00000000000ULL,0x40c0c00000000000ULL,
            }
        };

        // Bestrafung für offene Linien (keine eigenen Bauern) in der Nähe des Königs
        static constexpr int32_t KING_OPEN_FILE_BONUS[4] = {
            0, -11, -46, -60
        };

        static constexpr Bitboard fileMasks[8] = {
            0x101010101010101ULL,
            0x202020202020202ULL,
            0x404040404040404ULL,
            0x808080808080808ULL,
            0x1010101010101010ULL,
            0x2020202020202020ULL,
            0x4040404040404040ULL,
            0x8080808080808080ULL
        };

        static constexpr Array<int32_t, 3> nearbyFiles[8] = {
            {0, 1},
            {0, 1, 2},
            {1, 2, 3},
            {2, 3, 4},
            {3, 4, 5},
            {4, 5, 6},
            {5, 6, 7},
            {6, 7}
        };

        // Bestrafung für fortgeschrittene gegnerische Bauern
        // in der Nähe des Königs pro Rang
        static constexpr int32_t PAWN_STORM_BONUS[8] = {
            0, -2, -2, -10, -18, -30, -25, 0
        };

        static constexpr Bitboard pawnStormMask[2][64] = {
            // White
            {
                0x303030303030300ULL,0x707070707070700ULL,0xe0e0e0e0e0e0e00ULL,0x1c1c1c1c1c1c1c00ULL,0x3838383838383800ULL,0x7070707070707000ULL,0xe0e0e0e0e0e0e000ULL,0xc0c0c0c0c0c0c000ULL,
                0x303030303030000ULL,0x707070707070000ULL,0xe0e0e0e0e0e0000ULL,0x1c1c1c1c1c1c0000ULL,0x3838383838380000ULL,0x7070707070700000ULL,0xe0e0e0e0e0e00000ULL,0xc0c0c0c0c0c00000ULL,
                0x303030303000000ULL,0x707070707000000ULL,0xe0e0e0e0e000000ULL,0x1c1c1c1c1c000000ULL,0x3838383838000000ULL,0x7070707070000000ULL,0xe0e0e0e0e0000000ULL,0xc0c0c0c0c0000000ULL,
                0x303030300000000ULL,0x707070700000000ULL,0xe0e0e0e00000000ULL,0x1c1c1c1c00000000ULL,0x3838383800000000ULL,0x7070707000000000ULL,0xe0e0e0e000000000ULL,0xc0c0c0c000000000ULL,
                0x303030000000000ULL,0x707070000000000ULL,0xe0e0e0000000000ULL,0x1c1c1c0000000000ULL,0x3838380000000000ULL,0x7070700000000000ULL,0xe0e0e00000000000ULL,0xc0c0c00000000000ULL,
                0x303000000000000ULL,0x707000000000000ULL,0xe0e000000000000ULL,0x1c1c000000000000ULL,0x3838000000000000ULL,0x7070000000000000ULL,0xe0e0000000000000ULL,0xc0c0000000000000ULL,
                0x300000000000000ULL,0x700000000000000ULL,0xe00000000000000ULL,0x1c00000000000000ULL,0x3800000000000000ULL,0x7000000000000000ULL,0xe000000000000000ULL,0xc000000000000000ULL,
                0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,
            },
            // Black
            {
                0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,
                0x3ULL,0x7ULL,0xeULL,0x1cULL,0x38ULL,0x70ULL,0xe0ULL,0xc0ULL,
                0x303ULL,0x707ULL,0xe0eULL,0x1c1cULL,0x3838ULL,0x7070ULL,0xe0e0ULL,0xc0c0ULL,
                0x30303ULL,0x70707ULL,0xe0e0eULL,0x1c1c1cULL,0x383838ULL,0x707070ULL,0xe0e0e0ULL,0xc0c0c0ULL,
                0x3030303ULL,0x7070707ULL,0xe0e0e0eULL,0x1c1c1c1cULL,0x38383838ULL,0x70707070ULL,0xe0e0e0e0ULL,0xc0c0c0c0ULL,
                0x303030303ULL,0x707070707ULL,0xe0e0e0e0eULL,0x1c1c1c1c1cULL,0x3838383838ULL,0x7070707070ULL,0xe0e0e0e0e0ULL,0xc0c0c0c0c0ULL,
                0x30303030303ULL,0x70707070707ULL,0xe0e0e0e0e0eULL,0x1c1c1c1c1c1cULL,0x383838383838ULL,0x707070707070ULL,0xe0e0e0e0e0e0ULL,0xc0c0c0c0c0c0ULL,
                0x3030303030303ULL,0x7070707070707ULL,0xe0e0e0e0e0e0eULL,0x1c1c1c1c1c1c1cULL,0x38383838383838ULL,0x70707070707070ULL,0xe0e0e0e0e0e0e0ULL,0xc0c0c0c0c0c0c0ULL,
            }
        };

        // Bonus für alle Felder, die von einer Figur im nächsten Zug erreicht werden können (Mittelspiel)
        static constexpr int32_t MG_PIECE_MOBILITY_BONUS[6] = {
            0, // Empty
            0, // Pawn
            3, // Knight
            4, // Bishop
            2, // Rook
            0, // Queen
        };

        // Bonus für alle Felder, die von einer Figur im nächsten Zug erreicht werden können (Endspiel)
        static constexpr int32_t EG_PIECE_MOBILITY_BONUS[6] = {
            0, // Empty
            0, // Pawn
            1, // Knight
            1, // Bishop
            1, // Rook
            0, // Queen
        };

        // Bonus für alle starken Felder, auf denen eine Leichtfigur steht,
        // oder auf die eine Leichtfigur im nächsten Zug ziehen kann
        static constexpr int32_t MINOR_PIECE_ON_STRONG_SQUARE_BONUS = 23;

        // Bonus für Türme auf offenen Linien
        static constexpr int32_t MG_ROOK_ON_OPEN_FILE_BONUS = 20;

        // Bonus für Türme auf halboffenen Linien
        static constexpr int32_t MG_ROOK_ON_SEMI_OPEN_FILE_BONUS = 10;

        // Bonus für Könige in der Nähe von Bauern
        static constexpr int32_t EG_KING_PAWN_PROXIMITY_BONUS = 1;

        // Gewichte für die Bewertung der Königsposition abhängig von den Bauern
        static constexpr int32_t EG_KING_PROXIMITY_PAWN_WEIGHT = 2;
        static constexpr int32_t EG_KING_PROXIMITY_BACKWARD_PAWN_WEIGHT = 3;
        static constexpr int32_t EG_KING_PROXIMITY_PASSED_PAWN_WEIGHT = 5;
};

#endif