#ifndef HCE_PARAMETERS_H
#define HCE_PARAMETERS_H

#include <fstream>
#include <stdint.h>

#include "core/chess/BoardDefinitions.h"

class HCEParameters {

    public:
        /**
         * Faktoren für das Polynom zweiten Grades zur
         * Berechnung des Materialwertes einer Farbe.
         */

        int16_t pieceValues[20] = {
            121, 150, 235, 556, 424,
            -2, 6, 43, 23, 362,
            -10, -10, 1, -11, 35,
            37, 9, 48, 74, 154
        };

        /**
         * Faktoren für die Funktion zur Berechnung
         * des Materialungleichgewichts.
         */

        int16_t pieceImbalanceValues[10] = {
            -18, -15, -20, -15, -72,
            -31, 0, -70, -2, 128
        };

        /**
         * Positionstabellen für alle Figuren im Mittel- und Endspiel.
         */

        int16_t mgPSQTPawn[64] = {
            0,   0,   0,   0,   0,   0,   0,   0,
            -31, -31, -26, -49,  -8,  15,  30, -19,
            -32, -27, -17, -17,   6,   2,  -5,   7,
            -8, -12,  -4,  21, -18,  10, -22,  -8,
            -16,  -5, -42, -31,  17,  34, -20, -20,
            44,  76,  24,  44, 102, 130, 111,  94,
            120,  46,  74, 136,  19, -30,  18, 100,
            0,   0,   0,   0,   0,   0,   0,   0,
        };

        int16_t mgPSQTPacked[5][32] = {
            {
                -85, -13, -31,   9,
                -26, -34,  37,  24,
                13,  12,  26,  15,
                3,  38,  59,  33,
                61,  34,  32,  59,
                14,   3,  44,  51,
                22, -19,  63,  69,
                -65, -21, -72,  89,
            },
            {
                -30,  -8,   4, -23,
                -2,  21,  62,  19,
                13,  34,  30,  30,
                -17, -10,  17,  56,
                -18,   1,  32,   7,
                4,  25,  56,  44,
                -28,   7,  14, -24,
                -52, -46, -21, -97,
            },
            {
                10,  -8,   8,  19,
                -80,  -8, -35, -11,
                -37,  10, -47, -33,
                -14, -15, -27,  -1,
                 -7,   3,  28,   8,
                -12,  32,  26,  93,
                 41,  62,  66,  52,
                104, 126,  62, 103,
            },
            {
                35, -48,  -2,  32,
                -76,  45,  34,  23,
                -22,  57,  59,  57,
                 27,  27,  41,  36,
                 17,  47,  28,  46,
                -11,  62,  56,  62,
                 36,   6,  69,  51,
                 43,  -2,  28,  65,
            },
            {
                40,  53, -40, -12,
                49,  60,  15, -11,
               -53,  77,  -4, -17,
               -89, -67, -18, -24,
               -133, -83, -80,  73,
               -114, -48, -70,  -2,
               -15, -41, -47, -14,
               -75,  28,  -2,  39,
            }
        };

        int16_t egPSQTPawn[64] = {
            0,   0,   0,   0,   0,   0,   0,   0,
            46,  65,  -7,  -9,  11,  14,  32,   5,
            30,  31,  35,  21,  17,  36,  21,  -2,
            67,  42,   3,   8,  56,  22,  -8,  15,
            89,  45,  13,   8,   2,  34,  27,  38,
           100,  71,  25, 107,  32, 155,  66,  72,
           133, 116, 161, 179, 153, 185, 155, 132,
            0,   0,   0,   0,   0,   0,   0,   0,
        };

        int16_t egPSQTPacked[5][32] = {
            {
                75, 142,  57,  50,
                71,  78,  37,  65,
                84,  16,  50,  91,
                76,  18,  57,  74,
                25,  85,  38,  79,
                38,  78,  37,  86,
               119,  78,  54,   0,
                -4,  96,  83,  57,
            },
            {
                83,  93,  89,  92,
                99,  50,  40,  79,
                39,  30,  50,  64,
                23,  66,  35,  30,
                79,  61,  79,  10,
                94,  48,  57,  40,
               100,  91,  36,  87,
               128,  51,  81, 121,
            },
            {
                58,  72,  75,  41,
                48,  56,  85,  65,
                85,  84, 100,  86,
                95,  88,  66,  83,
               120, 134,  84,  90,
               101, 112,  83,  60,
                76, 109, 121,  92,
                75, 101, 110,  98,
            },
            {
                1,  10,  22,  20,
                44,  11, -27,  -7,
                21,  59,  91,  65,
                33, 102,  54,  44,
               138,  88,  33,  79,
               163,  85, 163, 122,
               113,  81,  45, 127,
               106,  74,  76,  76,
            },
            {
                -20, -14,  -3, -28,
                -17,  25, -11, -12,
                -23,  -9,  10,  -4,
                -16,   7,  16,  24,
                32,  50,  49,  38,
                57, 104,  47,  56,
                42,  82,  42,  44,
                -142,  14,  54, -13,
            }
        };

        /**
         * Bonus für das Zugrecht im Mittel- und Endspiel.
         */

        int16_t mgTempoBonus = 22;
        int16_t egTempoBonus = 10;

        /**
         * Bonus für angegriffene Figuren im Mittel- und Endspiel.
         * Angriffe auf kleinere Figuren, die von einem
         * Bauern gedeckt werden, werden nicht berücksichtigt.
         */

        int16_t mgAttackByMinorPieceBonus[5] = {
            14, 17, 35, 64, 55
        };

        int16_t mgAttackByRookBonus[5] = {
            -6, 54, 26, 35, 51
        };

        int16_t egAttackByMinorPieceBonus[5] = {
            21, 61, 17, 21, 43
        };

        int16_t egAttackByRookBonus[5] = {
            45, 34, 39, 3, -12
        };

        /**
         * Bewertungen für verschiedene Merkmale in
         * Bauernstrukturen im Mittel- und Endspiel.
         */

        int16_t mgConnectedPawnBonus[6] = {
            -2, 7, 6, 9, 37, 63
        }; // pro Rang (2 - 7)
        int16_t egConnectedPawnBonus[6] = {
            4, 9, 5, 15, 37, 3
        }; // pro Rang (2 - 7)
        int16_t mgDoubledPawnPenalty[4] = {
            -19, -15, -9, -13
        }; // pro Linie (1 - 4, gespiegelt)
        int16_t egDoubledPawnPenalty[4] = {
            -19, -27, -8, -20
        }; // pro Linie (1 - 4, gespiegelt)
        int16_t mgIsolatedPawnPenalty[4] = {
            -2, -6, -31, -32
        }; // pro Linie (1 - 4, gespiegelt)
        int16_t egIsolatedPawnPenalty[4] = {
            -14, -20, -16, -18
        }; // pro Linie (1 - 4, gespiegelt)
        int16_t mgBackwardPawnPenalty[5] = {
            -7, -14, -11, -11, -12
        }; // pro Rang (2 - 6)
        int16_t egBackwardPawnPenalty[5] = {
            -13, -6, -29, -35, -2
        }; // pro Rang (2 - 6)
        int16_t mgPassedPawnBonus[6] = {
            2, 10, 15, 72, 109, 161
        }; // pro Rang (2 - 7)
        int16_t egPassedPawnBonus[6] = {
            26, 30, 76, 101, 174, 262
        }; // pro Rang (2 - 7)
        int16_t mgCandidatePassedPawnBonus[5] = {
            18, 6, 10, 20, 75
        }; // pro Rang (2 - 6)
        int16_t egCandidatePassedPawnBonus[5] = {
            17, 4, 24, 32, 35
        }; // pro Rang (2 - 6)
        int16_t mgConnectedPassedPawnBonus[6] = {
            -8, 19, 19, -15, 17, -2
        }; // pro Rang (2 - 7)
        int16_t egConnectedPassedPawnBonus[6] = {
            68, 5, -34, -21, 40, 19
        }; // pro Rang (2 - 7)

        /**
         * Bewertungen für starke Felder im Mittelspiel.
         * Starke Felder sind Felder, die von einem eigenen Bauern
         * gedeckt sind und nie von einem gegnerischen Bauern
         * angegriffen werden können.
         */

        int16_t mgCenterOutpostBonus = 8; // Linien C - F
        int16_t mgEdgeOutpostBonus = 3; // Linien A - B und G - H

        /**
         * Bewertungen für die Sicherheit des Königs im Mittel- und Endspiel.
         */

        int16_t mgAttackWeight[4] = {
            3, 2, 2, 2
        }; // Gewichtung der Angreifer (Springer, Läufer, Turm, Dame)
        int16_t egAttackWeight[4] = {
            3, 1, 2, 4
        }; // Gewichtung der Angreifer (Springer, Läufer, Turm, Dame)
        int16_t mgUndefendedAttackWeight[4] = {
            2, 2, 1, 2
        }; // Gewichtung der unverteidigten Angriffe (Springer, Läufer, Turm, Dame)
        int16_t egUndefendedAttackWeight[4] = {
            2, 1, 2, 3
        }; // Gewichtung der unverteidigten Angriffe (Springer, Läufer, Turm, Dame)
        int16_t mgSafeCheckWeight[4] = {
            6, 7, 6, 10
        }; // (Springer, Läufer, Turm, Dame)
        int16_t egSafeCheckWeight[4] = {
            6, 9, 10, 10
        }; // (Springer, Läufer, Turm, Dame)
        int16_t mgSafeContactCheckWeight[2] = {
            3, 11
        }; // (Turm, Dame)
        int16_t egSafeContactCheckWeight[2] = {
            8, 13
        }; // (Turm, Dame)
        int16_t mgDefenseWeight[4] = {
            1, 1, 0, 0
        }; // Gewichtung der Verteidiger (Springer, Läufer, Turm, Dame)
        int16_t egDefenseWeight[4] = {
            0, 0, 0, 0
        }; // Gewichtung der Verteidiger (Springer, Läufer, Turm, Dame)

        /**
         * Bewertungen für verschiedene Merkmale in
         * Zusammenhang mit der Königssicherheit.
         */

        int16_t mgPawnShieldSizeBonus[3] = {-6, 27, 16};

        int16_t mgKingOpenFilePenalty[3] = {-27, -32, -28};

        int16_t mgPawnStormBonus[5] = {-13, -11, -6, -2, 67}; // pro Rang (2 - 6)

        /**
         * Bewertungen für die Mobilität der Figuren
         * im Mittel- und Endspiel.
         * (Springer, Läufer, Turm, Dame)
         */

        int16_t mgPieceMobilityBonus[4] = {
            1, 5, 14, 7
        };
        int16_t egPieceMobilityBonus[4] = {
            107, 64, 56, 145
        };

        int16_t mgPieceNoMobilityPenalty[4] = {
            -15, -106, -62, -23
        };
        int16_t egPieceNoMobilityPenalty[4] = {
            -30, -17, -21, -8
        };

        /**
         * Ein Bonus für jede Leichtfigur,
         * die auf einem starken Feld steht oder
         * im nächsten Zug erreichen kann.
         */

        int16_t mgKnightOnCenterOutpostBonus = 49;
        int16_t mgKnightOnEdgeOutpostBonus = 24;
        int16_t mgBishopOnCenterOutpostBonus = 40;
        int16_t mgBishopOnEdgeOutpostBonus = 20;

        /**
         * Eine Bestrafung für Läufer, die durch eigene
         * unbewegbare Bauern oder gegnerische, durch andere
         * Bauern gedeckte, Bauern blockiert werden.
         */

        int16_t mgBadBishopPenalty = -6;
        int16_t egBadBishopPenalty = -2;

        /**
         * Ein Bonus für jede offene oder halboffene
         * Linie, auf der sich ein Turm befindet.
         */

        int16_t mgRookOnOpenFileBonus = 31;
        int16_t mgRookOnSemiOpenFileBonus = 13;

        /**
         * Ein Bonus für gedoppelte Türme auf
         * halboffenen oder offenen Linien.
         */
        int16_t mgDoubledRooksOnOpenFileBonus = 40;
        int16_t mgDoubledRooksOnSemiOpenFileBonus = 8;

        /**
         * Ein Bonus für jeden Freibauern, der von
         * einem Turm auf derselben Linie verteidigt wird.
         */

        int16_t egRookBehindPassedPawnBonus = 51;

        /**
         * @brief Bonus für jede Figur, die einen
         * gegnerischen Freibauern blockiert.
         */

        int16_t egBlockedEnemyPassedPawnBonus = 57;

        /**
         * @brief Bonus für jeden Angriff auf den
         * Promotionspfad eines Freibauern.
         */
        int16_t egAttackOnPassedPawnPathBonus = 4;

        /**
         * Gewichtungsparameter für die Bewertung der Distanz
         * des Königs zu Bauern im Endspiel.
         */

        int16_t egKingProximityPawnWeight = 5;
        int16_t egKingProximityBackwardPawnWeight = 5;
        int16_t egKingProximityPassedPawnWeight = 23;

        /**
         * @brief Bonus für Freibauern in einem Bauernendspiel,
         * die nicht mehr vom gegnerischen König erreicht werden können.
         */

        int16_t ruleOfTheSquareBonus = 492;

        /**
         * @brief Bestrafung für ein Endspiel mit Läufern
         * unterschiedlicher Farbe. Die Bestrafung wird
         * der ansonsten führenden Seite angerechnet und
         * kann das Vorzeichen der Bewertung nicht ändern.
         */
        int16_t oppositeColorBishopsPenalty = -257;

        /**
         * @brief Bestrafung für ein Endspiel mit Türmen
         * und Bauern (und maximal einer weiteren Leichtfigur).
         * Die Bestrafung wird der ansonsten führenden Seite
         * angerechnet und kann das Vorzeichen der Bewertung nicht ändern.
         */
        int16_t rookEndgamePenalty = -18;

        /**
         * Parameter für die Mop-Up Evaluation.
         */

        int16_t egMopupBaseBonus = 800;
        int16_t egMopupProgressBonus = 150;

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

        inline int getMGDoubledPawnPenalty(int file) const { return mgDoubledPawnPenalty[file & 4 ? file ^ 7 : file]; }
        inline int getEGDoubledPawnPenalty(int file) const { return egDoubledPawnPenalty[file & 4 ? file ^ 7 : file]; }

        inline int getMGIsolatedPawnPenalty(int file) const { return mgIsolatedPawnPenalty[file & 4 ? file ^ 7 : file]; }
        inline int getEGIsolatedPawnPenalty(int file) const { return egIsolatedPawnPenalty[file & 4 ? file ^ 7 : file]; }

        inline int getMGBackwardPawnPenalty(int rank) const { return mgBackwardPawnPenalty[rank - 1]; }
        inline int getEGBackwardPawnPenalty(int rank) const { return egBackwardPawnPenalty[rank - 1]; }

        inline int getMGPassedPawnBonus(int rank) const { return mgPassedPawnBonus[rank - 1]; }
        inline int getEGPassedPawnBonus(int rank) const { return egPassedPawnBonus[rank - 1]; }

        inline int getMGCandidatePassedPawnBonus(int rank) const { return mgCandidatePassedPawnBonus[rank - 1]; }
        inline int getEGCandidatePassedPawnBonus(int rank) const { return egCandidatePassedPawnBonus[rank - 1]; }

        inline int getMGConnectedPassedPawnBonus(int rank) const { return mgConnectedPassedPawnBonus[rank - 1]; }
        inline int getEGConnectedPassedPawnBonus(int rank) const { return egConnectedPassedPawnBonus[rank - 1]; }

        inline int getMGCenterOutpostBonus() const { return mgCenterOutpostBonus; }
        inline int getMGEdgeOutpostBonus() const { return mgEdgeOutpostBonus; }

        inline int getMGAttackWeight(int piece) const { return mgAttackWeight[piece - 2]; }
        inline int getEGAttackWeight(int piece) const { return egAttackWeight[piece - 2]; }
        inline int getMGUndefendedAttackWeight(int piece) const { return mgUndefendedAttackWeight[piece - 2]; }
        inline int getEGUndefendedAttackWeight(int piece) const { return egUndefendedAttackWeight[piece - 2]; }
        inline int getMGSafeCheckWeight(int piece) const { return mgSafeCheckWeight[piece - 2]; }
        inline int getEGSafeCheckWeight(int piece) const { return egSafeCheckWeight[piece - 2]; }
        inline int getMGSafeContactCheckWeight(int piece) const { return mgSafeContactCheckWeight[piece - 4]; }
        inline int getEGSafeContactCheckWeight(int piece) const { return egSafeContactCheckWeight[piece - 4]; }
        inline int getMGDefenseWeight(int piece) const { return mgDefenseWeight[piece - 2]; }
        inline int getEGDefenseWeight(int piece) const { return egDefenseWeight[piece - 2]; }

        inline int getMGPawnShieldSizeBonus(int size) const { return size == 0 ? 0 : mgPawnShieldSizeBonus[size - 1]; }
        inline int getMGKingOpenFilePenalty(int numFiles) const { return numFiles == 0 ? 0 : mgKingOpenFilePenalty[numFiles - 1]; }
        inline int getMGPawnStormBonus(int rank) const { return mgPawnStormBonus[rank - 1]; }

        inline int getMGPieceMobilityBonus(int piece) const { return mgPieceMobilityBonus[piece - 2]; }
        inline int getEGPieceMobilityBonus(int piece) const { return egPieceMobilityBonus[piece - 2]; }

        inline int getMGPieceNoMobilityPenalty(int piece) const { return mgPieceNoMobilityPenalty[piece - 2]; }
        inline int getEGPieceNoMobilityPenalty(int piece) const { return egPieceNoMobilityPenalty[piece - 2]; }

        inline int getMGKnightOnCenterOutpostBonus() const { return mgKnightOnCenterOutpostBonus; }
        inline int getMGKnightOnEdgeOutpostBonus() const { return mgKnightOnEdgeOutpostBonus; }
        inline int getMGBishopOnCenterOutpostBonus() const { return mgBishopOnCenterOutpostBonus; }
        inline int getMGBishopOnEdgeOutpostBonus() const { return mgBishopOnEdgeOutpostBonus; }

        inline int getMGBadBishopPenalty() const { return mgBadBishopPenalty; }
        inline int getEGBadBishopPenalty() const { return egBadBishopPenalty; }

        inline int getMGRookOnOpenFileBonus() const { return mgRookOnOpenFileBonus; }
        inline int getMGRookOnSemiOpenFileBonus() const { return mgRookOnSemiOpenFileBonus; }

        inline int getMGDoubledRooksOnOpenFileBonus() const { return mgDoubledRooksOnOpenFileBonus; }
        inline int getMGDoubledRooksOnSemiOpenFileBonus() const { return mgDoubledRooksOnSemiOpenFileBonus; }

        inline int getEGRookBehindPassedPawnBonus() const { return egRookBehindPassedPawnBonus; }
        inline int getEGBlockedEnemyPassedPawnBonus() const { return egBlockedEnemyPassedPawnBonus; }

        inline int getEGAttackOnPassedPawnPathBonus() const { return egAttackOnPassedPawnPathBonus; }

        inline int getEGKingProximityPawnWeight() const { return egKingProximityPawnWeight; }
        inline int getEGKingProximityBackwardPawnWeight() const { return egKingProximityBackwardPawnWeight; }
        inline int getEGKingProximityPassedPawnWeight() const { return egKingProximityPassedPawnWeight; }

        inline int getRuleOfTheSquareBonus() const { return ruleOfTheSquareBonus; }

        inline int getOppositeColorBishopsPenalty() const { return oppositeColorBishopsPenalty; }

        inline int getRookEndgamePenalty() const { return rookEndgamePenalty; }

        inline int getMopupBaseBonus() const { return egMopupBaseBonus; }
        inline int getMopupProgressBonus() const { return egMopupProgressBonus; }
};

extern HCEParameters HCE_PARAMS;

#endif