#ifndef HCE_PARAMETERS_H
#define HCE_PARAMETERS_H

#include <fstream>
#include <stdint.h>

class HCEParameters {

    public:
        /**
         * Faktoren für das Polynom zweiten Grades zur
         * Berechnung des Materialwertes einer Farbe.
         */

        int16_t pieceValues[20] = {
            176, 411, 421, 671, 1225, // linear
            -4, 3, 27, 8, 33, // quadratisch
            // gekreuzt
            -5, // Springer * Bauer
            -14, 5, // Läufer * Bauer, Läufer * Springer
            -6, 5, 8, // Turm * Bauer, Turm * Springer, Turm * Läufer
            0, -6, -6, -13 // Dame * Bauer, Dame * Springer, Dame * Läufer, Dame * Turm
        };

        /**
         * Faktoren für die Funktion zur Berechnung
         * des Materialungleichgewichts.
         */

        int16_t pieceImbalanceValues[10] = {
            // gekreuzt
            -13, // Springer * Bauer
            -5, -7, // Läufer * Bauer, Läufer * Springer
            -2, -25, -24, // Turm * Bauer, Turm * Springer, Turm * Läufer
            9, -8, 7, 20 // Dame * Bauer, Dame * Springer, Dame * Läufer, Dame * Turm
        };

        /**
         * Positionstabellen für alle Figuren im Mittel- und Endspiel.
         */

        int16_t mgPSQTPawn[64] = {
            0, 0, 0, 0, 0, 0, 0, 0, // tote Parameterreihe
              0,  -3, -12, -27, -17,  18,  22, -35,
              4,  -1,  -6, -20, -11, -23,   6,  -4,
              6,   6,   7,   4,   8,   6, -12, -10,
             26,  12, -20,  10,  14,  11, -15,  -8,
             40,  54,  37,  42,  53,  69,  51,  28,
            103,  97,  78,  75,  97,  93,  38,  65,
            0, 0, 0, 0, 0, 0, 0, 0, // tote Parameterreihe
        };

        int16_t mgPSQTPacked[5][32] = {
            // Knight
            {
                    -65, -29, -38, -30,
                    -12, -16, -25,  -4,
                    -27,  -3,   2,  12,
                     -8,   4,   5,  11,
                     -9,  -7,  33,  23,
                     -1,  16,  27,  31,
                    -19, -20,   3,  11,
                   -110, -47, -13,  11,
            },
            // Bishop
            {
                    -13, -35, -24,  -8,
                    -32,   6,  14,   0,
                     -5,   6,  12,  15,
                    -18,   2,  -7,  16,
                    -22, -24,   1,  17,
                      3,   4,  21,  -1,
                    -55, -14, -17, -24,
                    -39, -20, -41, -27,
            },
            // Rook
            {
                    -10, -10,   5,   8,
                    -45, -35,  -9,  -4,
                    -26,  -4, -21,   2,
                    -22,  -6,  -8,   8,
                    -12,   3,  26,  34,
                     21,  38,  42,  42,
                     31,  25,  33,  42,
                     36,  47,  32,  33,
            },
            // Queen
            {
                    -49, -42, -35,  -5,
                    -37, -36,  -9,  -9,
                     -6,  -1,  -8, -17,
                    -12,  -5,  -4,   3,
                     -5,   9,   0,  -1,
                      4,  11,  23,  21,
                     -8,  -9,  16,  16,
                     -6,  28,  23,  19,

            },
            // King
            {
                    -23,   7, -62, -45,
                      5,  14, -15, -47,
                    -53, -20, -25, -29,
                    -57, -54, -48, -51,
                    -62, -39, -54, -75,
                    -37, -34, -29, -78,
                    -11, -21, -38, -59,
                    -26,   6,  -9, -23,
            }
        };

        int16_t egPSQTPawn[64] = {
            0, 0, 0, 0, 0, 0, 0, 0, // tote Parameterreihe
              5,  21,  -3,  -9, -23, -26, -28, -37,
            -22,  -8, -27, -19, -31, -16, -35, -49,
             -8,   7, -44, -36, -40, -20, -21, -33,
             13,  10, -12, -21, -16,  -9, -10, -36,
             55,  67,  41,  16,  19,  34,  57,  16,
             77, 139, 108,  94,  96, 115, 125,  56,
            0, 0, 0, 0, 0, 0, 0, 0, // tote Parameterreihe
        };

        int16_t egPSQTPacked[5][32] = {
            // Knight
            {
                    -45, -40, -45, -15,
                    -42, -23, -27,   3,
                    -40, -21,   4,  15,
                    -17,  -2,  19,  31,
                     -6,   4,  26,  31,
                    -26,  14,  25,  27,
                    -18,   7,   2,   2,
                    -46,   6, -19,   2,
            },
            // Bishop
            {
                    -13, -31, -11,  -6,
                    -18, -16,   1,   0,
                    -14,  -2,   8,  14,
                    -16,  -5,   9,   7,
                     -7,  22, -12,  -1,
                     -5,   4,  22,  -2,
                     14,   9,   0,  28,
                     23,  11,   5,   8,
            },
            // Rook
            {
                    -5,   3, -16, -12,
                    -2, -13, -19,   0,
                    -7,  11,   7, -15,
                    13,  29,  20,  11,
                    36,  23,  17,   7,
                    31,  30,  22,  15,
                    21,  31,  15,  15,
                     8,  35,  43,  39,
            },
            // Queen
            {
                    -31, -30, -38, -33,
                    -26, -10, -29,  -3,
                     -4,  -8,   4,   7,
                     -8, -10,  10,  34,
                      9,  24,  30,  24,
                      2,  32,  20,  20,
                     -1,  10,  16,  30,
                    -16,  -6, -11,  22,
            },
            // King
            {
                    -22, -62, -34, -48,
                    -38, -32, -23, -33,
                    -40, -26, -24, -30,
                    -22, -12,  -4,  -5,
                      5,  27,  28,  33,
                     47,  70,  57,  35,
                     37,  67,  44,  32,
                    -25,  51,  41,  32,
            }
        };

        /**
         * Bonus für das Zugrecht im Mittel- und Endspiel.
         */

        int16_t mgTempoBonus = 12;
        int16_t egTempoBonus = 13;

        /**
         * Bonus für angegriffene Figuren im Mittel- und Endspiel.
         * Angriffe auf kleinere Figuren, die von einem
         * Bauern gedeckt werden, werden nicht berücksichtigt.
         */

        int16_t mgAttackByMinorPieceBonus[5] = {
            2, 14, 35, 53, 50
        };

        int16_t mgAttackByRookBonus[5] = {
            6, 32, 34, -2, 45
        };

        int16_t egAttackByMinorPieceBonus[5] = {
            31, 28, 32, 8, 24
        };

        int16_t egAttackByRookBonus[5] = {
            42, 20, 41, 7, 19
        };

        /**
         * Bewertungen für verschiedene Merkmale in
         * Bauernstrukturen im Mittel- und Endspiel.
         */

        int16_t mgConnectedPawnBonus[6] = {
            0, 8, 9, 11, 45, 84
        }; // pro Rang (2 - 7)
        int16_t egConnectedPawnBonus[6] = {
            -5, 11, 0, 20, 27, 103
        }; // pro Rang (2 - 7)
        int16_t mgDoubledPawnPenalty = -7;
        int16_t egDoubledPawnPenalty = -23;
        int16_t mgIsolatedPawnPenalty = -17;
        int16_t egIsolatedPawnPenalty = -2;
        int16_t mgBackwardPawnPenalty = -21;
        int16_t egBackwardPawnPenalty = -16;
        int16_t mgPassedPawnBonus[6] = {
            -17, -21, -15, 32, 92, 149
        }; // pro Rang (2 - 7)
        int16_t egPassedPawnBonus[6] = {
            18, 34, 90, 137, 170, 257
        }; // pro Rang (2 - 7)
        int16_t mgCandidatePassedPawnBonus[5] = {
            -4, -8, -3, 9, 26
        }; // pro Rang (2 - 6)
        int16_t egCandidatePassedPawnBonus[5] = {
            -3, 20, 49, 39, 74
        }; // pro Rang (2 - 6)
        int16_t mgConnectedPassedPawnBonus[6] = {
            -4, -9, -18, -9, 1, 8
        }; // pro Rang (2 - 7)
        int16_t egConnectedPassedPawnBonus[6] = {
            27, 10, -25, -23, 0, 11
        }; // pro Rang (2 - 7)

        /**
         * Bewertungen für starke Felder im Mittelspiel.
         * Starke Felder sind Felder, die von einem eigenen Bauern
         * gedeckt sind und nie von einem gegnerischen Bauern
         * angegriffen werden können.
         */

        int16_t mgStrongSquareBonus = -3;

        /**
         * Ein Bonus für jedes Feld im Königsbereich des Gegners,
         * dass von einer eigenen Figur angegriffen wird.
         */

        int16_t numAttackerWeight[4] = {
            17, 39, 84, 100
        };

        int16_t knightAttackBonus = 11;
        int16_t bishopAttackBonus = 9;
        int16_t rookAttackBonus = 10;
        int16_t queenAttackBonus = 101;

        /**
         * Bewertungen für verschiedene Merkmale in
         * Zusammenhang mit der Königssicherheit.
         */

        int16_t mgPawnShieldSizeBonus[3] = {
            -16, 5, 23
        };

        int16_t mgKingOpenFilePenalty[3] = {
            4, -9, -44
        };

        int16_t mgPawnStormPenalty[6] = {
            75, 30, -7, 0, -1, -10
        }; // pro Rang (2 - 7)

        /**
         * Bewertungen für die Mobilität der Figuren
         * im Mittel- und Endspiel.
         * (Springer, Läufer, Turm, Dame)
         */

        int16_t mgPieceMobilityBonus[4] = {
            13, // Knight
            18, // Bishop
            1, // Rook
            18, // Queen
        };
        int16_t egPieceMobilityBonus[4] = {
            39, // Knight
            35, // Bishop
            64, // Rook
            28, // Queen
        };

        int16_t mgPieceNoMobilityPenalty[4] = {
            -30, // Knight
            -48, // Bishop
            -73, // Rook
            -85, // Queen
        };
        int16_t egPieceNoMobilityPenalty[4] = {
            -9, // Knight
            -42, // Bishop
            -50, // Rook
            -49, // Queen
        };

        /**
         * Ein Bonus für jede Leichtfigur,
         * die auf einem starken Feld steht oder
         * im nächsten Zug erreichen kann.
         */

        int16_t mgKnightOnStrongSquareBonus = 38;
        int16_t mgBishopOnStrongSquareBonus = 39;

        /**
         * Eine Bestrafung für Läufer, die durch eigene
         * unbewegbare Bauern oder gegnerische, durch andere
         * Bauern gedeckte, Bauern blockiert werden.
         */

        int16_t mgBadBishopPenalty = -13;
        int16_t egBadBishopPenalty = -17;

        /**
         * Ein Bonus für jede offene oder halboffene
         * Linie, auf der sich ein Turm befindet.
         */

        int16_t mgRookOnOpenFileBonus = 38;
        int16_t mgRookOnSemiOpenFileBonus = 18;

        /**
         * Ein Bonus für jeden Freibauern, der von
         * einem Turm auf derselben Linie verteidigt wird.
         */

        int16_t egRookBehindPassedPawnBonus = 71;

        /**
         * @brief Bonus für jede Figur, die einen
         * gegnerischen Freibauern blockiert.
         */

        int16_t egBlockedEnemyPassedPawnBonus = 69;

        /**
         * Gewichtungsparameter für die Bewertung der Distanz
         * des Königs zu Bauern im Endspiel.
         */

        int16_t egKingProximityPawnWeight = 6;
        int16_t egKingProximityBackwardPawnWeight = 8;
        int16_t egKingProximityCandidatePassedPawnWeight = 4;
        int16_t egKingProximityPassedPawnWeight = 30;

        /**
         * @brief Bonus für Freibauern in einem Bauernendspiel,
         * die nicht mehr vom gegnerischen König erreicht werden können.
         */

        int16_t ruleOfTheSquareBonus = 620;

        /**
         * @brief Bestrafung für ein Endspiel mit Läufern
         * unterschiedlicher Farbe. Die Bestrafung wird
         * der ansonsten führenden Seite angerechnet und
         * kann das Vorzeichen der Bewertung nicht ändern.
         */
        int16_t oppositeColorBishopsPenalty = -200;

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

        inline int getMGConnectedPassedPawnBonus(int rank) const { return mgConnectedPassedPawnBonus[rank - 1]; }
        inline int getEGConnectedPassedPawnBonus(int rank) const { return egConnectedPassedPawnBonus[rank - 1]; }

        inline int getMGStrongSquareBonus() const { return mgStrongSquareBonus; }

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

        inline int getEGKingProximityPawnWeight() const { return egKingProximityPawnWeight; }
        inline int getEGKingProximityBackwardPawnWeight() const { return egKingProximityBackwardPawnWeight; }
        inline int getEGKingProximityCandidatePassedPawnWeight() const { return egKingProximityCandidatePassedPawnWeight; }
        inline int getEGKingProximityPassedPawnWeight() const { return egKingProximityPassedPawnWeight; }

        inline int getRuleOfTheSquareBonus() const { return ruleOfTheSquareBonus; }

        inline int getOppositeColorBishopsPenalty() const { return oppositeColorBishopsPenalty; }
};

extern HCEParameters HCE_PARAMS;

#endif