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
            105, 400, 400, 615, 1200, // linear
            -2, -3, 10, -5, 15, // quadratisch
            // gekreuzt
            1, // Springer * Bauer
            0, 0, // Läufer * Bauer, Läufer * Springer
            0, 0, 0, // Turm * Bauer, Turm * Springer, Turm * Läufer
            0, 0, 0, 0 // Dame * Bauer, Dame * Springer, Dame * Läufer, Dame * Turm
        };

        /**
         * Faktoren für die Funktion zur Berechnung
         * des Materialungleichgewichts.
         */

        int16_t pieceImbalanceValues[10] = {
            // gekreuzt
            1, // Springer * Bauer
            0, 0, // Läufer * Bauer, Läufer * Springer
            0, 0, 0, // Turm * Bauer, Turm * Springer, Turm * Läufer
            0, 0, 0, 0 // Dame * Bauer, Dame * Springer, Dame * Läufer, Dame * Turm
        };

        /**
         * Positionstabellen für alle Figuren im Mittel- und Endspiel.
         */

        int16_t mgPSQTPawn[64] = {
            0, 0, 0, 0, 0, 0, 0, 0, // tote Parameterreihe
            1, 1, -14, -16, -16, -13, 0, 1,
            -3, -1, -16, -14, -13, -15, -1, -3,
            5, 1, 14, 19, 19, 8, 0, 2,
            9, 0, 2, 28, 28, 2, 0, 6,
            7, 4, 5, 25, 25, 6, 5, 5,
            12, 7, 9, 28, 28, 9, 10, 12,
            0, 0, 0, 0, 0, 0, 0, 0, // tote Parameterreihe
        };

        int16_t mgPSQTPacked[5][32] = {
            // Knight
            {
                    -67, -18, -23, -26,
                    -37, -9, -11, -9,
                    -28, -1, 2, -6,
                    -9, 7, 13, 18,
                    4, 12, 18, 21,
                    2, 9, 2, -10,
                    -8, -19, -13, -13,
                    -92, -55, -18, -14,
            },
            // Bishop
            {
                    -41, -22, -10, -3,
                    -34, 16, -1, 4,
                    -7, -12, 4, 15,
                    -15, 8, 14, 4,
                    -27, -12, -7, -2,
                    -5, -21, -18, -4,
                    -23, -9, -12, -26,
                    -52, -23, -19, -12,
            },
            // Rook
            {
                    -12, -17, 3, 11,
                    -15, -8, -1, 4,
                    -10, -9, -8, -3,
                    -8, -9, -5, 6,
                    4, 6, 16, 23,
                    27, 18, 14, 10,
                    51, 39, 35, 46,
                    48, 34, 34, 34,
            },
            // Queen
            {
                    -45, -39, -8, 12,
                    -27, -35, 3, 0,
                    -8, -5, -5, -9,
                    -24, -19, -11, -34,
                    -6, -21, -25, -28,
                    10, 11, 6, -8,
                    40, 37, 21, 8,
                    35, 47, 43, 25,

            },
            // King
            {
                    -10, 17, -1, -26,
                    -18, 7, -24, -34,
                    -25, -27, -29, -36,
                    -39, -48, -57, -65,
                    -46, -53, -64, -72,
                    -35, -50, -65, -78,
                    -29, -37, -53, -60,
                    -21, -29, -40, -47,
            }
        };

        int16_t egPSQTPawn[64] = {
            0, 0, 0, 0, 0, 0, 0, 0, // tote Parameterreihe
            -41, -24, -31, -39, -40, -18, -10, -45,
            -52, -28, -43, -39, -40, -33, -24, -45,
            -30, -9, -14, -17, -17, -6, 5, -24,
            -19, 1, 5, 8, 10, -5, 9, -18,
            8, 23, 22, 19, 18, 18, 20, 5,
            23, 45, 39, 34, 31, 35, 41, 19,
            0, 0, 0, 0, 0, 0, 0, 0, // tote Parameterreihe
        };

        int16_t egPSQTPacked[5][32] = {
            // Knight
            {
                    -32, -25, -9, -11,
                    -22, -11, -10, -1,
                    -11, -10, -1, 5,
                    -9, 2, 8, 8,
                    -9, 4, 5, 11,
                    -20, -9, -4, 0,
                    -26, -12, -12, -4,
                    -49, -31, -13, -15,
            },
            // Bishop
            {
                    -8, -2, -8, -4,
                    -13, -7, -4, 2,
                    -7, -3, 1, 6,
                    -4, -1, 5, 3,
                    1, 1, 5, 7,
                    2, 0, 3, -1,
                    -7, -2, -6, -1,
                    -12, -8, -4, -3,
            },
            // Rook
            {
                    -10, 2, -6, -2,
                    -1, -5, -4, -4, 
                    -8, -4, -6, -3,
                    -5, -4, -3, -2,
                    0, 0, -1, -2,
                    13, 10, 8, 6,
                    21, 18, 10, 9,
                    21, 19, 14, 13,
            },
            // Queen
            {
                    -4, -4, -3, 0,
                    -5, 0, -1, 2,
                    -3, -1, 2, 4,
                    -1, 1, 3, 4,
                    5, 2, 0, -1,
                    10, 6, 4, 3,
                    18, 14, 9, 7,
                    20, 15, 11, 9,
            },
            // King
            {
                    -43, -35, -39, -30,
                    -35, -26, -30, -21,
                    -17, -9, -7, 4,
                    -4, 0, 5, 15,
                    12, 7, 12, 32,
                    26, 13, 16, 26,
                    28, 26, 24, 18,
                    -4, 11, 5, -1,
            }
        };

        /**
         * Bonus für das Zugrecht im Mittel- und Endspiel.
         */

        int16_t mgTempoBonus = 20;
        int16_t egTempoBonus = 8;

        /**
         * Bonus für angegriffene Figuren im Mittel- und Endspiel.
         * Angriffe auf kleinere Figuren, die von einem
         * Bauern gedeckt werden, werden nicht berücksichtigt.
         */

        int16_t mgAttackByMinorPieceBonus[5] = {
            2, 21, 35, 39, 35
        };

        int16_t mgAttackByRookBonus[5] = {
            1, 14, 16, 0, 23
        };

        int16_t egAttackByMinorPieceBonus[5] = {
            14, 17, 22, 42, 50
        };

        int16_t egAttackByRookBonus[5] = {
            20, 27, 24, 13, 14
        };

        /**
         * Bewertungen für verschiedene Merkmale in
         * Bauernstrukturen im Mittel- und Endspiel.
         */

        int16_t mgConnectedPawnBonus[6] = {
            2, 2, 9, 17, 28, 41
        }; // pro Rang (2 - 7)
        int16_t egConnectedPawnBonus[6] = {
            4, 4, 12, 19, 37, 54
        }; // pro Rang (2 - 7)
        int16_t mgDoubledPawnPenalty = -12;
        int16_t egDoubledPawnPenalty = -22;
        int16_t mgIsolatedPawnPenalty = -7;
        int16_t egIsolatedPawnPenalty = -16;
        int16_t mgBackwardPawnPenalty = -20;
        int16_t egBackwardPawnPenalty = -10;
        int16_t mgPassedPawnBonus[6] = {
            3, 3, 10, 17, 28, 35
        }; // pro Rang (2 - 7)
        int16_t egPassedPawnBonus[6] = {
            30, 30, 56, 71, 98, 125
        }; // pro Rang (2 - 7)
        int16_t mgCandidatePassedPawnBonus[5] = {
            2, 2, 6, 9, 14
        }; // pro Rang (2 - 6)
        int16_t egCandidatePassedPawnBonus[5] = {
            19, 19, 35, 46, 60
        }; // pro Rang (2 - 6)

        /**
         * Bewertungen für starke Felder im Mittelspiel.
         * Starke Felder sind Felder, die von einem eigenen Bauern
         * gedeckt sind und nie von einem gegnerischen Bauern
         * angegriffen werden können.
         */

        int16_t mgStrongSquareBonus = 19;

        /**
         * Ein Bonus für jedes sichere Feld
         * in der Mitte des Spielfeldes.
         */

        int16_t mgSpaceBonus = 4;

        /**
         * Ein Bonus für jedes Feld im Königsbereich des Gegners,
         * dass von einer eigenen Figur angegriffen wird.
         */

        int16_t numAttackerWeight[9] = {
            0, 0, 15, 40, 50, 70, 80, 92, 100
        };

        int16_t knightAttackBonus = 15;
        int16_t bishopAttackBonus = 10;
        int16_t rookAttackBonus = 41;
        int16_t queenAttackBonus = 83;

        /**
         * Bewertungen für verschiedene Merkmale in
         * Zusammenhang mit der Königssicherheit.
         */

        int16_t mgPawnShieldSizeBonus[4] = {
            0, 15, 48, 60
        };

        int16_t mgKingOpenFilePenalty[4] = {
            0, -31, -45, -52
        };

        int16_t mgPawnStormPenalty[6] = {
            -2, -2, -16, -24, -28, -30
        }; // pro Rang (2 - 7)

        /**
         * Bewertungen für die Mobilität der Figuren
         * im Mittel- und Endspiel.
         * (Springer, Läufer, Turm, Dame)
         */

        int16_t mgPieceMobilityBonus[4] = {
            1, // Knight
            3, // Bishop
            3, // Rook
            0, // Queen
        };
        int16_t egPieceMobilityBonus[4] = {
            1, // Knight
            2, // Bishop
            3, // Rook
            1, // Queen
        };

        int16_t mgPieceNoMobilityPenalty[4] = {
            -28, // Knight
            -46, // Bishop
            -61, // Rook
            -77, // Queen
        };
        int16_t egPieceNoMobilityPenalty[4] = {
            -8, // Knight
            -21, // Bishop
            -30, // Rook
            -48, // Queen
        };

        /**
         * Ein Bonus für jede Leichtfigur,
         * die auf einem starken Feld steht oder
         * im nächsten Zug erreichen kann.
         */

        int16_t mgKnightOnStrongSquareBonus = 25;
        int16_t mgBishopOnStrongSquareBonus = 10;

        /**
         * Eine Bestrafung für Läufer, die durch eigene
         * unbewegbare Bauern oder gegnerische, durch andere
         * Bauern gedeckte, Bauern blockiert werden.
         */

        int16_t mgBadBishopPenalty = -18;
        int16_t egBadBishopPenalty = -10;

        /**
         * Ein Bonus für jede offene oder halboffene
         * Linie, auf der sich ein Turm befindet.
         */

        int16_t mgRookOnOpenFileBonus = 20;
        int16_t mgRookOnSemiOpenFileBonus = 5;

        /**
         * Ein Bonus für jeden Freibauern, der von
         * einem Turm auf derselben Linie verteidigt wird.
         */

        int16_t egRookBehindPassedPawnBonus = 30;

        /**
         * @brief Bonus für jede Figur, die einen
         * gegnerischen Freibauern blockiert.
         */

        int16_t egBlockedEnemyPassedPawnBonus = 20;

        /**
         * @brief Bestrafung für jede eigene Figur, die einen
         * eigenen Freibauern blockiert.
         */

        int16_t egBlockedOwnPassedPawnPenalty = -12;

        /**
         * Gewichtungsparameter für die Bewertung der Distanz
         * des Königs zu Bauern im Endspiel.
         */

        int16_t egKingProximityPawnWeight = 2;
        int16_t egKingProximityBackwardPawnWeight = 3;
        int16_t egKingProximityPassedPawnWeight = 5;

        /**
         * @brief Bonus für Freibauern in einem Bauernendspiel,
         * die nicht mehr vom gegnerischen König erreicht werden können.
         */

        int16_t ruleOfTheSquareBonus = 700;

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

        inline int getMGPawnShieldSizeBonus(int size) const { return mgPawnShieldSizeBonus[size]; }
        inline int getMGKingOpenFilePenalty(int numFiles) const { return mgKingOpenFilePenalty[numFiles]; }
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
        inline int getEGBlockedOwnPassedPawnPenalty() const { return egBlockedOwnPassedPawnPenalty; }

        inline int getEGKingProximityPawnWeight() const { return egKingProximityPawnWeight; }
        inline int getEGKingProximityBackwardPawnWeight() const { return egKingProximityBackwardPawnWeight; }
        inline int getEGKingProximityPassedPawnWeight() const { return egKingProximityPassedPawnWeight; }

        inline int getRuleOfTheSquareBonus() const { return ruleOfTheSquareBonus; }
};

extern HCEParameters HCE_PARAMS;

#endif