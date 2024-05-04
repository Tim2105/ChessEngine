#ifndef HANDCRAFTED_EVALUATOR_H
#define HANDCRAFTED_EVALUATOR_H

#include "core/engine/evaluation/EvaluationDefinitons.h"
#include "core/engine/evaluation/Evaluator.h"
#include "core/engine/search/SearchDefinitions.h"
#include "core/utils/hce/HCEParameters.h"

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

        const HCEParameters& hceParams;

        EvaluationVariables evaluationVars;

        std::vector<EvaluationVariables> evaluationHistory;

        void calculateMaterialScore();
        void calculatePawnScore();
        void calculateGamePhase();
        Score calculateKingSafetyScore();
        Score calculatePieceScore();

        Score evaluateKingAttackZone();
        Score evaluateOpenFiles();
        Score evaluatePawnStorm();

        Score evaluatePieceMobility();
        Score evaluateSafeCenterSpace();
        Score evaluateMinorPiecesOnStrongSquares();
        Score evaluateBishopPairs();
        Score evaluateRooksOnOpenFiles();
        Score evaluateRooksBehindPassedPawns();
        Score evaluateKingPawnProximity();

        bool isWinnable(int32_t side);

        int32_t evaluateKNBKEndgame(int32_t ownBishopSq, int32_t oppKingSq);
        int32_t evaluateWinningNoPawnsEndgame(int32_t oppKingSq);

        static constexpr int16_t EG_WINNING_BONUS = 800;
        static constexpr int16_t EG_SPECIAL_MATE_PROGRESS_BONUS = 150;

    public:
        HandcraftedEvaluator(Board& b, const HCEParameters& hceParams) : Evaluator(b), hceParams(hceParams) {
            evaluationHistory.reserve(MAX_PLY);

            calculateMaterialScore();
            calculatePawnScore();
            calculateGamePhase();
        };

        HandcraftedEvaluator(Board& b) : HandcraftedEvaluator(b, HCE_PARAMS) {};

        inline int32_t evaluate() override {
            int32_t numWhitePawns = board.getPieceBitboard(WHITE_PAWN).popcount();
            int32_t numBlackPawns = board.getPieceBitboard(BLACK_PAWN).popcount();
            int32_t numPawns = numWhitePawns + numBlackPawns;

        	// Spezialfall: Keine Bauern mehr auf dem Spielfeld
            if(numPawns == 0) {
                int32_t whiteKingSq = board.getKingSquare(WHITE);
                int32_t blackKingSq = board.getKingSquare(BLACK);

                // Wenn der Materialvorteil kleiner als das Gewicht eines Turms ist, dann ist es Unentschieden
                if(std::abs(evaluationVars.materialScore.eg) < hceParams.getEGWinningMaterialAdvantage()) {

                    // Außer im Fall von 2 Läufer gegen einen Springer (da ist es manchmal möglich zu gewinnen)
                    if(evaluationVars.materialScore.eg > DRAW_SCORE) {
                        if(board.getPieceBitboard(WHITE_KNIGHT).popcount() == 0 && board.getPieceBitboard(WHITE_BISHOP).popcount() == 2 &&
                           board.getPieceBitboard(WHITE_ROOK).popcount() == 0 && board.getPieceBitboard(WHITE_QUEEN).popcount() == 0 &&
                            board.getPieceBitboard(BLACK_KNIGHT).popcount() == 1 && board.getPieceBitboard(BLACK_BISHOP).popcount() == 0)
                            return (evaluateWinningNoPawnsEndgame(blackKingSq) - EG_WINNING_BONUS) * (board.getSideToMove() == WHITE ? 1 : -1) / 10; // Skaliere dei Bewertung runter, da der Sieg nicht sicher ist
                    } else {
                        if(board.getPieceBitboard(BLACK_KNIGHT).popcount() == 0 && board.getPieceBitboard(BLACK_BISHOP).popcount() == 2 &&
                           board.getPieceBitboard(BLACK_ROOK).popcount() == 0 && board.getPieceBitboard(BLACK_QUEEN).popcount() == 0 &&
                            board.getPieceBitboard(WHITE_KNIGHT).popcount() == 1 && board.getPieceBitboard(WHITE_BISHOP).popcount() == 0)
                            return (evaluateWinningNoPawnsEndgame(whiteKingSq) - EG_WINNING_BONUS) * (board.getSideToMove() == WHITE ? -1 : 1) / 10; // Skaliere dei Bewertung runter, da der Sieg nicht sicher ist
                    }

                    return DRAW_SCORE;
                }

                if(evaluationVars.materialScore.eg >= hceParams.getEGWinningMaterialAdvantage()) {
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
                } else if(evaluationVars.materialScore.eg <= -hceParams.getEGWinningMaterialAdvantage()) {
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

            // Bestimme den Materialungleichgewichtsfaktor
            int32_t numWhiteKnights = board.getPieceBitboard(WHITE_KNIGHT).popcount();
            int32_t numBlackKnights = board.getPieceBitboard(BLACK_KNIGHT).popcount();
            int32_t numWhiteBishops = board.getPieceBitboard(WHITE_BISHOP).popcount();
            int32_t numBlackBishops = board.getPieceBitboard(BLACK_BISHOP).popcount();
            int32_t numWhiteRooks = board.getPieceBitboard(WHITE_ROOK).popcount();
            int32_t numBlackRooks = board.getPieceBitboard(BLACK_ROOK).popcount();
            int32_t numWhiteQueens = board.getPieceBitboard(WHITE_QUEEN).popcount();
            int32_t numBlackQueens = board.getPieceBitboard(BLACK_QUEEN).popcount();

            double materialImbalanceFactor = (double)hceParams.getPawnImbalanceFactor(numWhitePawns, numBlackPawns) / (100.0 * hceParams.VALUE_ONE) *
                                             (double)hceParams.getKnightImbalanceFactor(numWhiteKnights, numBlackKnights) / (100.0 * hceParams.VALUE_ONE) *
                                             (double)hceParams.getBishopImbalanceFactor(numWhiteBishops, numBlackBishops) / (100.0 * hceParams.VALUE_ONE) *
                                             (double)hceParams.getRookImbalanceFactor(numWhiteRooks, numBlackRooks) / (100.0 * hceParams.VALUE_ONE) *
                                             (double)hceParams.getQueenImbalanceFactor(numWhiteQueens, numBlackQueens) / (100.0 * hceParams.VALUE_ONE);

            Score scaledMaterialScore = {(int32_t)(evaluationVars.materialScore.mg * materialImbalanceFactor),
                                         (int32_t)(evaluationVars.materialScore.eg * materialImbalanceFactor)};

            // Aktualisiere die Königssicherheitsbewertung
            Score kingSafetyScore = calculateKingSafetyScore();

            // Aktualisiere die kontextsensitiven Figurenbewertungen
            Score pieceScore = calculatePieceScore();

            Score score = (scaledMaterialScore + evaluationVars.pawnScore + pieceScore + kingSafetyScore +
                           Score{hceParams.getMGTempoBonus(), hceParams.getEGTempoBonus()} * (board.getSideToMove() == WHITE ? 1 : -1)) /
                           hceParams.VALUE_ONE;

            int32_t evaluation = ((1.0 - evaluationVars.phase) * score.mg + evaluationVars.phase * score.eg) *
                                 (board.getSideToMove() == WHITE ? 1 : -1);

            // Skaliere die Bewertung in Richtung 0, wenn wir uns der 50-Züge-Regel annähern.
            // (Starte erst nach 10 Zügen, damit die Bewertung nicht zu früh verzerrt wird.)
            int32_t fiftyMoveCounter = board.getFiftyMoveCounter();
            if(fiftyMoveCounter > 20)
                evaluation = evaluation * (100 - fiftyMoveCounter) / 80;

            if(!isWinnable(board.getSideToMove()))
                evaluation = std::min(evaluation, (int32_t)DRAW_SCORE);

            if(!isWinnable(board.getSideToMove() ^ COLOR_MASK))
                evaluation = std::max(evaluation, (int32_t)-DRAW_SCORE);

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

        static constexpr Bitboard connectedPawnMask[64] = {
                0x2,0x5,0xA,0x14,0x28,0x50,0xA0,0x40,
                0x200,0x500,0xA00,0x1400,0x2800,0x5000,0xA000,0x4000,
                0x20000,0x50000,0xA0000,0x140000,0x280000,0x500000,0xA00000,0x400000,
                0x2000000,0x5000000,0xA000000,0x14000000,0x28000000,0x50000000,0xA0000000,0x40000000,
                0x200000000,0x500000000,0xA00000000,0x1400000000,0x2800000000,0x5000000000,0xA000000000,0x4000000000,
                0x20000000000,0x50000000000,0xA0000000000,0x140000000000,0x280000000000,0x500000000000,0xA00000000000,0x400000000000,
                0x2000000000000,0x5000000000000,0xA000000000000,0x14000000000000,0x28000000000000,0x50000000000000,0xA0000000000000,0x40000000000000,
                0x200000000000000,0x500000000000000,0xA00000000000000,0x1400000000000000,0x2800000000000000,0x5000000000000000,0xA000000000000000,0x4000000000000000,
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

        static constexpr Bitboard extendedCenter = 0x3c3c3c3c0000;

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
            {0, 1, 2},
            {0, 1, 2},
            {0, 1, 2},
            {2, 3, 4},
            {3, 4, 5},
            {5, 6, 7},
            {5, 6, 7},
            {5, 6, 7}
        };

        static constexpr Bitboard pawnStormMask[2][64] = {
            // White
            {
                0x707070707070700ULL,0x707070707070700ULL,0x707070707070700ULL,0x1c1c1c1c1c1c1c00ULL,0x3838383838383800ULL,0xe0e0e0e0e0e0e000ULL,0xe0e0e0e0e0e0e000ULL,0xe0e0e0e0e0e0e000ULL,
                0x707070707070000ULL,0x707070707070000ULL,0x707070707070000ULL,0x1c1c1c1c1c1c0000ULL,0x3838383838380000ULL,0xe0e0e0e0e0e00000ULL,0xe0e0e0e0e0e00000ULL,0xe0e0e0e0e0e00000ULL,
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
                0x70707070707ULL,0x70707070707ULL,0x70707070707ULL,0x1c1c1c1c1c1cULL,0x383838383838ULL,0xe0e0e0e0e0e0ULL,0xe0e0e0e0e0e0ULL,0xe0e0e0e0e0e0ULL,
                0x7070707070707ULL,0x7070707070707ULL,0x7070707070707ULL,0x1c1c1c1c1c1c1cULL,0x38383838383838ULL,0xe0e0e0e0e0e0e0ULL,0xe0e0e0e0e0e0e0ULL,0xe0e0e0e0e0e0e0ULL,
            }
        };
};

#endif