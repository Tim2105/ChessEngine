#ifndef HCE_PARAMETERS_H
#define HCE_PARAMETERS_H

#include <fstream>
#include <stdint.h>

class HCEParameters {

    private:
        /**
         * Faktoren für das Polynom zweiten Grades zur
         * Berechnung des Materialwertes einer Farbe.
         */

        int16_t pieceValues[20] = {
            132, 392, 401, 615, 1180, // linear
            -2, -1, 13, -7, -5, // quadratisch
            // gekreuzt
            -2, // Springer * Bauer
            -8, -3, // Läufer * Bauer, Läufer * Springer
            -7, -5, -6, // Turm * Bauer, Turm * Springer, Turm * Läufer
            -19, -10, -1, -35 // Dame * Bauer, Dame * Springer, Dame * Läufer, Dame * Turm
        };

        /**
         * Faktoren für die Funktion zur Berechnung
         * des Materialungleichgewichts.
         */

        int16_t pieceImbalanceValues[10] = {
            // gekreuzt
            -8, // Springer * Bauer
            -6, -5, // Läufer * Bauer, Läufer * Springer
            -5, -4, -13, // Turm * Bauer, Turm * Springer, Turm * Läufer
            -17, 0, -2, -11 // Dame * Bauer, Dame * Springer, Dame * Läufer, Dame * Turm
        };

        /**
         * Positionstabellen für alle Figuren im Mittel- und Endspiel.
         */

        int16_t mgPSQTPawn[64] = {
            0, 0, 0, 0, 0, 0, 0, 0, // tote Parameterreihe
             -3, -10,  -9, -12,  -7, -21,   4, -21,
            -10,  -5, -13,  -1,  -2, -23,   4, -14,
              4, -18,  10,  11,  11, -12,  -5, -14,
             28,   9,  -7,  14,   4,   0,   0,  10,
             22,   6,  22,  28,  36,  14,  18,  37,
             48,  36,  35,  51,  46,  32,  19,  31,
            0, 0, 0, 0, 0, 0, 0, 0, // tote Parameterreihe
        };

        int16_t mgPSQTPacked[5][32] = {
            // Knight
            {
                    -63, -37, -28, -33,
                    -27, -17, -19,  -4,
                    -16,   9,  10,   3,
                     -8,  -2,   8,   2,
                     -4,   4,  17,  26,
                     -5,  18,  13,  -8,
                    -10, -12, -15,   4,
                    -96, -65,  -9,   5,
            },
            // Bishop
            {
                    -37, -30,  -7, -25,
                    -42,  14,   7,   7,
                    -25,  -5,   8,  14,
                    -18,  -5,   0,   1,
                    -32, -15,   4,  13,
                    -15, -13,   8,  -1,
                    -36,   3,  -4, -26,
                    -59, -28, -25,  -9,
            },
            // Rook
            {
                    -11, -13,   6,  10,
                    -22, -12,   9,  -7,
                     -5, -13, -22,   8,
                    -23, -12, -11,  -3,
                      5,   9,  41,  20,
                     26,  25,  19,  20,
                     47,  26,  41,  34,
                     49,  29,  27,  22,
            },
            // Queen
            {
                    -58, -36, -24,  -3,
                    -43, -51,  -6,   0,
                    -18, -15,  13,  -6,
                    -20,  -5,  -4,  -6,
                    -15,   5, -18, -15,
                     -3,  -6,  13,   3,
                     26,  19,  22,   5,
                      8,  42,  37,  11,

            },
            // King
            {
                    -17,  15, -24, -44,
                    -13,   9, -20, -40,
                    -25, -22, -29, -37,
                    -32, -54, -51, -58,
                    -43, -51, -41, -72,
                    -35, -39, -47, -77,
                    -18, -32, -35, -52,
                    -14, -21, -36, -34,
            }
        };

        int16_t egPSQTPawn[64] = {
            0, 0, 0, 0, 0, 0, 0, 0, // tote Parameterreihe
            -24, -31, -35, -33, -16, -10,  -5, -42,
            -44, -45, -32, -23, -21, -22, -21, -52,
            -25, -18, -22, -42, -25, -20,  -3, -27,
             -4,  17, -17,  -8,  -4,  -6,   3, -29,
             38,  35,  34,   7,  20,  23,  42,  20,
             60,  82,  78,  53,  58,  66,  68,  42,
            0, 0, 0, 0, 0, 0, 0, 0, // tote Parameterreihe
        };

        int16_t egPSQTPacked[5][32] = {
            // Knight
            {
                    -32, -46, -30, -17,
                    -41,  -9, -13,  -8,
                    -31, -21,   5,  13,
                    -16,  -2,   6,  16,
                    -19,   1,  10,  16,
                    -29,  12,  24,   5,
                    -44, -19, -11,   1,
                    -40, -28, -13,  -9,
            },
            // Bishop
            {
                    -31, -11, -22, -30,
                    -23, -16,  -9, -17,
                    -19, -14,  -5,  -7,
                    -11, -14,   6,  16,
                    -11,  23,  18,   6,
                      3,   4,  20,   2,
                    -10,   8,  -8,   7,
                     -9, -12,  -3,   5,
            },
            // Rook
            {
                    -22,  -1, -10,   3,
                      5, -17,   3,  -1,
                     -2,   5,  -1,  -6,
                     -8,  -2,  14, -13,
                      9,   8,  -3, -13,
                     17,  10,  -6,  14,
                     24,  26,  19,  15,
                     23,  15,  27,  15,
            },
            // Queen
            {
                     -7, -15,  -9,  -2,
                    -11, -13, -19,  -4,
                     -3, -11,   7,   3,
                     -6,   1,   8,  20,
                      5,  20,   3,   8,
                      6,  10,  13,  13,
                      5,  10,  15,  16,
                     -5,  10,  -1,   6,
            },
            // King
            {
                    -51, -39, -58, -47,
                    -50, -24, -28, -29,
                    -31, -13, -11,  -6,
                      0, -10,  12,   6,
                      3,  14,  36,  38,
                     30,  32,  44,  39,
                     33,  42,  47,  27,
                      9,  23,   8,  16,
            }
        };

        /**
         * Bonus für das Zugrecht im Mittel- und Endspiel.
         */

        int16_t mgTempoBonus = 8;
        int16_t egTempoBonus = 11;

        /**
         * Bonus für angegriffene Figuren im Mittel- und Endspiel.
         * Angriffe auf kleinere Figuren, die von einem
         * Bauern gedeckt werden, werden nicht berücksichtigt.
         */

        int16_t mgAttackByMinorPieceBonus[5] = {
            6, 14, 20, 36, 39
        };

        int16_t mgAttackByRookBonus[5] = {
            -7, 22, 22, -3, 28
        };

        int16_t egAttackByMinorPieceBonus[5] = {
            26, 19, 18, 30, 37
        };

        int16_t egAttackByRookBonus[5] = {
            31, 24, 27, 10, 17
        };

        /**
         * Bewertungen für verschiedene Merkmale in
         * Bauernstrukturen im Mittel- und Endspiel.
         */

        int16_t mgConnectedPawnBonus[6] = {
            -7, 0, -4, 7, 52, 68
        }; // pro Rang (2 - 7)
        int16_t egConnectedPawnBonus[6] = {
            5, 8, -4, 12, 41, 79
        }; // pro Rang (2 - 7)
        int16_t mgDoubledPawnPenalty = -10;
        int16_t egDoubledPawnPenalty = -18;
        int16_t mgIsolatedPawnPenalty = -10;
        int16_t egIsolatedPawnPenalty = -1;
        int16_t mgBackwardPawnPenalty = -5;
        int16_t egBackwardPawnPenalty = -8;
        int16_t mgPassedPawnBonus[6] = {
            -24, -18, -5, 12, 60, 76
        }; // pro Rang (2 - 7)
        int16_t egPassedPawnBonus[6] = {
            18, 14, 71, 87, 121, 171
        }; // pro Rang (2 - 7)
        int16_t mgCandidatePassedPawnBonus[5] = {
            -4, 0, -5, 4, 19
        }; // pro Rang (2 - 6)
        int16_t egCandidatePassedPawnBonus[5] = {
            11, 19, 32, 30, 65
        }; // pro Rang (2 - 6)

        /**
         * Bewertungen für starke Felder im Mittelspiel.
         * Starke Felder sind Felder, die von einem eigenen Bauern
         * gedeckt sind und nie von einem gegnerischen Bauern
         * angegriffen werden können.
         */

        int16_t mgStrongSquareBonus = 4;

        /**
         * Ein Bonus für jedes sichere Feld
         * in der Mitte des Spielfeldes.
         */

        int16_t mgSpaceBonus = -1;

        /**
         * Ein Bonus für jedes Feld im Königsbereich des Gegners,
         * dass von einer eigenen Figur angegriffen wird.
         */

        int16_t numAttackerWeight[9] = {
            5, 14, 14, 22, 39, 59, 67, 88, 110
        };

        int16_t knightAttackBonus = 13;
        int16_t bishopAttackBonus = 3;
        int16_t rookAttackBonus = 20;
        int16_t queenAttackBonus = 67;

        /**
         * Bewertungen für verschiedene Merkmale in
         * Zusammenhang mit der Königssicherheit.
         */

        int16_t mgPawnShieldSizeBonus[3] = {
            -2, 1, 15
        };

        int16_t mgKingOpenFilePenalty[3] = {
            6, -2, -19
        };

        int16_t mgPawnStormPenalty[6] = {
            23, 3, -14, -3, -13, -18
        }; // pro Rang (2 - 7)

        /**
         * Bewertungen für die Mobilität der Figuren
         * im Mittel- und Endspiel.
         * (Springer, Läufer, Turm, Dame)
         */

        int16_t mgPieceMobilityBonus[4] = {
            0, // Knight
            10, // Bishop
            7, // Rook
            10, // Queen
        };
        int16_t egPieceMobilityBonus[4] = {
            5, // Knight
            2, // Bishop
            19, // Rook
            -6, // Queen
        };

        int16_t mgPieceNoMobilityPenalty[4] = {
            -23, // Knight
            -54, // Bishop
            -77, // Rook
            -75, // Queen
        };
        int16_t egPieceNoMobilityPenalty[4] = {
            -5, // Knight
            -31, // Bishop
            -44, // Rook
            -48, // Queen
        };

        /**
         * Ein Bonus für jede Leichtfigur,
         * die auf einem starken Feld steht oder
         * im nächsten Zug erreichen kann.
         */

        int16_t mgKnightOnStrongSquareBonus = 27;
        int16_t mgBishopOnStrongSquareBonus = 14;

        /**
         * Eine Bestrafung für Läufer, die durch eigene
         * unbewegbare Bauern oder gegnerische, durch andere
         * Bauern gedeckte, Bauern blockiert werden.
         */

        int16_t mgBadBishopPenalty = -16;
        int16_t egBadBishopPenalty = -26;

        /**
         * Ein Bonus für jede offene oder halboffene
         * Linie, auf der sich ein Turm befindet.
         */

        int16_t mgRookOnOpenFileBonus = 17;
        int16_t mgRookOnSemiOpenFileBonus = 10;

        /**
         * Ein Bonus für jeden Freibauern, der von
         * einem Turm auf derselben Linie verteidigt wird.
         */

        int16_t egRookBehindPassedPawnBonus = 34;

        /**
         * @brief Bonus für jede Figur, die einen
         * gegnerischen Freibauern blockiert.
         */

        int16_t egBlockedEnemyPassedPawnBonus = 27;

        /**
         * @brief Bestrafung für jede eigene Leichtfigur,
         * die einen eigenen Freibauern blockiert.
         */

        int16_t egRookBlocksOwnPassedPawnPenalty = -5;

        /**
         * Gewichtungsparameter für die Bewertung der Distanz
         * des Königs zu Bauern im Endspiel.
         */

        int16_t egKingProximityPawnWeight = 4;
        int16_t egKingProximityBackwardPawnWeight = 7;
        int16_t egKingProximityPassedPawnWeight = 13;

        /**
         * @brief Bonus für Freibauern in einem Bauernendspiel,
         * die nicht mehr vom gegnerischen König erreicht werden können.
         */

        int16_t ruleOfTheSquareBonus = 679;

        /**
         * Hier werden die entpackten Positionstabellen
         * für die Mittel- und Endspielbewertung gespeichert.
         */

        int16_t mgPSQT[6][64];
        int16_t egPSQT[6][64];


    public:
        HCEParameters();
        HCEParameters(std::istream& stream);

        ~HCEParameters();

        void unpackPSQT();

        void loadParameters(std::istream& stream);
        void saveParameters(std::ostream& stream) const;

        /**
         * @brief Gibt die Parameter in lesbarer Form auf
         * einem Ausgabe-Stream aus.
         */
        void displayParameters(std::ostream& stream) const;

        int16_t& operator[](size_t index);
        int16_t operator[](size_t index) const;
        size_t indexOf(int16_t* ptr) const;

        static constexpr size_t size() { return (sizeof(HCEParameters) - sizeof(mgPSQT) - sizeof(egPSQT)) / sizeof(int16_t); }

        /**
         * @brief Dieses Objekt enthält einige wenige Parameter,
         * die nie abgerufen werden (Positionstabellen für Bauern
         * auf der ersten und letzten Reihe). Diese Methode prüft,
         * ob ein Parameter an einem bestimmten Index tot ist.
         */
        bool isParameterDead(size_t index) const;

        /**
         * @brief Prüft, ob ein Parameter optimiert werden kann.
         */
        bool isOptimizable(size_t index) const;

        inline int getLinearPieceValue(int piece) const { return pieceValues[piece - 1]; }
        inline int getQuadraticPieceValue(int piece) const { return pieceValues[piece + 4]; }

        inline int getCrossedPieceValue(int piece1, int piece2) const {
            int minPiece = std::min(piece1, piece2) - 1;
            int maxPiece = std::max(piece1, piece2) - 2;

            return pieceValues[10 + maxPiece * (maxPiece + 1) / 2 + minPiece];
        }

        inline int getPieceImbalanceValue(int piece1, int piece2) const {
            int minPiece = std::min(piece1, piece2) - 1;
            int maxPiece = std::max(piece1, piece2) - 2;

            return pieceImbalanceValues[maxPiece * (maxPiece + 1) / 2 + minPiece];
        }

        inline int getEGWinningMaterialAdvantage() const { return (pieceValues[3] - pieceValues[2]) / 2 + pieceValues[2]; }

        inline int getMGPSQT(int piece, int square) const { return mgPSQT[piece - 1][square]; }
        inline int getEGPSQT(int piece, int square) const { return egPSQT[piece - 1][square]; }

        inline int getMGTempoBonus() const { return mgTempoBonus; }
        inline int getEGTempoBonus() const { return egTempoBonus; }

        inline int getMGAttackByMinorPieceBonus(int piece) const { return mgAttackByMinorPieceBonus[piece - 1]; }
        inline int getMGAttackByRookBonus(int piece) const { return mgAttackByRookBonus[piece - 1]; }
        inline int getEGAttackByMinorPieceBonus(int piece) const { return egAttackByMinorPieceBonus[piece - 1]; }
        inline int getEGAttackByRookBonus(int piece) const { return egAttackByRookBonus[piece - 1]; }

        inline int getMGConnectedPawnBonus(int rank) const { return mgConnectedPawnBonus[rank - 1]; }
        inline int getEGConnectedPawnBonus(int rank) const { return egConnectedPawnBonus[rank - 1]; }

        inline int getMGDoubledPawnPenalty() const { return mgDoubledPawnPenalty; }
        inline int getEGDoubledPawnPenalty() const { return egDoubledPawnPenalty; }

        inline int getMGIsolatedPawnPenalty() const { return mgIsolatedPawnPenalty; }
        inline int getEGIsolatedPawnPenalty() const { return egIsolatedPawnPenalty; }

        inline int getMGBackwardPawnPenalty() const { return mgBackwardPawnPenalty; }
        inline int getEGBackwardPawnPenalty() const { return egBackwardPawnPenalty; }

        inline int getMGPassedPawnBonus(int rank) const { return mgPassedPawnBonus[rank - 1]; }
        inline int getEGPassedPawnBonus(int rank) const { return egPassedPawnBonus[rank - 1]; }

        inline int getMGCandidatePassedPawnBonus(int rank) const { return mgCandidatePassedPawnBonus[rank - 1]; }
        inline int getEGCandidatePassedPawnBonus(int rank) const { return egCandidatePassedPawnBonus[rank - 1]; }

        inline int getMGStrongSquareBonus() const { return mgStrongSquareBonus; }

        inline int getMGSpaceBonus() const { return mgSpaceBonus; }

        inline int getNumAttackerWeight(int numAttackers) const { return numAttackers == 0 ? 0 : numAttackerWeight[numAttackers - 1]; }

        inline int getKnightAttackBonus() const { return knightAttackBonus; }
        inline int getBishopAttackBonus() const { return bishopAttackBonus; }
        inline int getRookAttackBonus() const { return rookAttackBonus; }
        inline int getQueenAttackBonus() const { return queenAttackBonus; }

        inline int getMGPawnShieldSizeBonus(int size) const { return size == 0 ? 0 : mgPawnShieldSizeBonus[size - 1]; }
        inline int getMGKingOpenFilePenalty(int numFiles) const { return numFiles == 0 ? 0 : mgKingOpenFilePenalty[numFiles - 1]; }
        inline int getMGPawnStormPenalty(int rank) const { return mgPawnStormPenalty[rank - 1]; }

        inline int getMGPieceMobilityBonus(int piece) const { return mgPieceMobilityBonus[piece - 2]; }
        inline int getEGPieceMobilityBonus(int piece) const { return egPieceMobilityBonus[piece - 2]; }

        inline int getMGPieceNoMobilityPenalty(int piece) const { return mgPieceNoMobilityPenalty[piece - 2]; }
        inline int getEGPieceNoMobilityPenalty(int piece) const { return egPieceNoMobilityPenalty[piece - 2]; }

        inline int getMGKnightOnStrongSquareBonus() const { return mgKnightOnStrongSquareBonus; }
        inline int getMGBishopOnStrongSquareBonus() const { return mgBishopOnStrongSquareBonus; }

        inline int getMGBadBishopPenalty() const { return mgBadBishopPenalty; }
        inline int getEGBadBishopPenalty() const { return egBadBishopPenalty; }

        inline int getMGRookOnOpenFileBonus() const { return mgRookOnOpenFileBonus; }
        inline int getMGRookOnSemiOpenFileBonus() const { return mgRookOnSemiOpenFileBonus; }

        inline int getEGRookBehindPassedPawnBonus() const { return egRookBehindPassedPawnBonus; }
        inline int getEGBlockedEnemyPassedPawnBonus() const { return egBlockedEnemyPassedPawnBonus; }
        inline int getEGRooksBlocksOwnPassedPawnPenalty() const { return egRookBlocksOwnPassedPawnPenalty; }

        inline int getEGKingProximityPawnWeight() const { return egKingProximityPawnWeight; }
        inline int getEGKingProximityBackwardPawnWeight() const { return egKingProximityBackwardPawnWeight; }
        inline int getEGKingProximityPassedPawnWeight() const { return egKingProximityPassedPawnWeight; }

        inline int getRuleOfTheSquareBonus() const { return ruleOfTheSquareBonus; }
};

extern HCEParameters HCE_PARAMS;

#endif