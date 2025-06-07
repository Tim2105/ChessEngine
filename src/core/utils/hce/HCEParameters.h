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
            123, 159, 282, 585, 454,
            -4, 6, 46, 45, 392,
            -6, -11, 36, 8, 42,
            64, 12, 55, 80, 173
        };

        /**
         * Faktoren für die Funktion zur Berechnung
         * des Materialungleichgewichts.
         */

        int16_t pieceImbalanceValues[10] = {
            -7, -12, -25, -12, -73,
            -45, 7, -73, -1, 130
        };

        /**
         * Positionstabellen für alle Figuren im Mittel- und Endspiel.
         */

        int16_t mgPSQTPawn[64] = {
            0,   0,   0,   0,   0,   0,   0,   0,
            -33, -25, -15, -32, -30,  22,  20,  -6,
            -25, -25, -27, -19, -13, -11,   2,   3,
            -11,  -4,  -5,   3,   9,  10, -17, -18,
            -17,  -2,  -9,   3,  22,  18,   2, -28,
            41,  54,  34,  32,  96, 126, 108,  91,
            133,  58,  88, 135,  22, -20,  18,  77,
            0,   0,   0,   0,   0,   0,   0,   0,
        };

        int16_t mgPSQTPacked[5][32] = {
            {
                -74, -18, -34,   6,
                -29, -23,  30,  19,
                -1,  14,  32,  18,
                3,  33,  45,  45,
                61,  28,  49,  69,
                40,   6,  62,  53,
                24, -16,  84,  89,
                -53, -11, -59,  97,
            },
            {
                -29,   4,   4, -16,
                -7,  32,  45,  24,
                14,  24,  21,  37,
                -11,  11,  20,  35,
                -2,   4,  31,  37,
                20,  33,  48,  43,
                -27, -14,  21,  -4,
                -43, -32, -34, -93,
            },
            {
                  2,  -9,   8,  23,
                -79, -25, -15, -15,
                -43, -15, -27, -17,
                -12, -12,  -7,  -2,
                -12,  -2,  21,  16,
                0,  40,  49,  85,
                36,  47,  60,  57,
                110, 110,  62, 100,
            },
            {
                 30, -22,  -7,  34,
                -58,  33,  37,  35,
                4,  34,  36,  37,
                38,  43,  38,  54,
                40,  41,  45,  57,
                6,  76,  53,  60,
                60,  14,  78,  48,
                45,  20,  22,  67,
            },
            {
                 57,  49, -25,  -6,
                52,  56,   0, -16,
                -49,  62,  -5, -24,
                -102, -74, -13, -27,
                -128, -89, -67,  62,
                -114, -48, -52,   5,
                -17, -37, -49, -15,
                -78,  32, -12,  40,
            }
        };

        int16_t egPSQTPawn[64] = {
            0,   0,   0,   0,   0,   0,   0,   0,
            42,  60,  13,   1,  25,  21,  23,  16,
            47,  45,   7,  22,   8,  16,  18,   8,
            53,  37,  33,  27,  31,   7,   5,  11,
            79,  60,  37,  20,  22,  15,  30,  23,
            90,  89,  48,  72,  47,  78,  78,  61,
            123, 146, 160, 158, 146, 173, 152, 120,
            0,   0,   0,   0,   0,   0,   0,   0,
        };

        int16_t egPSQTPacked[5][32] = {
            {
                72, 116,  51,  51,
                65,  61,  29,  57,
                80,  39,  62,  74,
                87,  32,  62,  79,
                52,  82,  60,  86,
                45,  92,  52,  83,
                118,  90,  54,  32,
                8,  96,  96,  52,
            },
            {
                86,  68,  76,  92,
                81,  76,  23,  72,
                48,  43,  60,  65,
                29,  53,  53,  38,
                82,  88,  66,  30,
                100,  61,  47,  46,
                115,  63,  43,  95,
                122,  65,  70, 113,
            },
            {
                72,  73,  70,  61,
                54,  73,  74,  61,
                84,  80,  96,  72,
                84, 105,  79,  94,
                105, 122,  93,  92,
                116, 111, 104,  80,
                86,  98, 100,  94,
                79, 101, 108,  94,
            },
            {
                2,  19,  13,   7,
                66,   4, -16,  11,
                26,  54,  66,  56,
                37,  96,  51,  56,
                140,  95,  63,  81,
                163,  84, 144, 119,
                127,  94,  69, 140,
                108,  94,  82,  94,
            },
            {
                -7, -10,  -1, -16,
                -9,   5,  -4, -14,
                -1, -17,   4,  -1,
                -12,   4,  22,  18,
                27,  52,  41,  27,
                43,  86,  53,  30,
                44,  83,  55,  40,
                -142,  23,  41,  11,
            }
        };

        /**
         * Bonus für das Zugrecht im Mittel- und Endspiel.
         */

        int16_t mgTempoBonus = 16;
        int16_t egTempoBonus = 16;

        /**
         * Bonus für angegriffene Figuren im Mittel- und Endspiel.
         * Angriffe auf kleinere Figuren, die von einem
         * Bauern gedeckt werden, werden nicht berücksichtigt.
         */

        int16_t mgAttackByMinorPieceBonus[5] = {
            12, 19, 33, 72, 55
        };

        int16_t mgAttackByRookBonus[5] = {
            0, 48, 31, 44, 57
        };

        int16_t egAttackByMinorPieceBonus[5] = {
            40, 52, 19, 18, 37
        };

        int16_t egAttackByRookBonus[5] = {
            32, 32, 36, 2, -8
        };

        /**
         * Bewertungen für verschiedene Merkmale in
         * Bauernstrukturen im Mittel- und Endspiel.
         */

        int16_t mgConnectedPawnBonus[6] = {
            5, 11, 14, 21, 40, 72
        }; // pro Rang (2 - 7)
        int16_t egConnectedPawnBonus[6] = {
            3, -5, 5, 15, 26, 47
        }; // pro Rang (2 - 7)
        int16_t mgDoubledPawnPenalty[4] = {
            -18, -6, -5, -23
        }; // pro Linie (1 - 4, gespiegelt)
        int16_t egDoubledPawnPenalty[4] = {
            -39, -16, -7, -8
        }; // pro Linie (1 - 4, gespiegelt)
        int16_t mgIsolatedPawnPenalty[4] = {
            -12, -7, -10, -33
        }; // pro Linie (1 - 4, gespiegelt)
        int16_t egIsolatedPawnPenalty[4] = {
            -12, -18, -12, -24
        }; // pro Linie (1 - 4, gespiegelt)
        int16_t mgBackwardPawnPenalty[5] = {
            -24, -27, -18, -10, 5
        }; // pro Rang (2 - 6)
        int16_t egBackwardPawnPenalty[5] = {
            -10, -26, -32, -59, 8
        }; // pro Rang (2 - 6)
        int16_t mgPassedPawnBonus[6] = {
            18, 13, 30, 70, 89, 160
        }; // pro Rang (2 - 7)
        int16_t egPassedPawnBonus[6] = {
            25, 32, 63, 109, 160, 237
        }; // pro Rang (2 - 7)
        int16_t mgCandidatePassedPawnBonus[5] = {
            18, 17, 16, 36, 67
        }; // pro Rang (2 - 6)
        int16_t egCandidatePassedPawnBonus[5] = {
            3, 10, 19, 30, 44
        }; // pro Rang (2 - 6)
        int16_t mgConnectedPassedPawnBonus[6] = {
            -25, 19, 7, 10, -5, -7
        }; // pro Rang (2 - 7)
        int16_t egConnectedPassedPawnBonus[6] = {
            39, -1, 6, 9, 29, 36
        }; // pro Rang (2 - 7)

        /**
         * Bewertungen für starke Felder im Mittelspiel.
         * Starke Felder sind Felder, die von einem eigenen Bauern
         * gedeckt sind und nie von einem gegnerischen Bauern
         * angegriffen werden können.
         */

        int16_t mgCenterOutpostBonus = -4; // Linien C - F
        int16_t mgEdgeOutpostBonus = 16; // Linien A - B und G - H

        /**
         * Bewertungen für die Sicherheit des Königs im Mittel- und Endspiel.
         */

        int16_t mgAttackWeight[4] = {
            4, 3, 4, 0
        }; // Gewichtung der Angreifer (Springer, Läufer, Turm, Dame)
        int16_t egAttackWeight[4] = {
            -3, -2, -3, 2
        }; // Gewichtung der Angreifer (Springer, Läufer, Turm, Dame)
        int16_t mgUndefendedAttackWeight[4] = {
            8, 5, 3, 7
        }; // Gewichtung der unverteidigten Angriffe (Springer, Läufer, Turm, Dame)
        int16_t egUndefendedAttackWeight[4] = {
            -1, 7, -3, 1
        }; // Gewichtung der unverteidigten Angriffe (Springer, Läufer, Turm, Dame)
        int16_t mgSafeCheckWeight[4] = {
            17, 11, 14, 20
        }; // (Springer, Läufer, Turm, Dame)
        int16_t egSafeCheckWeight[4] = {
            3, 20, 7, 20
        }; // (Springer, Läufer, Turm, Dame)
        int16_t mgSafeContactCheckWeight[2] = {
            7, 25
        }; // (Turm, Dame)
        int16_t egSafeContactCheckWeight[2] = {
            6, 14
        }; // (Turm, Dame)
        int16_t mgDefenseWeight[4] = {
            3, -1, 1, -3
        }; // Gewichtung der Verteidiger (Springer, Läufer, Turm, Dame)
        int16_t egDefenseWeight[4] = {
            -12, -4, 0, 6
        }; // Gewichtung der Verteidiger (Springer, Läufer, Turm, Dame)
        int16_t mgPawnShieldSizeWeight[3] = {
            3, 6, 4
        }; // Gewichtung der Größe des Bauernschildes (defensiv)
        int16_t mgKingOpenFileWeight[3] = {
            10, 16, 19
        }; // Gewichtung der offenen Linien (offensiv)
        int16_t mgPawnStormWeight[5] = {
            0, -5, 3, 10, 11
        }; // pro Rang (2 - 6)

        /**
         * Bewertungen für die Mobilität der Figuren
         * im Mittel- und Endspiel.
         * (Springer, Läufer, Turm, Dame)
         */

        int16_t mgPieceMobilityBonus[4] = {
            7, 13, 18, 9
        };
        int16_t egPieceMobilityBonus[4] = {
            112, 63, 59, 150
        };

        int16_t mgPieceNoMobilityPenalty[4] = {
            -93, -102, -157, -187
        };
        int16_t egPieceNoMobilityPenalty[4] = {
            -44, -44, -55, -58
        };

        /**
         * Ein Bonus für jede Leichtfigur,
         * die auf einem starken Feld steht oder
         * im nächsten Zug erreichen kann.
         */

        int16_t mgKnightOnCenterOutpostBonus = 56;
        int16_t mgKnightOnEdgeOutpostBonus = 23;
        int16_t mgBishopOnCenterOutpostBonus = 52;
        int16_t mgBishopOnEdgeOutpostBonus = 30;

        /**
         * Eine Bestrafung für Läufer, die durch eigene
         * unbewegbare Bauern oder gegnerische, durch andere
         * Bauern gedeckte, Bauern blockiert werden.
         */

        int16_t mgBadBishopPenalty = -10;
        int16_t egBadBishopPenalty = -18;

        /**
         * Ein Bonus für jede offene oder halboffene
         * Linie, auf der sich ein Turm befindet.
         */

        int16_t mgRookOnOpenFileBonus = 47;
        int16_t mgRookOnSemiOpenFileBonus = 26;

        /**
         * Ein Bonus für gedoppelte Türme auf
         * halboffenen oder offenen Linien.
         */
        int16_t mgDoubledRooksOnOpenFileBonus = 41;
        int16_t mgDoubledRooksOnSemiOpenFileBonus = 27;

        /**
         * Ein Bonus für jeden Freibauern, der von
         * einem Turm auf derselben Linie verteidigt wird.
         */

        int16_t egRookBehindPassedPawnBonus = 47;

        /**
         * @brief Bonus für jede Figur, die einen
         * gegnerischen Freibauern blockiert.
         */

        int16_t egBlockedEnemyPassedPawnBonus = 41;

        /**
         * @brief Bonus für jeden Angriff auf den
         * Promotionspfad eines Freibauern.
         */
        int16_t egAttackOnPassedPawnPathBonus = 2;

        /**
         * Gewichtungsparameter für die Bewertung der Distanz
         * des Königs zu Bauern im Endspiel.
         */

        int16_t egKingProximityPawnWeight = 3;
        int16_t egKingProximityBackwardPawnWeight = 9;
        int16_t egKingProximityPassedPawnWeight = 19;

        /**
         * @brief Bonus für Freibauern in einem Bauernendspiel,
         * die nicht mehr vom gegnerischen König erreicht werden können.
         */

        int16_t ruleOfTheSquareBonus = 469;

        /**
         * "Draw Conditions"
         */

        /**
         * @brief Bestrafung für ein Endspiel mit Läufern
         * unterschiedlicher Farbe. Die Bestrafung wird
         * der ansonsten führenden Seite angerechnet und
         * kann das Vorzeichen der Bewertung nicht ändern.
         */
        int16_t oppositeColorBishopsEndgameWinnablePenalty = -221;

        /**
         * @brief Bestrafung, wenn beide Seiten einen Läufer
         * auf unterschiedlichen Farben haben. Die Bestrafung
         * wird der ansonsten führenden Seite angerechnet und
         * kann das Vorzeichen der Bewertung nicht ändern.
         */

        int16_t mgOppositeColorBishopsWinnablePenalty = -30;
        int16_t egOppositeColorBishopsWinnablePenalty = -80;

        /**
         * @brief Bestrafung für ein Endspiel mit Türmen
         * und Bauern (und maximal einer weiteren Leichtfigur).
         * Die Bestrafung wird der ansonsten führenden Seite
         * angerechnet und kann das Vorzeichen der Bewertung nicht ändern.
         */
        int16_t rookEndgameWinnablePenalty = -48;

        /**
         * @brief Konstante Bestrafung Richtung 0 für
         * Mittelspiel und Endspiel. Die Bestrafung wird
         * der ansonsten führenden Seite angerechnet und
         * kann das Vorzeichen der Bewertung nicht ändern.
         */

        int16_t mgDefaultWinnablePenalty = -50;
        int16_t egDefaultWinnablePenalty = -70;

        /**
         * @brief Bonus für ein Endspiel mit nur Bauern
         * und Königen. Der Bonus kann die Vorzeichen-
         * erhaltenden Bestrafungen abschwächen, aber nicht
         * die tatsächliche Bewertung erhöhen.
         */
        int16_t kingAndPawnEndgameWinnableBonus = 30;

        /**
         * @brief Bonus für jeden Bauern. Der Bonus kann
         * die Vorzeichen-erhaltenden Bestrafungen
         * abschwächen, aber nicht die tatsächliche
         * Bewertung erhöhen.
         */

        int16_t mgPawnWinnableBonus = 8;
        int16_t egPawnWinnableBonus = 10;

        /**
         * @brief Bonus für jeden Freibauern(kandidat).
         * Der Bonus kann die Vorzeichen-erhaltenden
         * Bestrafungen abschwächen, aber nicht
         * die tatsächliche Bewertung erhöhen.
         */

        int16_t mgPassedPawnWinnableBonus = 5;
        int16_t egPassedPawnWinnableBonus = 7;

        /**
         * @brief Bonus für einen König, der
         * auf der gegnerischen Seite steht.
         * Der Bonus kann die Vorzeichen-erhaltenden
         * Bestrafungen abschwächen, aber nicht
         * die tatsächliche Bewertung erhöhen.
         */

        int16_t kingInfiltrationWinnableBonus = 20;

        /**
         * Parameter für die Mop-Up Evaluation.
         */

        int16_t egMopupBaseBonus = 760;
        int16_t egMopupProgressBonus = 94;

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

        inline int getMGPawnShieldSizeWeight(int size) const { return size == 0 ? 0 : mgPawnShieldSizeWeight[size - 1]; }
        inline int getMGKingOpenFileWeight(int numFiles) const { return numFiles == 0 ? 0 : mgKingOpenFileWeight[numFiles - 1]; }
        inline int getMGPawnStormWeight(int rank) const { return mgPawnStormWeight[rank - 1]; }

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

        inline int getOppositeColorBishopsEndgameWinnablePenalty() const { return oppositeColorBishopsEndgameWinnablePenalty; }

        inline int getMGOppositeColorBishopsWinnablePenalty() const { return mgOppositeColorBishopsWinnablePenalty; }
        inline int getEGOppositeColorBishopsWinnablePenalty() const { return egOppositeColorBishopsWinnablePenalty; }

        inline int getRookEndgameWinnablePenalty() const { return rookEndgameWinnablePenalty; }

        inline int getMGDefaultWinnablePenalty() const { return mgDefaultWinnablePenalty; }
        inline int getEGDefaultWinnablePenalty() const { return egDefaultWinnablePenalty; }

        inline int getKingAndPawnEndgameWinnableBonus() const { return kingAndPawnEndgameWinnableBonus; }

        inline int getMGPawnWinnableBonus() const { return mgPawnWinnableBonus; }
        inline int getEGPawnWinnableBonus() const { return egPawnWinnableBonus; }

        inline int getMGPassedPawnWinnableBonus() const { return mgPassedPawnWinnableBonus; }
        inline int getEGPassedPawnWinnableBonus() const { return egPassedPawnWinnableBonus; }

        inline int getKingInfiltrationWinnableBonus() const { return kingInfiltrationWinnableBonus; }

        inline int getMopupBaseBonus() const { return egMopupBaseBonus; }
        inline int getMopupProgressBonus() const { return egMopupProgressBonus; }
};

extern HCEParameters HCE_PARAMS;

#endif