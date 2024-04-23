#ifndef HCE_PARAMETERS_H
#define HCE_PARAMETERS_H

#include <fstream>
#include <stdint.h>

class HCEParameters {

    private:

        /**
         * Bewertungen der Figuren (Bauer, Springer, Läufer, Turm, Dame)
         * im Mittel- und Endspiel.
         */

        int16_t MG_PIECE_VALUE[5] = {
            100, // Pawn
            400, // Knight
            410, // Bishop
            600, // Rook
            1200 // Queen
        };

        int16_t EG_PIECE_VALUE[5] = {
            110, // Pawn
            380, // Knight
            430, // Bishop
            640, // Rook
            1250 // Queen
        };

        /**
         * Positionstabellen für alle Figuren im Mittel- und Endspiel.
         */

        int16_t MG_PSQT[6][64] = {
            // Pawn
            {
                    0, 0, 0, 0, 0, 0, 0, 0, // tote Parameterreihe
                    11, 6, -14, -8, -8, -13, 5, 11,
                    7, 4, -16, -4, -4, -15, 4, 7,
                    15, -1, 8, 19, 20, 8, -3, 12,
                    19, -8, 2, 23, 23, 2, -14, 16,
                    9, -26, -20, 15, 15, -15, -26, 10,
                    3, -5, 9, 28, 28, 9, -5, 3,
                    0, 0, 0, 0, 0, 0, 0, 0, // tote Parameterreihe
            },
            // Knight
            {
                    -74, -25, -27, -31, -30, -22, -20, -66,
                    -45, -9, -11, -9, -7, -23, -20, -33,
                    -30, -1, 5, -6, -8, 5, -10, -31,
                    -9, 7, 13, 18, 20, 15, 8, -8,
                    4, 12, 18, 21, 22, 18, 12, 9,
                    2, 9, 2, -10, -3, 3, 10, 6,
                    -8, -19, -13, -13, -30, -13, -19, -8,
                    -92, -55, -18, -14, -14, -6, -53, -92,
            },
            // Bishop
            {
                    -41, -22, -10, -3, -4, -14, -27, -48,
                    -34, 16, -1, 4, 3, 0, 15, -33,
                    -7, -12, 4, 15, 15, -6, -17, -10,
                    -15, 8, 14, 4, 6, 19, 10, -7,
                    -27, -12, -7, -2, 0, -3, -5, -29,
                    -5, -21, -18, -4, -5, -13, -21, 14,
                    -23, -9, -12, -26, -20, -17, -10, -22,
                    -52, -23, -19, -12, -11, -20, -25, -55,
            },
            // Rook
            {
                    -12, -17, 3, 11, 9, 0, -18, -14,
                    -15, -8, -1, 4, 4, -4, -10, -16,
                    -26, -9, -8, -3, -5, -16, -17, -29,
                    -16, -9, -5, 6, 9, -10, -18, -30,
                    0, 6, 16, 23, 22, 14, 6, -1,
                    27, 18, 14, 10, 7, 13, 15, 24,
                    38, 30, 41, 46, 42, 32, 25, 27,
                    32, 27, 34, 34, 32, 27, 19, 26,
            },
            // Queen
            {
                    -45, -39, -8, 10, -15, -36, -51, -63,
                    -27, -35, 3, 0, -1, -7, -47, -40,
                    -8, 14, 5, 3, 2, -12, -3, -21,
                    -24, -19, -2, -34, -16, 4, -15, -21,
                    -20, -21, -25, -28, -24, -17, -19, -19,
                    15, 11, 6, -8, -10, 0, 7, 12,
                    30, 37, 21, 8, 7, 18, 33, 28,
                    35, 47, 43, 25, 21, 36, 45, 35,

            },
            // King
            {
                    -10, 17, -1, -26, -17, -18, 21, -9,
                    -18, 7, -24, -34, -31, -20, 9, -17,
                    -25, -27, -29, -36, -36, -29, -17, -25,
                    -39, -48, -57, -65, -65, -57, -48, -39,
                    -46, -53, -64, -72, -72, -64, -53, -46,
                    -35, -50, -65, -78, -78, -65, -50, -35,
                    -29, -37, -53, -60, -60, -53, -37, -29,
                    -21, -29, -40, -47, -47, -40, -29, -21,
            }
        };
        int16_t EG_PSQT[6][64] = {
            // Pawn
            {
                    0, 0, 0, 0, 0, 0, 0, 0, // tote Parameterreihe
                    -21, -24, -31, -39, -40, -18, -10, -25,
                    -32, -28, -43, -39, -40, -33, -24, -25,
                    -10, -9, -14, -17, -17, -6, 5, -4,
                    1, 1, 5, 8, 10, -5, 9, 2,
                    28, 23, 22, 19, 18, 18, 20, 25,
                    53, 45, 39, 34, 31, 35, 41, 49,
                    0, 0, 0, 0, 0, 0, 0, 0, // tote Parameterreihe
            },
            // Knight
            {
                    -32, -25, -9, -11, -7, -11, -25, -14, 
                    -22, -11, -10, -1, -2, -5, -10, -21, 
                    -11, -10, -1, 5, 7, 0, -1, -11, 
                    -9, 2, 8, 8, 12, 8, -3, -9, 
                    -9, 4, 5, 11, 11, 11, 1, -8, 
                    -20, -9, -4, 0, 4, 5, -10, -12, 
                    -26, -12, -12, -4, -1, -12, -4, -12, 
                    -49, -31, -13, -15, -14, -6, -19, -29, 
            },
            // Bishop
            {
                    -8, -2, -8, -4, -2, -11, -4, -11, 
                    -13, -7, -4, 2, 0, -3, -9, -7, 
                    -7, -3, 1, 6, 5, 4, -1, -6, 
                    -4, -1, 5, 3, 9, 6, 1, -3, 
                    1, 1, 5, 7, 4, 6, 4, -1, 
                    2, 0, 3, -1, 0, 0, -4, 1, 
                    -7, -2, -6, -1, -6, 3, -2, -4, 
                    -12, -8, -4, -3, -4, -5, -10, -7, 
            },
            // Rook
            {
                    -10, 2, -6, -2, 0, 1, 1, -4, 
                    -1, -5, -4, -4, 1, 0, -3, -3, 
                    -8, -4, -6, -3, 0, -2, 0, -2, 
                    -5, -4, -3, -2, 2, 4, 2, 1, 
                    1, 0, 0, 1, 0, 6, 1, 2, 
                    -1, -2, -1, 2, 2, 3, 3, 3, 
                    1, 4, 1, -1, 5, 6, 6, 5, 
                    2, 4, 6, 6, 7, 9, 5, 6, 
            },
            // Queen
            {
                    -4, -4, -3, 0, 1, -5, -6, -5,
                    -5, 0, -1, 2, 1, -2, 0, -4,
                    -3, -1, 2, 4, 3, 2, 0, -2,
                    -1, 1, 3, 4, 5, 3, 2, 0,
                    -2, -1, 1, 3, 2, 2, 1, -1,
                    1, 2, 4, 6, 6, 5, 3, 0,
                    3, 5, 2, 8, 9, 6, 5, 2,
                    7, 5, 4, 11, 11, 8, 6, 6,
            },
            // King
            {
                    -43, -35, -39, -30, -31, -37, -29, -47,
                    -35, -26, -30, -21, -20, -28, -24, -28,
                    -17, -9, -7, 4, 6, -10, -8, -19,
                    -4, 0, 5, 15, 13, 1, -1, -10,
                    12, 7, 12, 32, 35, 10, 8, 14,
                    26, 13, 16, 26, 28, 14, 11, 22,
                    28, 26, 24, 18, 17, 23, 27, 31,
                    -4, 11, 5, -1, 0, 2, 12, -1,
            }
    };

        /**
         * Bonus für das Zugrecht im Mittel- und Endspiel.
         */

        int16_t MG_TEMPO_BONUS = 25;
        int16_t EG_TEMPO_BONUS = 16;

        /**
         * Bewertungen für verschiedene Merkmale in
         * Bauernstrukturen im Mittel- und Endspiel.
         */

        int16_t MG_CONNECTED_PAWN_BONUS[6] = {
            0, 3, 6, 8, 10, 12
        }; // pro Rang (2 - 7)
        int16_t EG_CONNECTED_PAWN_BONUS[6] = {
            2, 4, 10, 11, 13, 15
        }; // pro Rang (2 - 7)
        int16_t MG_DOUBLED_PAWN_PENALTY[8] = {
            -3, -4, -5, -6, -6, -5, -4, -3
        }; // pro Linie
        int16_t EG_DOUBLED_PAWN_PENALTY[8] = {
            -5, -7, -8, -10, -10, -8, -7, -5
        }; // pro Linie
        int16_t MG_ISOLATED_PAWN_PENALTY[8] = {
            -7, -7, -8, -9, -9, -8, -7, -7
        }; // pro Linie
        int16_t EG_ISOLATED_PAWN_PENALTY[8] = {
            -7, -9, -10, -12, -12, -10, -9, -7
        }; // pro Linie
        int16_t MG_BACKWARD_PAWN_PENALTY[6] = {
            -15, -18, -25, -17, 0, 0
        }; // pro Rang (2 - 7)
        int16_t EG_BACKWARD_PAWN_PENALTY[6] = {
            -18, -16, -5, 0, 0, 0
        }; // pro Rang (2 - 7)
        int16_t MG_PASSED_PAWN_BONUS[6] = {
            5, 5, 8, 12, 17, 25
        }; // pro Rang (2 - 7)
        int16_t EG_PASSED_PAWN_BONUS[6] = {
            28, 28, 37, 52, 70, 100
        }; // pro Rang (2 - 7)

        /**
         * Bewertungen für starke Felder im Mittel- und Endspiel.
         * Starke Felder sind Felder, die von einem eigenen Bauern
         * gedeckt sind und nie von einem gegnerischen Bauern
         * angegriffen werden können.
         */

        int16_t MG_STRONG_SQUARE_BONUS[5][8] = {
            {0, 0, 0, 0, 0, 0, 0, 0},
            {5, 7, 14, 21, 21, 14, 7, 5},
            {9, 21, 22, 30, 30, 22, 21, 9},
            {17, 26, 24, 25, 25, 24, 26, 17},
            {24, 29, 30, 34, 34, 30, 29, 24}
        }; // pro Rang (3 - 7) und Linie
        int16_t EG_STRONG_SQUARE_BONUS[5][8] = {
            {0, 0, 0, 0, 0, 0, 0, 0},
            {0, 1, 2, 6, 6, 2, 1, 0},
            {2, 3, 4, 7, 7, 4, 3, 2},
            {5, 6, 8, 10, 10, 8, 6, 5},
            {8, 10, 12, 14, 14, 12, 10, 8}
        }; // pro Rang (3 - 7) und Linie

        /**
         * Ein Bonus für jedes sichere Feld für jede Figur
         * in der Mitte des Spielfeldes.
         */

        int16_t MG_SPACE_BONUS_PER_PIECE = 4;

        /**
         * Ein Bonus für jedes Feld im Königsbereich des Gegners,
         * dass von einer eigenen Figur angegriffen wird.
         */

        int16_t NUM_ATTACKER_WEIGHT[5] = {
            0, 50, 75, 90, 100
        };

        int16_t KNIGHT_ATTACK_BONUS = 16;
        int16_t BISHOP_ATTACK_BONUS = 16;
        int16_t ROOK_ATTACK_BONUS = 40;
        int16_t QUEEN_ATTACK_BONUS = 82;

        /**
         * Ein Bonus für jede Leichtfigur, die den eigenen
         * Königsbereich verteidigt.
         */

        int16_t MINOR_PIECE_DEFENDER_BONUS = 20;

        /**
         * Bewertungen für verschiedene Merkmale in
         * Zusammenhang mit der Königssicherheit.
         */

        int16_t PAWN_SHIELD_SIZE_BONUS[4] = {
            0, 27, 95, 110
        };
        int16_t KING_OPEN_FILE_PENALTY[4] = {
            0, -31, -76, -85
        };
        int16_t PAWN_STORM_BONUS[6] = {
            -2, -2, -10, -18, -30, -25
        }; // pro Rang (2 - 7)

        /**
         * Bewertungen für die Mobilität der Figuren
         * im Mittel- und Endspiel. Die Mobilität ist
         * die Anzahl der, im nächsten Zug, erreichbaren
         * Felder, die nicht von gegnerischen Bauern
         * angegriffen werden.
         * (Springer, Läufer, Turm, Dame)
         */

        int16_t MG_PIECE_MOBILITY_BONUS[4] = {
            3, // Knight
            4, // Bishop
            2, // Rook
            0, // Queen
        };
        int16_t EG_PIECE_MOBILITY_BONUS[4] = {
            1, // Knight
            1, // Bishop
            1, // Rook
            0, // Queen
        };

        /**
         * Ein Bonus für jede Leichtfigur,
         * die auf einem starken Feld steht.
         */

        int16_t MG_MINOR_PIECE_ON_STRONG_SQUARE_BONUS = 23;

        /**
         * Ein Bonus für das Läuferpaar im Mittel- und Endspiel.
         */

        int16_t MG_BISHOP_PAIR_BONUS = 15;
        int16_t EG_BISHOP_PAIR_BONUS = 30;

        /**
         * Ein Bonus für jede offene oder halboffene
         * Linie, auf der sich ein Turm befindet.
         * (Nur Mittelspiel)
         */

        int16_t MG_ROOK_ON_OPEN_FILE_BONUS = 20;
        int16_t MG_ROOK_ON_SEMI_OPEN_FILE_BONUS = 10;

        /**
         * Ein Bonus für jeden Freibauern im Endspiel,
         * der von einem Turm verteidigt/blockiert wird.
         */
        int16_t EG_ROOK_BEHIND_PASSED_PAWN_BONUS = 30;

        /**
         * Gewichtungsparameter für die Bewertung der Distanz
         * des Königs zu Bauern im Endspiel.
         */

        int16_t EG_KING_PROXIMITY_PAWN_WEIGHT = 2;
        int16_t EG_KING_PROXIMITY_BACKWARD_PAWN_WEIGHT = 3;
        int16_t EG_KING_PROXIMITY_PASSED_PAWN_WEIGHT = 5;

    public:
        HCEParameters();
        HCEParameters(std::istream& stream);

        ~HCEParameters();

        void loadParameters(std::istream& stream);
        void saveParameters(std::ostream& stream) const;

        int16_t& operator[](size_t index);
        int16_t operator[](size_t index) const;

        static constexpr size_t size() { return sizeof(HCEParameters) / sizeof(int16_t); }

        /**
         * @brief Dieses Objekt enthält einige wenige Parameter,
         * die nie abgerufen werden (Positionstabellen für Bauern
         * auf der ersten und letzten Reihe). Diese Methode prüft,
         * ob ein Parameter an einem bestimmten Index tot ist.
         */
        bool isParameterDead(size_t index) const;

        inline int16_t getMGPieceValue(int32_t piece) const { return MG_PIECE_VALUE[piece - 1]; }
        inline int16_t getEGPieceValue(int32_t piece) const { return EG_PIECE_VALUE[piece - 1]; }

        inline int16_t getEGWinningMaterialAdvantage() const { return (EG_PIECE_VALUE[3] - EG_PIECE_VALUE[2]) / 2 + EG_PIECE_VALUE[2]; }

        inline int16_t getMGPSQT(int32_t piece, int32_t square) const { return MG_PSQT[piece - 1][square]; }
        inline int16_t getEGPSQT(int32_t piece, int32_t square) const { return EG_PSQT[piece - 1][square]; }

        inline int16_t getMGTempoBonus() const { return MG_TEMPO_BONUS; }
        inline int16_t getEGTempoBonus() const { return EG_TEMPO_BONUS; }

        inline int16_t getMGConnectedPawnBonus(int32_t rank) const { return MG_CONNECTED_PAWN_BONUS[rank - 1]; }
        inline int16_t getEGConnectedPawnBonus(int32_t rank) const { return EG_CONNECTED_PAWN_BONUS[rank - 1]; }

        inline int16_t getMGDoubledPawnPenalty(int32_t file) const { return MG_DOUBLED_PAWN_PENALTY[file]; }
        inline int16_t getEGDoubledPawnPenalty(int32_t file) const { return EG_DOUBLED_PAWN_PENALTY[file]; }

        inline int16_t getMGIsolatedPawnPenalty(int32_t file) const { return MG_ISOLATED_PAWN_PENALTY[file]; }
        inline int16_t getEGIsolatedPawnPenalty(int32_t file) const { return EG_ISOLATED_PAWN_PENALTY[file]; }

        inline int16_t getMGBackwardPawnPenalty(int32_t rank) const { return MG_BACKWARD_PAWN_PENALTY[rank - 1]; }
        inline int16_t getEGBackwardPawnPenalty(int32_t rank) const { return EG_BACKWARD_PAWN_PENALTY[rank - 1]; }

        inline int16_t getMGPassedPawnBonus(int32_t rank) const { return MG_PASSED_PAWN_BONUS[rank - 1]; }
        inline int16_t getEGPassedPawnBonus(int32_t rank) const { return EG_PASSED_PAWN_BONUS[rank - 1]; }

        inline int16_t getMGStrongSquareBonus(int32_t rank, int32_t file) const { return rank == 7 ? 0 : MG_STRONG_SQUARE_BONUS[rank - 2][file]; }
        inline int16_t getEGStrongSquareBonus(int32_t rank, int32_t file) const { return rank == 7 ? 0 : EG_STRONG_SQUARE_BONUS[rank - 2][file]; }

        inline int16_t getMGSpaceBonusPerPiece() const { return MG_SPACE_BONUS_PER_PIECE; }

        inline int16_t getNumAttackerWeight(int32_t numAttackers) const { return numAttackers == 0 ? 0 : NUM_ATTACKER_WEIGHT[numAttackers - 1]; }

        inline int16_t getKnightAttackBonus() const { return KNIGHT_ATTACK_BONUS; }
        inline int16_t getBishopAttackBonus() const { return BISHOP_ATTACK_BONUS; }
        inline int16_t getRookAttackBonus() const { return ROOK_ATTACK_BONUS; }
        inline int16_t getQueenAttackBonus() const { return QUEEN_ATTACK_BONUS; }

        inline int16_t getMinorPieceDefenderBonus() const { return MINOR_PIECE_DEFENDER_BONUS; }

        inline int16_t getPawnShieldSizeBonus(int32_t size) const { return PAWN_SHIELD_SIZE_BONUS[size]; }
        inline int16_t getKingOpenFilePenalty(int32_t numFiles) const { return KING_OPEN_FILE_PENALTY[numFiles]; }
        inline int16_t getPawnStormBonus(int32_t rank) const { return PAWN_STORM_BONUS[rank - 1]; }

        inline int16_t getMGPieceMobilityBonus(int32_t piece) const { return MG_PIECE_MOBILITY_BONUS[piece - 2]; }
        inline int16_t getEGPieceMobilityBonus(int32_t piece) const { return EG_PIECE_MOBILITY_BONUS[piece - 2]; }

        inline int16_t getMGMinorPieceOnStrongSquareBonus() const { return MG_MINOR_PIECE_ON_STRONG_SQUARE_BONUS; }

        inline int16_t getMGBishopPairBonus() const { return MG_BISHOP_PAIR_BONUS; }
        inline int16_t getEGBishopPairBonus() const { return EG_BISHOP_PAIR_BONUS; }

        inline int16_t getMGRookOnOpenFileBonus() const { return MG_ROOK_ON_OPEN_FILE_BONUS; }
        inline int16_t getMGRookOnSemiOpenFileBonus() const { return MG_ROOK_ON_SEMI_OPEN_FILE_BONUS; }

        inline int16_t getEGRookBehindPassedPawnBonus() const { return EG_ROOK_BEHIND_PASSED_PAWN_BONUS; }

        inline int16_t getEGKingProximityPawnWeight() const { return EG_KING_PROXIMITY_PAWN_WEIGHT; }
        inline int16_t getEGKingProximityBackwardPawnWeight() const { return EG_KING_PROXIMITY_BACKWARD_PAWN_WEIGHT; }
        inline int16_t getEGKingProximityPassedPawnWeight() const { return EG_KING_PROXIMITY_PASSED_PAWN_WEIGHT; }
};

#endif