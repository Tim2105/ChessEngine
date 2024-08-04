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
            Bitboard whiteCandidatePassedPawns; // Weiße Freibauerkandidaten
            Bitboard blackCandidatePassedPawns; // Schwarze Freibauerkandidaten
            Bitboard whiteImmobilePawns; // Weiße unbewegbare Bauern
            Bitboard blackImmobilePawns; // Schwarze unbewegbare Bauern
            Bitboard whiteStrongSquares; // Starke Felder für weiße Figuren
            Bitboard blackStrongSquares; // Starke Felder für schwarze Figuren
            double phase; // Aktuelle Spielphase (0 = Mittelspiel, 1 = Endspiel)
            int phaseWeight; // Materialgewichtung für die Spielphase
        };

        const HCEParameters& hceParams;

        EvaluationVariables evaluationVars;

        std::vector<EvaluationVariables> evaluationHistory;

        void calculateMaterialScore();
        void calculatePawnScore();
        void calculateGamePhase();
        Score calculateKingSafetyScore();
        Score calculatePieceScore();

        int evaluateKingAttackZone();
        int evaluateOpenFiles();
        int evaluatePawnSafety();

        Score evaluateAttackedPieces();
        Score evaluatePieceMobility();
        Score evaluateSafeCenterSpace();
        Score evaluateMinorPiecesOnStrongSquares();
        Score evaluateBadBishops();
        Score evaluateRooksOnOpenFiles();
        Score evaluateRooksBehindPassedPawns();
        Score evaluateBlockedPassedPawns();
        Score evaluateKingPawnProximity();

        Score evaluateRuleOfTheSquare();

        bool isWinnable(int side);
        bool isOppositeColorBishopEndgame();

        int evaluateKNBKEndgame(int ownBishopSq, int oppKingSq);
        int evaluateWinningNoPawnsEndgame(int oppKingSq);

        static constexpr int EG_WINNING_BONUS = 800;
        static constexpr int EG_SPECIAL_MATE_PROGRESS_BONUS = 150;

    public:
        HandcraftedEvaluator(Board& b, const HCEParameters& hceParams) : Evaluator(b), hceParams(hceParams) {
            evaluationHistory.reserve(MAX_PLY);

            calculateMaterialScore();
            calculatePawnScore();
            calculateGamePhase();
        };

        HandcraftedEvaluator(Board& b) : HandcraftedEvaluator(b, HCE_PARAMS) {};

        inline int evaluate() override {
            int numWhitePawns = board.getPieceBitboard(WHITE_PAWN).popcount();
            int numBlackPawns = board.getPieceBitboard(BLACK_PAWN).popcount();
            int numPawns = numWhitePawns + numBlackPawns;

        	// Spezialfall: Keine Bauern mehr auf dem Spielfeld
            if(numPawns == 0) {
                int whiteKingSq = board.getKingSquare(WHITE);
                int blackKingSq = board.getKingSquare(BLACK);

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

            // Aktualisiere die Königssicherheitsbewertung
            Score kingSafetyScore = calculateKingSafetyScore();

            // Aktualisiere die kontextsensitiven Figurenbewertungen
            Score pieceScore = calculatePieceScore();

            Score score = (evaluationVars.materialScore + evaluationVars.pawnScore + pieceScore + kingSafetyScore +
                           Score{hceParams.getMGTempoBonus(), hceParams.getEGTempoBonus()} * (board.getSideToMove() == WHITE ? 1 : -1));

            int evaluation = ((1.0 - evaluationVars.phase) * score.mg + evaluationVars.phase * score.eg) *
                              (board.getSideToMove() == WHITE ? 1 : -1);

            // Endspiele mit Bauern und Läufern auf unterschiedlichen
            // Feldern sind schwierig zu gewinnen
            if(isOppositeColorBishopEndgame()) {
                int evaluationSign = evaluation >= 0 ? 1 : -1;
                evaluation -= hceParams.getLinearPieceValue(PAWN) * evaluationSign;

                if(evaluationSign == 1)
                    evaluation = std::max(evaluation, DRAW_SCORE);
                else
                    evaluation = std::min(evaluation, -DRAW_SCORE);
            }

            // Skaliere die Bewertung in Richtung 0, wenn wir uns der 50-Züge-Regel annähern.
            // (Starte erst nach 10 Zügen, damit die Bewertung nicht zu früh verzerrt wird.)
            int fiftyMoveCounter = board.getFiftyMoveCounter();
            if(fiftyMoveCounter > 20)
                evaluation = evaluation * (100 - fiftyMoveCounter) / 80;

            if(!isWinnable(board.getSideToMove()))
                evaluation = std::min(evaluation, DRAW_SCORE);

            if(!isWinnable(board.getSideToMove() ^ COLOR_MASK))
                evaluation = std::max(evaluation, -DRAW_SCORE);

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
        static constexpr int PAWN_WEIGHT = 0;
        static constexpr int KNIGHT_WEIGHT = 1;
        static constexpr int BISHOP_WEIGHT = 1;
        static constexpr int ROOK_WEIGHT = 2;
        static constexpr int QUEEN_WEIGHT = 4;

        static constexpr int PIECE_WEIGHT[7] = {
            0, // Empty
            PAWN_WEIGHT,
            KNIGHT_WEIGHT,
            BISHOP_WEIGHT,
            ROOK_WEIGHT,
            QUEEN_WEIGHT,
            0 // King
        };

        static constexpr int TOTAL_WEIGHT = PAWN_WEIGHT * 16 + KNIGHT_WEIGHT * 4 + BISHOP_WEIGHT * 4 + ROOK_WEIGHT * 4 + QUEEN_WEIGHT * 2;

        // Phasengrenzen, können unter 0 oder über 1 sein,
        // die berechnete Phase wird aber zwischen 0 und 1 eingeschränkt
        static constexpr double MIN_PHASE = -0.5;
        static constexpr double MAX_PHASE = 1.0;

        static constexpr Bitboard lightSquares = 0x55aa55aa55aa55aaULL;

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

        static constexpr Bitboard extendedCenter = 0x3c3c000000ULL;

        static constexpr Bitboard kingAttackZone[64] = {
            0x70707ULL,0x70707ULL,0xe0e0eULL,0x1c1c1cULL,0x383838ULL,0x707070ULL,0xe0e0e0ULL,0xe0e0e0ULL,
            0x7070707ULL,0x7070707ULL,0xe0e0e0eULL,0x1c1c1c1cULL,0x38383838ULL,0x70707070ULL,0xe0e0e0e0ULL,0xe0e0e0e0ULL,
            0x707070707ULL,0x707070707ULL,0xe0e0e0e0eULL,0x1c1c1c1c1cULL,0x3838383838ULL,0x7070707070ULL,0xe0e0e0e0e0ULL,0xe0e0e0e0e0ULL,
            0x70707070700ULL,0x70707070700ULL,0xe0e0e0e0e00ULL,0x1c1c1c1c1c00ULL,0x383838383800ULL,0x707070707000ULL,0xe0e0e0e0e000ULL,0xe0e0e0e0e000ULL,
            0x7070707070000ULL,0x7070707070000ULL,0xe0e0e0e0e0000ULL,0x1c1c1c1c1c0000ULL,0x38383838380000ULL,0x70707070700000ULL,0xe0e0e0e0e00000ULL,0xe0e0e0e0e00000ULL,
            0x707070707000000ULL,0x707070707000000ULL,0xe0e0e0e0e000000ULL,0x1c1c1c1c1c000000ULL,0x3838383838000000ULL,0x7070707070000000ULL,0xe0e0e0e0e0000000ULL,0xe0e0e0e0e0000000ULL,
            0x707070700000000ULL,0x707070700000000ULL,0xe0e0e0e00000000ULL,0x1c1c1c1c00000000ULL,0x3838383800000000ULL,0x7070707000000000ULL,0xe0e0e0e000000000ULL,0xe0e0e0e000000000ULL,
            0x707070000000000ULL,0x707070000000000ULL,0xe0e0e0000000000ULL,0x1c1c1c0000000000ULL,0x3838380000000000ULL,0x7070700000000000ULL,0xe0e0e00000000000ULL,0xe0e0e00000000000ULL,
        };

        static constexpr Bitboard pawnShieldMask[2][64] = {
            // White
            {
                0x70706,0x70705,0xe0e0a,0x1c1c1c14,0x38383828,0x707050,0xe0e0a0,0xe0e060,
                0x7070600,0x7070500,0xe0e0a00,0x1c1c1c1400,0x3838382800,0x70705000,0xe0e0a000,0xe0e06000,
                0x707060000,0x707050000,0xe0e0a0000,0x1c1c1c140000,0x383838280000,0x7070500000,0xe0e0a00000,0xe0e0600000,
                0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
            },
            // Black
            {
                0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                0x60707000000,0x50707000000,0xa0e0e000000,0x141c1c1c0000,0x283838380000,0x507070000000,0xa0e0e0000000,0x60e0e0000000,
                0x6070700000000,0x5070700000000,0xa0e0e00000000,0x141c1c1c000000,0x28383838000000,0x50707000000000,0xa0e0e000000000,0x60e0e000000000,
                0x607070000000000,0x507070000000000,0xa0e0e0000000000,0x141c1c1c00000000,0x2838383800000000,0x5070700000000000,0xa0e0e00000000000,0x60e0e00000000000,
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

        static constexpr Array<int, 3> nearbyFiles[8] = {
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

        static constexpr Bitboard badBishopMask[2][64] = {
            // White
            {
                0x281408040000,0x142814080000,0x281428140000,0x142814280000,0x281428140000,0x142814280000,0x281428100000,0x142810200000,
                0x140804000000,0x281408040000,0x142814080000,0x281428140000,0x142814280000,0x281428100000,0x142810200000,0x281020000000,
                0x80400000000,0x140804000000,0x281408000000,0x142814000000,0x281428000000,0x142810000000,0x281020000000,0x102000000000,
                0x40000000000,0x80400000000,0x140800000000,0x281400000000,0x142800000000,0x281000000000,0x102000000000,0x200000000000,
                0x0,0x40000000000,0x80000000000,0x140000000000,0x280000000000,0x100000000000,0x200000000000,0x0,
                0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
            },
            // Black
            {
                0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                0x0,0x40000,0x80000,0x140000,0x280000,0x100000,0x200000,0x0,
                0x40000,0x4080000,0x8140000,0x14280000,0x28140000,0x10280000,0x20100000,0x200000,
                0x4080000,0x408140000,0x814280000,0x1428140000,0x2814280000,0x1028140000,0x2010280000,0x20100000,
                0x408140000,0x40814280000,0x81428140000,0x142814280000,0x281428140000,0x102814280000,0x201028140000,0x2010280000,
                0x40814280000,0x81428140000,0x142814280000,0x281428140000,0x142814280000,0x281428140000,0x102814280000,0x201028140000,
            }
        };
};

#endif