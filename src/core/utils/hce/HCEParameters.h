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

        int32_t MG_PIECE_VALUE[5];

        int32_t EG_PIECE_VALUE[5];

        /**
         * Faktoren für die Skalierung des Materialwertes
         * bei ungleichen Materialverhältnissen als dreieckige Tabelle.
         */

        int32_t PAWN_IMBALANCE_FACTOR[45]; // 9 x 9

        int32_t KNIGHT_IMBALANCE_FACTOR[6]; // 3 x 3

        int32_t BISHOP_IMBALANCE_FACTOR[6]; // 3 x 3

        int32_t ROOK_IMBALANCE_FACTOR[6]; // 3 x 3

        int32_t QUEEN_IMBALANCE_FACTOR[3]; // 2 x 2

        /**
         * Positionstabellen für alle Figuren im Mittel- und Endspiel.
         */

        int32_t MG_PSQT[6][64];
        int32_t EG_PSQT[6][64];

        /**
         * Bonus für das Zugrecht im Mittel- und Endspiel.
         */

        int32_t MG_TEMPO_BONUS;
        int32_t EG_TEMPO_BONUS;

        /**
         * Bewertungen für verschiedene Merkmale in
         * Bauernstrukturen im Mittel- und Endspiel.
         */

        int32_t MG_CONNECTED_PAWN_BONUS[6]; // pro Rang (2 - 7)
        int32_t EG_CONNECTED_PAWN_BONUS[6]; // pro Rang (2 - 7)
        int32_t MG_DOUBLED_PAWN_PENALTY[8]; // pro Linie
        int32_t EG_DOUBLED_PAWN_PENALTY[8]; // pro Linie
        int32_t MG_ISOLATED_PAWN_PENALTY[8]; // pro Linie
        int32_t EG_ISOLATED_PAWN_PENALTY[8]; // pro Linie
        int32_t MG_BACKWARD_PAWN_PENALTY[6]; // pro Rang (2 - 7)
        int32_t EG_BACKWARD_PAWN_PENALTY[6]; // pro Rang (2 - 7)
        int32_t MG_PASSED_PAWN_BONUS[6]; // pro Rang (2 - 7)
        int32_t EG_PASSED_PAWN_BONUS[6]; // pro Rang (2 - 7)

        /**
         * Bewertungen für starke Felder im Mittel- und Endspiel.
         * Starke Felder sind Felder, die von einem eigenen Bauern
         * gedeckt sind und nie von einem gegnerischen Bauern
         * angegriffen werden können.
         */

        int32_t MG_STRONG_SQUARE_BONUS[5][8]; // pro Rang (3 - 7) und Linie
        int32_t EG_STRONG_SQUARE_BONUS[5][8]; // pro Rang (3 - 7) und Linie

        /**
         * Ein Bonus für jedes sichere Feld
         * in der Mitte des Spielfeldes.
         */

        int32_t MG_SPACE_BONUS;

        /**
         * Ein Bonus für jedes Feld im Königsbereich des Gegners,
         * dass von einer eigenen Figur angegriffen wird.
         */

        int32_t MG_NUM_ATTACKER_WEIGHT[5];

        int32_t EG_NUM_ATTACKER_WEIGHT[5];

        int32_t MG_KNIGHT_ATTACK_BONUS;
        int32_t EG_KNIGHT_ATTACK_BONUS;
        int32_t MG_BISHOP_ATTACK_BONUS;
        int32_t EG_BISHOP_ATTACK_BONUS;
        int32_t MG_ROOK_ATTACK_BONUS;
        int32_t EG_ROOK_ATTACK_BONUS;
        int32_t MG_QUEEN_ATTACK_BONUS;
        int32_t EG_QUEEN_ATTACK_BONUS;

        /**
         * Ein Bonus für jede Leichtfigur, die den eigenen
         * Königsbereich verteidigt.
         */

        int32_t MG_MINOR_PIECE_DEFENDER_BONUS;
        int32_t EG_MINOR_PIECE_DEFENDER_BONUS;

        /**
         * Bewertungen für verschiedene Merkmale in
         * Zusammenhang mit der Königssicherheit.
         */

        int32_t MG_PAWN_SHIELD_SIZE_BONUS[4];
        int32_t EG_PAWN_SHIELD_SIZE_BONUS[4];

        int32_t MG_KING_OPEN_FILE_PENALTY[4];
        int32_t EG_KING_OPEN_FILE_PENALTY[4];

        int32_t MG_PAWN_STORM_BONUS[6]; // pro Rang (2 - 7)
        int32_t EG_PAWN_STORM_BONUS[6]; // pro Rang (2 - 7)

        /**
         * Bewertungen für die Mobilität der Figuren
         * im Mittel- und Endspiel. Die Mobilität ist
         * die Anzahl der, im nächsten Zug, erreichbaren
         * Felder, die nicht von gegnerischen Bauern
         * angegriffen werden.
         * (Springer, Läufer, Turm, Dame)
         */

        int32_t MG_PIECE_MOBILITY_BONUS[4];
        int32_t EG_PIECE_MOBILITY_BONUS[4];

        /**
         * Ein Bonus für jede Leichtfigur,
         * die auf einem starken Feld steht oder
         * im nächsten Zug erreichen kann.
         */

        int32_t MG_MINOR_PIECE_ON_STRONG_SQUARE_BONUS;
        int32_t EG_MINOR_PIECE_ON_STRONG_SQUARE_BONUS;

        /**
         * Ein Bonus für das Läuferpaar im Mittel- und Endspiel.
         */

        int32_t MG_BISHOP_PAIR_BONUS;
        int32_t EG_BISHOP_PAIR_BONUS;

        /**
         * Ein Bonus für jede offene oder halboffene
         * Linie, auf der sich ein Turm befindet.
         */

        int32_t MG_ROOK_ON_OPEN_FILE_BONUS;
        int32_t EG_ROOK_ON_OPEN_FILE_BONUS;
        int32_t MG_ROOK_ON_SEMI_OPEN_FILE_BONUS;
        int32_t EG_ROOK_ON_SEMI_OPEN_FILE_BONUS;

        /**
         * Ein Bonus für jeden Freibauern, der von
         * einem Turm verteidigt/blockiert wird.
         */

        int32_t MG_ROOK_BEHIND_PASSED_PAWN_BONUS;
        int32_t EG_ROOK_BEHIND_PASSED_PAWN_BONUS;

        /**
         * Gewichtungsparameter für die Bewertung der Distanz
         * des Königs zu Bauern im Endspiel.
         */

        int32_t EG_KING_PROXIMITY_PAWN_WEIGHT;
        int32_t EG_KING_PROXIMITY_BACKWARD_PAWN_WEIGHT;
        int32_t EG_KING_PROXIMITY_PASSED_PAWN_WEIGHT;

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

        int32_t& operator[](size_t index);
        int32_t operator[](size_t index) const;

        static constexpr size_t size() { return sizeof(HCEParameters) / sizeof(int32_t); }

        static constexpr int32_t VALUE_ONE = 8;

        /**
         * @brief Dieses Objekt enthält einige wenige Parameter,
         * die nie abgerufen werden (Positionstabellen für Bauern
         * auf der ersten und letzten Reihe). Diese Methode prüft,
         * ob ein Parameter an einem bestimmten Index tot ist.
         */
        bool isParameterDead(size_t index) const;

        inline int32_t getMGPieceValue(int32_t piece) const { return MG_PIECE_VALUE[piece - 1]; }
        inline int32_t getEGPieceValue(int32_t piece) const { return EG_PIECE_VALUE[piece - 1]; }

        inline int32_t getEGWinningMaterialAdvantage() const { return (EG_PIECE_VALUE[3] - EG_PIECE_VALUE[2]) / 2 + EG_PIECE_VALUE[2]; }

        inline int32_t getPawnImbalanceFactor(int32_t numWhitePawns, int32_t numBlackPawns) const {
            int32_t high = std::max(numWhitePawns, numBlackPawns) + 1;
            int32_t low = std::min(numWhitePawns, numBlackPawns);

            high = std::min(high, 9);
            low = std::min(low, 8);

            return PAWN_IMBALANCE_FACTOR[high * (high - 1) / 2 + low];
        }

        inline int32_t getKnightImbalanceFactor(int32_t numWhiteKnights, int32_t numBlackKnights) const {
            int32_t high = std::max(numWhiteKnights, numBlackKnights) + 1;
            int32_t low = std::min(numWhiteKnights, numBlackKnights);

            high = std::min(high, 3);
            low = std::min(low, 2);

            return KNIGHT_IMBALANCE_FACTOR[high * (high - 1) / 2 + low];
        }

        inline int32_t getBishopImbalanceFactor(int32_t numWhiteBishops, int32_t numBlackBishops) const {
            int32_t high = std::max(numWhiteBishops, numBlackBishops) + 1;
            int32_t low = std::min(numWhiteBishops, numBlackBishops);

            high = std::min(high, 3);
            low = std::min(low, 2);

            return BISHOP_IMBALANCE_FACTOR[high * (high - 1) / 2 + low];
        }

        inline int32_t getRookImbalanceFactor(int32_t numWhiteRooks, int32_t numBlackRooks) const {
            int32_t high = std::max(numWhiteRooks, numBlackRooks) + 1;
            int32_t low = std::min(numWhiteRooks, numBlackRooks);

            high = std::min(high, 3);
            low = std::min(low, 2);

            return ROOK_IMBALANCE_FACTOR[high * (high - 1) / 2 + low];
        }

        inline int32_t getQueenImbalanceFactor(int32_t numWhiteQueens, int32_t numBlackQueens) const {
            int32_t high = std::max(numWhiteQueens, numBlackQueens) + 1;
            int32_t low = std::min(numWhiteQueens, numBlackQueens);

            high = std::min(high, 2);
            low = std::min(low, 1);

            return QUEEN_IMBALANCE_FACTOR[high * (high - 1) / 2 + low];
        }

        inline int32_t getMGPSQT(int32_t piece, int32_t square) const { return MG_PSQT[piece - 1][square]; }
        inline int32_t getEGPSQT(int32_t piece, int32_t square) const { return EG_PSQT[piece - 1][square]; }

        inline int32_t getMGTempoBonus() const { return MG_TEMPO_BONUS; }
        inline int32_t getEGTempoBonus() const { return EG_TEMPO_BONUS; }

        inline int32_t getMGConnectedPawnBonus(int32_t rank) const { return MG_CONNECTED_PAWN_BONUS[rank - 1]; }
        inline int32_t getEGConnectedPawnBonus(int32_t rank) const { return EG_CONNECTED_PAWN_BONUS[rank - 1]; }

        inline int32_t getMGDoubledPawnPenalty(int32_t file) const { return MG_DOUBLED_PAWN_PENALTY[file]; }
        inline int32_t getEGDoubledPawnPenalty(int32_t file) const { return EG_DOUBLED_PAWN_PENALTY[file]; }

        inline int32_t getMGIsolatedPawnPenalty(int32_t file) const { return MG_ISOLATED_PAWN_PENALTY[file]; }
        inline int32_t getEGIsolatedPawnPenalty(int32_t file) const { return EG_ISOLATED_PAWN_PENALTY[file]; }

        inline int32_t getMGBackwardPawnPenalty(int32_t rank) const { return MG_BACKWARD_PAWN_PENALTY[rank - 1]; }
        inline int32_t getEGBackwardPawnPenalty(int32_t rank) const { return EG_BACKWARD_PAWN_PENALTY[rank - 1]; }

        inline int32_t getMGPassedPawnBonus(int32_t rank) const { return MG_PASSED_PAWN_BONUS[rank - 1]; }
        inline int32_t getEGPassedPawnBonus(int32_t rank) const { return EG_PASSED_PAWN_BONUS[rank - 1]; }

        inline int32_t getMGStrongSquareBonus(int32_t rank, int32_t file) const { return rank == 7 ? 0 : MG_STRONG_SQUARE_BONUS[rank - 2][file]; }
        inline int32_t getEGStrongSquareBonus(int32_t rank, int32_t file) const { return rank == 7 ? 0 : EG_STRONG_SQUARE_BONUS[rank - 2][file]; }

        inline int32_t getMGSpaceBonus() const { return MG_SPACE_BONUS; }

        inline int32_t getMGNumAttackerWeight(int32_t numAttackers) const { return numAttackers == 0 ? 0 : MG_NUM_ATTACKER_WEIGHT[numAttackers - 1]; }
        inline int32_t getEGNumAttackerWeight(int32_t numAttackers) const { return numAttackers == 0 ? 0 : EG_NUM_ATTACKER_WEIGHT[numAttackers - 1]; }

        inline int32_t getMGKnightAttackBonus() const { return MG_KNIGHT_ATTACK_BONUS; }
        inline int32_t getEGKnightAttackBonus() const { return EG_KNIGHT_ATTACK_BONUS; }
        inline int32_t getMGBishopAttackBonus() const { return MG_BISHOP_ATTACK_BONUS; }
        inline int32_t getEGBishopAttackBonus() const { return EG_BISHOP_ATTACK_BONUS; }
        inline int32_t getMGRookAttackBonus() const { return MG_ROOK_ATTACK_BONUS; }
        inline int32_t getEGRookAttackBonus() const { return EG_ROOK_ATTACK_BONUS; }
        inline int32_t getMGQueenAttackBonus() const { return MG_QUEEN_ATTACK_BONUS; }
        inline int32_t getEGQueenAttackBonus() const { return EG_QUEEN_ATTACK_BONUS; }

        inline int32_t getMGMinorPieceDefenderBonus() const { return MG_MINOR_PIECE_DEFENDER_BONUS; }
        inline int32_t getEGMinorPieceDefenderBonus() const { return EG_MINOR_PIECE_DEFENDER_BONUS; }

        inline int32_t getMGPawnShieldSizeBonus(int32_t size) const { return MG_PAWN_SHIELD_SIZE_BONUS[size]; }
        inline int32_t getEGPawnShieldSizeBonus(int32_t size) const { return EG_PAWN_SHIELD_SIZE_BONUS[size]; }
        inline int32_t getMGKingOpenFilePenalty(int32_t numFiles) const { return MG_KING_OPEN_FILE_PENALTY[numFiles]; }
        inline int32_t getEGKingOpenFilePenalty(int32_t numFiles) const { return EG_KING_OPEN_FILE_PENALTY[numFiles]; }
        inline int32_t getMGPawnStormBonus(int32_t rank) const { return MG_PAWN_STORM_BONUS[rank - 1]; }
        inline int32_t getEGPawnStormBonus(int32_t rank) const { return EG_PAWN_STORM_BONUS[rank - 1]; }

        inline int32_t getMGPieceMobilityBonus(int32_t piece) const { return MG_PIECE_MOBILITY_BONUS[piece - 2]; }
        inline int32_t getEGPieceMobilityBonus(int32_t piece) const { return EG_PIECE_MOBILITY_BONUS[piece - 2]; }

        inline int32_t getMGMinorPieceOnStrongSquareBonus() const { return MG_MINOR_PIECE_ON_STRONG_SQUARE_BONUS; }
        inline int32_t getEGMinorPieceOnStrongSquareBonus() const { return EG_MINOR_PIECE_ON_STRONG_SQUARE_BONUS; }

        inline int32_t getMGBishopPairBonus() const { return MG_BISHOP_PAIR_BONUS; }
        inline int32_t getEGBishopPairBonus() const { return EG_BISHOP_PAIR_BONUS; }

        inline int32_t getMGRookOnOpenFileBonus() const { return MG_ROOK_ON_OPEN_FILE_BONUS; }
        inline int32_t getEGRookOnOpenFileBonus() const { return EG_ROOK_ON_OPEN_FILE_BONUS; }
        inline int32_t getMGRookOnSemiOpenFileBonus() const { return MG_ROOK_ON_SEMI_OPEN_FILE_BONUS; }
        inline int32_t getEGRookOnSemiOpenFileBonus() const { return EG_ROOK_ON_SEMI_OPEN_FILE_BONUS; }

        inline int32_t getMGRookBehindPassedPawnBonus() const { return MG_ROOK_BEHIND_PASSED_PAWN_BONUS; }
        inline int32_t getEGRookBehindPassedPawnBonus() const { return EG_ROOK_BEHIND_PASSED_PAWN_BONUS; }

        inline int32_t getEGKingProximityPawnWeight() const { return EG_KING_PROXIMITY_PAWN_WEIGHT; }
        inline int32_t getEGKingProximityBackwardPawnWeight() const { return EG_KING_PROXIMITY_BACKWARD_PAWN_WEIGHT; }
        inline int32_t getEGKingProximityPassedPawnWeight() const { return EG_KING_PROXIMITY_PASSED_PAWN_WEIGHT; }
};

extern HCEParameters HCE_PARAMS;

#endif