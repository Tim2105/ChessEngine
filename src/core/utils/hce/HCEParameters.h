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
            134, 391, 402, 616, 1190, // linear
            -5, -1, 13, -8, 3, // quadratisch
            // gekreuzt
            -5, // Springer * Bauer
            -11, -2, // Läufer * Bauer, Läufer * Springer
            -8, -7, -7, // Turm * Bauer, Turm * Springer, Turm * Läufer
            -12, -3, -5, -32 // Dame * Bauer, Dame * Springer, Dame * Läufer, Dame * Turm
        };

        /**
         * Faktoren für die Funktion zur Berechnung
         * des Materialungleichgewichts.
         */

        int16_t pieceImbalanceValues[10] = {
            // gekreuzt
            -11, // Springer * Bauer
            -7, -5, // Läufer * Bauer, Läufer * Springer
            -11, -10, -13, // Turm * Bauer, Turm * Springer, Turm * Läufer
            -13, -4, 1, -6 // Dame * Bauer, Dame * Springer, Dame * Läufer, Dame * Turm
        };

        /**
         * Positionstabellen für alle Figuren im Mittel- und Endspiel.
         */

        int16_t mgPSQTPawn[64] = {
            0, 0, 0, 0, 0, 0, 0, 0, // tote Parameterreihe
             0,  -7, -16, -11, -12,  -5,   3, -21,
            -6,   1, -12,  -4,   4, -27,  10, -17,
            -6,  -5,  11,  12,   0,  -7, -15,  -5,
            16,   7,  -5,  12,  -3,  -7, -10,   6,
            23,  13,  29,  22,  33,  16,  19,  18,
            51,  50,  38,  66,  44,  44,  19,  27,
            0, 0, 0, 0, 0, 0, 0, 0, // tote Parameterreihe
        };

        int16_t mgPSQTPacked[5][32] = {
            // Knight
            {
                    -65, -38, -32, -25,
                    -28, -23, -19,   4,
                    -24,  12,   7,   3,
                     -4,   0,  14,   7,
                    -12,   3,  18,  23,
                     -4,  18,  18,   6,
                    -22, -14, -14,   4,
                    -96, -64, -17,   6,
            },
            // Bishop
            {
                    -30, -29, -12, -24,
                    -22,   7,   9,   7,
                    -32,   1,   6,  13,
                    -14,   1,  -2,   5,
                    -27, -19,   1,  17,
                    -10, -13,  18,   5,
                    -42,   4,  -4, -24,
                    -46, -28, -35, -11,
            },
            // Rook
            {
                     -7,  -8,   3,   9,
                    -17, -16,  -2,   2,
                    -12, -21, -22, -11,
                    -20,  -7, -14,   3,
                     10,  21,  37,  20,
                     27,  41,  21,  25,
                     34,  31,  37,  34,
                     43,  31,  29,  27,
            },
            // Queen
            {
                    -66, -37, -25,  -2,
                    -41, -33, -13,  -4,
                     -5, -16,   3,  -9,
                    -15,  -1,  -2,  -3,
                    -11,   4, -10,  -2,
                     -1,   0,   7,   6,
                     23,  12,  17,  10,
                      4,  40,  36,   9,

            },
            // King
            {
                    -11,   4, -23, -45,
                     -7,  11, -22, -40,
                    -24, -22, -18, -40,
                    -34, -43, -51, -53,
                    -40, -43, -43, -74,
                    -30, -38, -35, -82,
                    -14, -24, -33, -57,
                    -19, -17, -29, -31,
            }
        };

        int16_t egPSQTPawn[64] = {
            0, 0, 0, 0, 0, 0, 0, 0, // tote Parameterreihe
            -26, -26, -28, -21, -19, -11, -10, -31,
            -31, -33, -36, -20, -18, -16, -24, -49,
            -25, -12, -37, -36, -37, -15,  -6, -26,
             -1,   5, -15, -16, -10,  -9,  -5, -31,
             37,  46,  34,   1,   7,  22,  31,  12,
             57,  93,  88,  67,  62,  69,  78,  51,
            0, 0, 0, 0, 0, 0, 0, 0, // tote Parameterreihe
        };

        int16_t egPSQTPacked[5][32] = {
            // Knight
            {
                    -41, -40, -34,  -4,
                    -41, -13, -19,   1,
                    -30, -27,  -3,  11,
                    -15,   6,   8,  16,
                    -17,  -2,   2,  23,
                    -30,  10,  21,  12,
                    -48, -16, -11,  -4,
                    -45, -24, -29,  -6,
            },
            // Bishop
            {
                    -23,  -6, -24, -26,
                    -17, -15,   3, -16,
                    -24,  -6,  -8,   0,
                    -20, -15,   8,  17,
                     -6,  15,  11,   5,
                      4,   5,  17,   4,
                     -2,  10,  -3,   9,
                     -1, -16, -11,   2,
            },
            // Rook
            {
                    -18,  -3, -16,  -6,
                      6, -21,  -3,   6,
                      3,  -7,  -3, -10,
                      0,   2,   9,  -7,
                     16,  14,   3,  -8,
                     20,  15,  -3,  10,
                     15,  22,  22,  17,
                     20,  19,  36,  26,
            },
            // Queen
            {
                    -12, -13,  -9,  -3,
                    -13, -12, -21,  -7,
                      0,  -4,   3,   2,
                     -7,   4,   5,  20,
                      9,  23,   5,   6,
                      6,  14,   2,  10,
                      7,  10,  10,  26,
                     -1,   7,  -3,   5,
            },
            // King
            {
                    -45, -40, -48, -46,
                    -52, -28, -25, -29,
                    -26, -14,  -7, -15,
                    -4,  -1,   8,   6,
                    3,  26,  31,  26,
                    35,  37,  47,  32,
                    32,  53,  50,  21,
                    -1,  29,  20,  15,
            }
        };

        /**
         * Bonus für das Zugrecht im Mittel- und Endspiel.
         */

        int16_t mgTempoBonus = 7;
        int16_t egTempoBonus = 12;

        /**
         * Bonus für angegriffene Figuren im Mittel- und Endspiel.
         * Angriffe auf kleinere Figuren, die von einem
         * Bauern gedeckt werden, werden nicht berücksichtigt.
         */

        int16_t mgAttackByMinorPieceBonus[5] = {
            1, 15, 30, 41, 42
        };

        int16_t mgAttackByRookBonus[5] = {
            -4, 27, 22, -5, 32
        };

        int16_t egAttackByMinorPieceBonus[5] = {
            25, 23, 23, 26, 33
        };

        int16_t egAttackByRookBonus[5] = {
            32, 21, 33, 8, 12
        };

        /**
         * Bewertungen für verschiedene Merkmale in
         * Bauernstrukturen im Mittel- und Endspiel.
         */

        int16_t mgConnectedPawnBonus[6] = {
            0, 4, -3, 5, 32, 70
        }; // pro Rang (2 - 7)
        int16_t egConnectedPawnBonus[6] = {
            2, 13, -1, 11, 22, 81
        }; // pro Rang (2 - 7)
        int16_t mgDoubledPawnPenalty = 2;
        int16_t egDoubledPawnPenalty = -15;
        int16_t mgIsolatedPawnPenalty = -9;
        int16_t egIsolatedPawnPenalty = -4;
        int16_t mgBackwardPawnPenalty = -8;
        int16_t egBackwardPawnPenalty = -7;
        int16_t mgPassedPawnBonus[6] = {
            -10, -19, -7, 22, 61, 85
        }; // pro Rang (2 - 7)
        int16_t egPassedPawnBonus[6] = {
            23, 25, 68, 85, 114, 186
        }; // pro Rang (2 - 7)
        int16_t mgCandidatePassedPawnBonus[5] = {
            -11, 5, -1, -6, 17
        }; // pro Rang (2 - 6)
        int16_t egCandidatePassedPawnBonus[5] = {
            8, 15, 42, 18, 64
        }; // pro Rang (2 - 6)
        int16_t mgConnectedPassedPawnBonus[6] = {
            0, 0, 0, 0, 0, 0
        }; // pro Rang (2 - 7)
        int16_t egConnectedPassedPawnBonus[6] = {
            0, 0, 0, 0, 0, 0
        }; // pro Rang (2 - 7)

        /**
         * Bewertungen für starke Felder im Mittelspiel.
         * Starke Felder sind Felder, die von einem eigenen Bauern
         * gedeckt sind und nie von einem gegnerischen Bauern
         * angegriffen werden können.
         */

        int16_t mgStrongSquareBonus = 8;

        /**
         * Ein Bonus für jedes sichere Feld
         * in der Mitte des Spielfeldes.
         */

        int16_t mgSpaceBonus = -3;

        /**
         * Ein Bonus für jedes Feld im Königsbereich des Gegners,
         * dass von einer eigenen Figur angegriffen wird.
         */

        int16_t numAttackerWeight[9] = {
            4, 11, 11, 20, 40, 59, 66, 94, 111
        };

        int16_t knightAttackBonus = 9;
        int16_t bishopAttackBonus = 5;
        int16_t rookAttackBonus = 12;
        int16_t queenAttackBonus = 63;

        /**
         * Bewertungen für verschiedene Merkmale in
         * Zusammenhang mit der Königssicherheit.
         */

        int16_t mgPawnShieldSizeBonus[3] = {
            -12, 4, 18
        };

        int16_t mgKingOpenFilePenalty[3] = {
            0, 1, -29
        };

        int16_t mgPawnStormPenalty[6] = {
            34, 11, -8, -4, -12, -13
        }; // pro Rang (2 - 7)

        /**
         * Bewertungen für die Mobilität der Figuren
         * im Mittel- und Endspiel.
         * (Springer, Läufer, Turm, Dame)
         */

        int16_t mgPieceMobilityBonus[4] = {
            5, // Knight
            11, // Bishop
            9, // Rook
            12, // Queen
        };
        int16_t egPieceMobilityBonus[4] = {
            1, // Knight
            4, // Bishop
            22, // Rook
            2, // Queen
        };

        int16_t mgPieceNoMobilityPenalty[4] = {
            -24, // Knight
            -50, // Bishop
            -72, // Rook
            -80, // Queen
        };
        int16_t egPieceNoMobilityPenalty[4] = {
            -9, // Knight
            -35, // Bishop
            -50, // Rook
            -48, // Queen
        };

        /**
         * Ein Bonus für jede Leichtfigur,
         * die auf einem starken Feld steht oder
         * im nächsten Zug erreichen kann.
         */

        int16_t mgKnightOnStrongSquareBonus = 41;
        int16_t mgBishopOnStrongSquareBonus = 19;

        /**
         * Eine Bestrafung für Läufer, die durch eigene
         * unbewegbare Bauern oder gegnerische, durch andere
         * Bauern gedeckte, Bauern blockiert werden.
         */

        int16_t mgBadBishopPenalty = -12;
        int16_t egBadBishopPenalty = -20;

        /**
         * Ein Bonus für jede offene oder halboffene
         * Linie, auf der sich ein Turm befindet.
         */

        int16_t mgRookOnOpenFileBonus = 18;
        int16_t mgRookOnSemiOpenFileBonus = 13;

        /**
         * Ein Bonus für jeden Freibauern, der von
         * einem Turm auf derselben Linie verteidigt wird.
         */

        int16_t egRookBehindPassedPawnBonus = 40;

        /**
         * @brief Bonus für jede Figur, die einen
         * gegnerischen Freibauern blockiert.
         */

        int16_t egBlockedEnemyPassedPawnBonus = 35;

        /**
         * Gewichtungsparameter für die Bewertung der Distanz
         * des Königs zu Bauern im Endspiel.
         */

        int16_t egKingProximityPawnWeight = 3;
        int16_t egKingProximityBackwardPawnWeight = 3;
        int16_t egKingProximityCandidatePassedPawnWeight = 3;
        int16_t egKingProximityPassedPawnWeight = 16;

        /**
         * @brief Bonus für Freibauern in einem Bauernendspiel,
         * die nicht mehr vom gegnerischen König erreicht werden können.
         */

        int16_t ruleOfTheSquareBonus = 665;

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

        inline int getMGConnectedPassedPawnBonus(int rank) const { return mgConnectedPassedPawnBonus[rank - 1]; }
        inline int getEGConnectedPassedPawnBonus(int rank) const { return egConnectedPassedPawnBonus[rank - 1]; }

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

        inline int getEGKingProximityPawnWeight() const { return egKingProximityPawnWeight; }
        inline int getEGKingProximityBackwardPawnWeight() const { return egKingProximityBackwardPawnWeight; }
        inline int getEGKingProximityCandidatePassedPawnWeight() const { return egKingProximityCandidatePassedPawnWeight; }
        inline int getEGKingProximityPassedPawnWeight() const { return egKingProximityPassedPawnWeight; }

        inline int getRuleOfTheSquareBonus() const { return ruleOfTheSquareBonus; }
};

extern HCEParameters HCE_PARAMS;

#endif