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
            124, 158, 281, 588, 461,
            -6, 1, 46, 48, 398,
            -10, -15, 15, 2, 38,
            59, 15, 51, 74, 159
        };

        /**
         * Faktoren für die Funktion zur Berechnung
         * des Materialungleichgewichts.
         */

        int16_t pieceImbalanceValues[10] = {
            -14, -15, -26, -19, -70,
            -40, 6, -68, 1, 144
        };

        /**
         * Positionstabellen für alle Figuren im Mittel- und Endspiel.
         */

        int16_t mgPSQTPawn[64] = {
            0,   0,   0,   0,   0,   0,   0,   0,
            -31, -19, -24, -36, -24,  28,  20,  -8,
            -23, -30, -25, -24, -19,   0,   5,  -3,
            -15, -12,  -8,   6,  11,  -2, -18, -10,
            -18,   2,  -4,  14,  26,  11,  -7, -17,
            30,  56,  46,  46,  89, 122,  91,  79,
            128,  59,  80, 128,  36, -25,  16,  80,
            0,   0,   0,   0,   0,   0,   0,   0,
        };

        int16_t mgPSQTPacked[5][32] = {
            {
                -57, -24, -23,   0,
                -17, -37,  18,  17,
                -9,  19,  35,  36,
                -6,  30,  51,  48,
                61,  28,  47,  55,
                38,  17,  58,  47,
                28,  -6,  87,  85,
                -48, -17, -76,  82,
            },
            {
                -22, -13,   2, -14,
                -8,  31,  48,  21,
                6,  38,  21,  43,
                -1,  -9,  19,  43,
                0,   6,  22,  36,
                10,  29,  24,  37,
                -28, -23,  23, -28,
                -56, -31, -60, -106,
            },
            {
                 -4,  -7,   9,  25,
                -52, -24, -14, -23,
                -38, -21, -35, -17,
                -19,  -9,  -4,   3,
                -10,  -9,  20,  11,
                -3,  37,  44,  99,
                54,  40,  56,  66,
                108,  94,  39,  76,
            },
            {
                 21, -26, -13,  31,
                -52,  29,  43,  34,
                8,  38,  32,  44,
                32,  32,  33,  55,
                29,  47,  49,  57,
                13,  65,  52,  68,
                55,  24,  85,  58,
                51,  34,  46,  79,
            },
            {
                 50,  62, -17, -17,
                58,  54,  -8, -21,
                -44,  48,  -8, -21,
                -103, -55, -18, -27,
                -130, -79, -63,  60,
                -106, -41, -48,  -3,
                -16, -38, -40, -19,
                -77,  32,  -7,  32,
            }
        };

        int16_t egPSQTPawn[64] = {
            0,   0,   0,   0,   0,   0,   0,   0,
            58,  67,  23,  -5,  32,  28,  15,  19,
            45,  48,  14,  25,  13,  12,  19,  13,
            49,  33,  29,  23,  26,  17,  18,  12,
            73,  60,  38,  29,  18,  12,  26,  19,
            87,  91,  49,  64,  44,  66,  65,  66,
            122, 154, 149, 148, 145, 159, 159, 114,
            0,   0,   0,   0,   0,   0,   0,   0,
        };

        int16_t egPSQTPacked[5][32] = {
            {
                70, 113,  53,  51,
                69,  59,  29,  66,
                75,  47,  45,  80,
                93,  47,  65,  79,
                58,  67,  68,  79,
                54,  91,  64,  70,
                125,  99,  64,  43,
                14, 100,  85,  59,
            },
            {
                83,  59,  74,  85,
                70,  80,  34,  69,
                51,  46,  63,  70,
                41,  53,  48,  53,
                69,  89,  76,  42,
                81,  74,  51,  41,
                108,  67,  56,  91,
                113,  72,  69,  99,
            },
            {
                64,  74,  65,  54,
                51,  66,  84,  63,
                76,  85,  86,  75,
                86, 112,  85,  91,
                113, 123,  99,  91,
                123, 127, 111,  93,
                99, 111, 100,  98,
                75, 101, 104,  85,
            },
            {
                -5,  12,   4,  -3,
                72,   3, -17,   9,
                32,  54,  65,  51,
                55,  96,  54,  65,
                128, 117,  74,  93,
                169,  76, 149, 118,
                119, 115,  86, 144,
                109, 108, 105, 100,
            },
            {
                -18,  -7,   3, -28,
                2,   8,  -1,  -7,
                2, -14,   3,  -2,
                -13,  15,   6,   7,
                26,  48,  43,  20,
                47,  91,  59,  33,
                36,  87,  59,  35,
                -142,  29,  48,  6,
            }
        };

        /**
         * Bonus für das Zugrecht im Mittel- und Endspiel.
         */

        int16_t mgTempoBonus = 36;
        int16_t egTempoBonus = 22;

        /**
         * Bonus für angegriffene Figuren im Mittel- und Endspiel.
         * Angriffe auf kleinere Figuren, die von einem
         * Bauern gedeckt werden, werden nicht berücksichtigt.
         */

        int16_t mgAttackByMinorPieceBonus[5] = {
            6, 24, 52, 95, 38
        };

        int16_t mgAttackByRookBonus[5] = {
            7, 55, 63, 43, 66
        };

        int16_t egAttackByMinorPieceBonus[5] = {
            43, 59, 32, 76, 43
        };

        int16_t egAttackByRookBonus[5] = {
            48, 59, 61, 8, 3
        };

        /**
         * Bewertungen für verschiedene Merkmale in
         * Bauernstrukturen im Mittel- und Endspiel.
         */

        int16_t mgConnectedPawnBonus[6] = {
            9, 12, 11, 23, 50, 92
        }; // pro Rang (2 - 7)
        int16_t egConnectedPawnBonus[6] = {
            -1, 2, 6, 17, 29, 68
        }; // pro Rang (2 - 7)
        int16_t mgDoubledPawnPenalty[4] = {
            -9, -14, -6, -13
        }; // pro Linie (1 - 4, gespiegelt)
        int16_t egDoubledPawnPenalty[4] = {
            -24, -31, 5, 2
        }; // pro Linie (1 - 4, gespiegelt)
        int16_t mgIsolatedPawnPenalty[4] = {
            -11, -2, -11, -20
        }; // pro Linie (1 - 4, gespiegelt)
        int16_t egIsolatedPawnPenalty[4] = {
            -13, -22, -14, -20
        }; // pro Linie (1 - 4, gespiegelt)
        int16_t mgBackwardPawnPenalty[5] = {
            -14, -31, -15, -9, 6
        }; // pro Rang (2 - 6)
        int16_t egBackwardPawnPenalty[5] = {
            -5, -26, -33, -56, 15
        }; // pro Rang (2 - 6)
        int16_t mgPassedPawnBonus[6] = {
            18, 7, 26, 69, 95, 148
        }; // pro Rang (2 - 7)
        int16_t egPassedPawnBonus[6] = {
            22, 28, 64, 110, 149, 219
        }; // pro Rang (2 - 7)
        int16_t mgCandidatePassedPawnBonus[5] = {
            17, 20, 15, 42, 56
        }; // pro Rang (2 - 6)
        int16_t egCandidatePassedPawnBonus[5] = {
            -7, 17, 25, 37, 32
        }; // pro Rang (2 - 6)
        int16_t mgConnectedPassedPawnBonus[6] = {
            -21, 11, -4, 2, -7, -5
        }; // pro Rang (2 - 7)
        int16_t egConnectedPassedPawnBonus[6] = {
            43, -5, 1, -5, 28, 32
        }; // pro Rang (2 - 7)

        /**
         * Bewertungen für starke Felder im Mittelspiel.
         * Starke Felder sind Felder, die von einem eigenen Bauern
         * gedeckt sind und nie von einem gegnerischen Bauern
         * angegriffen werden können.
         */

        int16_t mgCenterOutpostBonus = -2; // Linien C - F
        int16_t mgEdgeOutpostBonus = -4; // Linien A - B und G - H

        /**
         * Bewertungen für die Sicherheit des Königs im Mittel- und Endspiel.
         */

        int16_t mgAttackWeight[4] = {
            3, 4, 1, 0
        }; // Gewichtung der Angreifer (Springer, Läufer, Turm, Dame)
        int16_t egAttackWeight[4] = {
            -6, -11, -2, 0
        }; // Gewichtung der Angreifer (Springer, Läufer, Turm, Dame)
        int16_t mgUndefendedAttackWeight[4] = {
            0, 6, 3, 8
        }; // Gewichtung der unverteidigten Angriffe (Springer, Läufer, Turm, Dame)
        int16_t egUndefendedAttackWeight[4] = {
            -3, -2, 3, -5
        }; // Gewichtung der unverteidigten Angriffe (Springer, Läufer, Turm, Dame)
        int16_t mgSafeCheckWeight[4] = {
            19, 11, 17, 20
        }; // (Springer, Läufer, Turm, Dame)
        int16_t egSafeCheckWeight[4] = {
            -1, 9, 12, 16
        }; // (Springer, Läufer, Turm, Dame)
        int16_t mgSafeContactCheckWeight[2] = {
            18, 14
        }; // (Turm, Dame)
        int16_t egSafeContactCheckWeight[2] = {
            19, 6
        }; // (Turm, Dame)
        int16_t mgDefenseWeight[4] = {
            1, 1, -1, -1
        }; // Gewichtung der Verteidiger (Springer, Läufer, Turm, Dame)
        int16_t egDefenseWeight[4] = {
            -5, 1, -1, 14
        }; // Gewichtung der Verteidiger (Springer, Läufer, Turm, Dame)
        int16_t mgPawnShieldSizeWeight[3] = {
            9, 2, 5
        }; // Gewichtung der Größe des Bauernschildes (defensiv)
        int16_t mgKingOpenFileWeight[3] = {
            6, 9, 8
        }; // Gewichtung der offenen Linien (offensiv)
        int16_t mgPawnStormWeight[5] = {
            -5, -11, -4, 6, -5
        }; // pro Rang (2 - 6)

        /**
         * Bewertungen für die Mobilität der Figuren
         * im Mittel- und Endspiel.
         * (Springer, Läufer, Turm, Dame)
         */

        int16_t mgPieceMobilityBonus[4] = {
            5, 9, 8, 4
        };
        int16_t egPieceMobilityBonus[4] = {
            110, 60, 58, 135
        };

        int16_t mgPieceNoMobilityPenalty[4] = {
            -95, -92, -142, -185
        };
        int16_t egPieceNoMobilityPenalty[4] = {
            -43, -52, -58, -58
        };

        /**
         * Ein Bonus für jede Leichtfigur,
         * die auf einem starken Feld steht oder
         * im nächsten Zug erreichen kann.
         */

        int16_t mgKnightOnCenterOutpostBonus = 66;
        int16_t mgKnightOnEdgeOutpostBonus = 32;
        int16_t mgBishopOnCenterOutpostBonus = 57;
        int16_t mgBishopOnEdgeOutpostBonus = 31;

        /**
         * Eine Bestrafung für Läufer, die durch eigene
         * unbewegbare Bauern oder gegnerische, durch andere
         * Bauern gedeckte, Bauern blockiert werden.
         */

        int16_t mgBadBishopPenalty = -6;
        int16_t egBadBishopPenalty = -7;

        /**
         * Ein Bonus für einen viel besseren Läufer auf einer Farbe.
         */

        int16_t mgBishopDominanceBonus = 5;
        int16_t egBishopDominanceBonus = 3;

        /**
         * Ein Bonus für jede offene oder halboffene
         * Linie, auf der sich ein Turm befindet.
         */

        int16_t mgRookOnOpenFileBonus = 41;
        int16_t mgRookOnSemiOpenFileBonus = 27;

        /**
         * Ein Bonus für gedoppelte Türme auf
         * halboffenen oder offenen Linien.
         */
        int16_t mgDoubledRooksOnOpenFileBonus = 32;
        int16_t mgDoubledRooksOnSemiOpenFileBonus = 25;

        /**
         * Ein Bonus für jeden Freibauern, der von
         * einem Turm auf derselben Linie verteidigt wird.
         */

        int16_t egRookBehindPassedPawnBonus = 51;

        /**
         * @brief Bonus für jede Figur, die einen
         * gegnerischen Freibauern blockiert.
         */

        int16_t egBlockedEnemyPassedPawnBonus = 45;

        /**
         * @brief Bonus für jeden Angriff auf den
         * Promotionspfad eines Freibauern.
         */
        int16_t egAttackOnPassedPawnPathBonus = 3;

        /**
         * Gewichtungsparameter für die Bewertung der Distanz
         * des Königs zu Bauern im Endspiel.
         */

        int16_t egKingProximityPawnWeight = 7;
        int16_t egKingProximityBackwardPawnWeight = 2;
        int16_t egKingProximityPassedPawnWeight = 21;

        /**
         * @brief Bonus für Freibauern in einem Bauernendspiel,
         * die nicht mehr vom gegnerischen König erreicht werden können.
         */

        int16_t ruleOfTheSquareBonus = 447;

        /**
         * "Draw Conditions"
         */

        /**
         * @brief Bestrafung für ein Endspiel mit Läufern
         * unterschiedlicher Farbe. Die Bestrafung wird
         * der ansonsten führenden Seite angerechnet und
         * kann das Vorzeichen der Bewertung nicht ändern.
         */
        int16_t oppositeColorBishopsEndgameWinnablePenalty = -105;

        /**
         * @brief Bestrafung, wenn beide Seiten einen Läufer
         * auf unterschiedlichen Farben haben. Die Bestrafung
         * wird der ansonsten führenden Seite angerechnet und
         * kann das Vorzeichen der Bewertung nicht ändern.
         */

        int16_t mgOppositeColorBishopsWinnablePenalty = -21;
        int16_t egOppositeColorBishopsWinnablePenalty = -75;

        /**
         * @brief Bestrafung für ein Endspiel mit Türmen
         * und Bauern (und maximal einer weiteren Leichtfigur).
         * Die Bestrafung wird der ansonsten führenden Seite
         * angerechnet und kann das Vorzeichen der Bewertung nicht ändern.
         */
        int16_t rookEndgameWinnablePenalty = -45;

        /**
         * @brief Konstante Bestrafung Richtung 0 für
         * Mittelspiel und Endspiel. Die Bestrafung wird
         * der ansonsten führenden Seite angerechnet und
         * kann das Vorzeichen der Bewertung nicht ändern.
         */

        int16_t mgDefaultWinnablePenalty = -60;
        int16_t egDefaultWinnablePenalty = -90;

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
        int16_t egPawnWinnableBonus = 16;

        /**
         * @brief Bonus für jeden Freibauern(kandidat).
         * Der Bonus kann die Vorzeichen-erhaltenden
         * Bestrafungen abschwächen, aber nicht
         * die tatsächliche Bewertung erhöhen.
         */

        int16_t mgPassedPawnWinnableBonus = -1;
        int16_t egPassedPawnWinnableBonus = 7;

        /**
         * @brief Bonus für einen König, der
         * auf der gegnerischen Seite steht.
         * Der Bonus kann die Vorzeichen-erhaltenden
         * Bestrafungen abschwächen, aber nicht
         * die tatsächliche Bewertung erhöhen.
         */

        int16_t kingInfiltrationWinnableBonus = 2;

        /**
         * Parameter für die Mop-Up Evaluation.
         */

        int16_t egMopupBaseBonus = 716;
        int16_t egMopupProgressBonus = 115;

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

        inline int getMGBishopDominanceBonus() const { return mgBishopDominanceBonus; }
        inline int getEGBishopDominanceBonus() const { return egBishopDominanceBonus; }

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