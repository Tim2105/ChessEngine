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
            105, 400, 405, 615, 1200, // linear
            -2, -3, 15, -5, 15, // quadratisch
            // gekreuzt
            2, // Springer * Bauer
            0, 0, // Läufer * Bauer, Springer * Läufer
            0, 0, 0, // Turm * Bauer, Turm * Springer, Turm * Läufer
            0, 0, 0, 0 // Dame * Bauer, Dame * Springer, Dame * Läufer, Dame * Turm
        };

        /**
         * Positionstabellen für alle Figuren im Mittel- und Endspiel.
         */

        int16_t mgPSQT[6][64] = {
            // Pawn
            {
                    0, 0, 0, 0, 0, 0, 0, 0, // tote Parameterreihe
                    1, 1, -14, -15, -15, -13, 0, 1,
                    -3, -1, -16, -10, -5, -15, -1, -3,
                    5, 1, 14, 19, 20, 8, 0, 2,
                    9, 0, 2, 28, 28, 2, 0, 6,
                    7, 4, 5, 25, 25, 6, 5, 5,
                    12, 7, 9, 28, 28, 9, 10, 12,
                    0, 0, 0, 0, 0, 0, 0, 0, // tote Parameterreihe
            },
            // Knight
            {
                    -67, -18, -23, -26, -25, -20, -20, -64,
                    -37, -9, -11, -9, -7, -18, -18, -29,
                    -28, -1, 2, -6, -8, 5, -10, -27,
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
                    -45, -39, -8, 12, -15, -36, -51, -63,
                    -27, -35, 3, 0, -1, -7, -47, -40,
                    -8, -5, -5, -9, -12, -12, -7, -21,
                    -24, -19, -11, -34, -27, -10, -15, -21,
                    -20, -21, -25, -28, -24, -17, -19, -19,
                    -9, 11, 6, -8, -10, 0, 7, -9,
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

        int16_t egPSQT[6][64] = {
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

        int16_t mgTempoBonus = 20;
        int16_t egTempoBonus = 8;

        /**
         * Bewertungen für verschiedene Merkmale in
         * Bauernstrukturen im Mittel- und Endspiel.
         */

        int16_t mgConnectedPawnBonus = 5;
        int16_t egConnectedPawnBonus = 5;
        int16_t mgDoubledPawnPenalty = -6;
        int16_t egDoubledPawnPenalty = -10;
        int16_t mgIsolatedPawnPenalty = -3;
        int16_t egIsolatedPawnPenalty = -4;
        int16_t mgPawnIslandPenalty = -4;
        int16_t egPawnIslandPenalty = -8;
        int16_t mgBackwardPawnPenalty = -18;
        int16_t egBackwardPawnPenalty = -12;
        int16_t mgPassedPawnBonus[6] = {
            3, 3, 10, 17, 28, 35
        }; // pro Rang (2 - 7)
        int16_t egPassedPawnBonus[6] = {
            28, 28, 40, 58, 82, 110
        }; // pro Rang (2 - 7)

        /**
         * Bewertungen für starke Felder im Mittelspiel.
         * Starke Felder sind Felder, die von einem eigenen Bauern
         * gedeckt sind und nie von einem gegnerischen Bauern
         * angegriffen werden können.
         */

        int16_t mgStrongSquareBonus = 22;

        /**
         * Ein Bonus für jedes sichere Feld
         * in der Mitte des Spielfeldes.
         */

        int16_t mgSpaceBonus = 2;

        /**
         * Ein Bonus für jedes Feld im Königsbereich des Gegners,
         * dass von einer eigenen Figur angegriffen wird.
         */

        int16_t numAttackerWeight[5] = {
            0, 50, 75, 90, 100
        };

        int16_t knightAttackBonus = 12;
        int16_t bishopAttackBonus = 7;
        int16_t rookAttackBonus = 32;
        int16_t queenAttackBonus = 65;

        /**
         * Bewertungen für verschiedene Merkmale in
         * Zusammenhang mit der Königssicherheit.
         */

        int16_t mgPawnShieldSizeBonus[4] = {
            0, 27, 95, 110
        };

        int16_t mgKingOpenFilePenalty[4] = {
            0, -31, -76, -85
        };

        int16_t mgPawnStormPenalty[6] = {
            -2, -2, -10, -16, -25, -27
        }; // pro Rang (2 - 7)

        /**
         * Bewertungen für die Mobilität der Figuren
         * im Mittel- und Endspiel. Die Mobilität ist
         * die (gewurzelte doppelte) Anzahl der, im nächsten Zug,
         * erreichbaren Felder, die nicht von gegnerischen Bauern
         * angegriffen werden. Felder in der Mitte des Spielfeldes
         * oder in gegnerischem Territorium, die nicht verteidigt
         * werden, werden zusätzlich (linear) eingerechnet.
         * (Springer, Läufer, Turm, Dame)
         */

        int16_t mgPieceMobilityBonus[4] = {
            3, // Knight
            4, // Bishop
            4, // Rook
            0, // Queen
        };
        int16_t egPieceMobilityBonus[4] = {
            2, // Knight
            3, // Bishop
            3, // Rook
            1, // Queen
        };

        /**
         * Ein Bonus für jede Leichtfigur,
         * die auf einem starken Feld steht oder
         * im nächsten Zug erreichen kann.
         */

        int16_t mgMinorPieceOnStrongSquareBonus = 22;

        /**
         * Eine Bestrafung für Läufer, die durch eigene
         * unbewegbare Bauern oder gegnerische, durch andere
         * Bauern gedeckte, Bauern blockiert werden.
         */

        int16_t mgBadBishopPenalty = -25;
        int16_t egBadBishopPenalty = -6;

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

    public:
        HCEParameters();
        HCEParameters(std::istream& stream);

        ~HCEParameters();

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

        static constexpr size_t size() { return sizeof(HCEParameters) / sizeof(int16_t); }

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

        inline const int16_t& getLinearPieceValue(int32_t piece) const { return pieceValues[piece - 1]; }
        inline const int16_t& getQuadraticPieceValue(int32_t piece) const { return pieceValues[piece + 4]; }

        inline const int16_t& getCrossedPieceValue(int32_t piece1, int32_t piece2) const {
            int32_t minPiece = std::min(piece1, piece2) - 1;
            int32_t maxPiece = std::max(piece1, piece2) - 2;

            return pieceValues[10 + maxPiece * (maxPiece + 1) / 2 + minPiece];
        }

        inline int16_t getEGWinningMaterialAdvantage() const { return (pieceValues[3] - pieceValues[2]) / 2 + pieceValues[2]; }

        inline int16_t getMGPSQT(int32_t piece, int32_t square) const { return mgPSQT[piece - 1][square]; }
        inline int16_t getEGPSQT(int32_t piece, int32_t square) const { return egPSQT[piece - 1][square]; }

        inline int16_t getMGTempoBonus() const { return mgTempoBonus; }
        inline int16_t getEGTempoBonus() const { return egTempoBonus; }

        inline int16_t getMGConnectedPawnBonus() const { return mgConnectedPawnBonus; }
        inline int16_t getEGConnectedPawnBonus() const { return egConnectedPawnBonus; }

        inline int16_t getMGDoubledPawnPenalty() const { return mgDoubledPawnPenalty; }
        inline int16_t getEGDoubledPawnPenalty() const { return egDoubledPawnPenalty; }

        inline int16_t getMGIsolatedPawnPenalty() const { return mgIsolatedPawnPenalty; }
        inline int16_t getEGIsolatedPawnPenalty() const { return egIsolatedPawnPenalty; }

        inline int16_t getMGPawnIslandPenalty() const { return mgPawnIslandPenalty; }
        inline int16_t getEGPawnIslandPenalty() const { return egPawnIslandPenalty; }

        inline int16_t getMGBackwardPawnPenalty() const { return mgBackwardPawnPenalty; }
        inline int16_t getEGBackwardPawnPenalty() const { return egBackwardPawnPenalty; }

        inline int16_t getMGPassedPawnBonus(int32_t rank) const { return mgPassedPawnBonus[rank - 1]; }
        inline int16_t getEGPassedPawnBonus(int32_t rank) const { return egPassedPawnBonus[rank - 1]; }

        inline int16_t getMGStrongSquareBonus() const { return mgStrongSquareBonus; }

        inline int16_t getMGSpaceBonus() const { return mgSpaceBonus; }

        inline int16_t getNumAttackerWeight(int32_t numAttackers) const { return numAttackers == 0 ? 0 : numAttackerWeight[numAttackers - 1]; }

        inline int16_t getKnightAttackBonus() const { return knightAttackBonus; }
        inline int16_t getBishopAttackBonus() const { return bishopAttackBonus; }
        inline int16_t getRookAttackBonus() const { return rookAttackBonus; }
        inline int16_t getQueenAttackBonus() const { return queenAttackBonus; }

        inline int16_t getMGPawnShieldSizeBonus(int32_t size) const { return mgPawnShieldSizeBonus[size]; }
        inline int16_t getMGKingOpenFilePenalty(int32_t numFiles) const { return mgKingOpenFilePenalty[numFiles]; }
        inline int16_t getMGPawnStormPenalty(int32_t rank) const { return mgPawnStormPenalty[rank - 1]; }

        inline int16_t getMGPieceMobilityBonus(int32_t piece) const { return mgPieceMobilityBonus[piece - 2]; }
        inline int16_t getEGPieceMobilityBonus(int32_t piece) const { return egPieceMobilityBonus[piece - 2]; }

        inline int16_t getMGMinorPieceOnStrongSquareBonus() const { return mgMinorPieceOnStrongSquareBonus; }

        inline int16_t getMGBadBishopPenalty() const { return mgBadBishopPenalty; }
        inline int16_t getEGBadBishopPenalty() const { return egBadBishopPenalty; }

        inline int16_t getMGRookOnOpenFileBonus() const { return mgRookOnOpenFileBonus; }
        inline int16_t getMGRookOnSemiOpenFileBonus() const { return mgRookOnSemiOpenFileBonus; }

        inline int16_t getEGRookBehindPassedPawnBonus() const { return egRookBehindPassedPawnBonus; }
        inline int16_t getEGBlockedEnemyPassedPawnBonus() const { return egBlockedEnemyPassedPawnBonus; }
        inline int16_t getEGBlockedOwnPassedPawnPenalty() const { return egBlockedOwnPassedPawnPenalty; }

        inline int16_t getEGKingProximityPawnWeight() const { return egKingProximityPawnWeight; }
        inline int16_t getEGKingProximityBackwardPawnWeight() const { return egKingProximityBackwardPawnWeight; }
        inline int16_t getEGKingProximityPassedPawnWeight() const { return egKingProximityPassedPawnWeight; }
};

extern HCEParameters HCE_PARAMS;

#endif