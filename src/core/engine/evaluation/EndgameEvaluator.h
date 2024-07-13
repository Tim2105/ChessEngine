#ifndef ENDGAME_EVALUATOR_H
#define ENDGAME_EVALUATOR_H

#include "core/chess/Board.h"
#include "core/engine/evaluation/Evaluator.h"
#include "core/engine/evaluation/EvaluationDefinitons.h"
#include "core/utils/Bitboard.h"

/**
 * @brief Die Klasse BoardEvaluator ist für die statische Bewertung eines Spielfeldes zuständig.
 * Parameter können in den Dateien "EvaluationDefinitions.h" und "EvaluationDefinitions.cpp" angepasst werden.
 */
class EndgameEvaluator : public Evaluator {

    private:
        /**
         * @brief Die Methode findDoublePawns findet alle Bauern, die sich auf derselben Linie befinden.
         */
        Bitboard findDoublePawns(const Bitboard& ownPawns, int32_t side);

        /**
         * @brief Die Methode findIsolatedPawns findet alle isolierten Bauern.
         * Isolierte Bauern sind Bauern, die keine befreundeten Bauern auf der Linie links oder rechts von ihnen haben.
         */
        Bitboard findIsolatedPawns(const Bitboard& ownPawns);

        /**
         * @brief Die Methode findPassedPawns findet alle Bauern, die nicht von gegnerischen Bauern abgefangen werden können.
         */
        Bitboard findPassedPawns(const Bitboard& ownPawns, const Bitboard& otherPawns, int32_t side);

        /**
         * @brief Die Methode findPawnChains findet alle Bauernketten.
         * Bauernketten sind Bauern, die andere Bauern schützen.
         */
        Bitboard findPawnChains(const Bitboard& ownPawns, int32_t side);

        /**
         * @brief Die Methode findConnectedPawns findet alle verbundenen Bauern.
         * Verbundene Bauern sind Bauern, die sich nebeneinander auf demselben Rang befinden.
         */
        Bitboard findConnectedPawns(const Bitboard& ownPawns);

        /**
         * @brief Die Methode evalMaterial bewertet die Materialstärke der beiden Spieler im Endspiel.
         */
        int32_t evalEGMaterial();

        /**
         * @brief Die Methode evalMGPieces bewertet die Stärke der Figuren der beiden Spieler im Endspiel.
         */
        int32_t evalEGPieces();

        /**
         * @brief Die Methode evalEGKnights bewertet die Stärke der Springer der beiden Spieler im Endspiel.
        */
        inline int32_t evalEGKnights(const Bitboard& ownKnights, const Bitboard& pawns);

        /**
         * @brief Die Methode evalEGBishops bewertet die Stärke der Läufer der beiden Spieler im Endspiel.
        */
        inline int32_t evalEGBishops(const Bitboard& ownBishops);

        /**
         * @brief Die Methode evalEGRooks bewertet die Stärke der Türme der beiden Spieler im Endspiel.
         */
        inline int32_t evalEGRooks(const Bitboard& ownRooks, const Bitboard& ownPawns, const Bitboard& otherPawns,
                                   const Bitboard& ownPassedPawns, const Bitboard& otherPassedPawns, int32_t side);

        /**
         * @brief Die Methode evalEG_PSQT bewertet die Figurenpositionen der beiden Spieler
         * mit dem Endgame-PSQT-Array.
         */
        inline int32_t evalEG_PSQT();

        /**
         * @brief Die Methode evalPawnStructure bewertet die Bauernstruktur der beiden Spieler.
         */
        int32_t evalPawnStructure(int32_t side);

        inline int32_t evalDoublePawns(Bitboard doublePawns);
        inline int32_t evalIsolatedPawns(Bitboard isolatedPawns);
        inline int32_t evalPassedPawns(Bitboard passedPawns, const Bitboard& ownPawnAttacks, int32_t side);
        inline int32_t evalPawnChains(Bitboard pawnChains);
        inline int32_t evalConnectedPawns(Bitboard connectedPawns);

        /**
         * @brief Die Methode evalEGMobility bewertet die Mobilität der beiden Spieler.
         */
        int32_t evalEGMobility();

        /**
         * @brief Die Methode evalKingPawnEG bewertet die Stellung des Königs und der Bauern im Endspiel.
         */
        int32_t evalKingPawnEG();

        /**
         * @brief Überprüft, ob die Seite side einen Freibauern hat, der nicht mehr gestoppt werden kann.
         */
        inline int32_t evalEGRuleOfTheSquare(const Bitboard& ownPassedPawns, int32_t otherKingSquare, int32_t side, bool canMoveNext);

    public:
        EndgameEvaluator() = delete;

        EndgameEvaluator(Board& b) : Evaluator(b) {};

        ~EndgameEvaluator() {}

        EndgameEvaluator(const EndgameEvaluator& other) = delete;
        EndgameEvaluator& operator=(const EndgameEvaluator& other) = delete;

        EndgameEvaluator(EndgameEvaluator&& other);
        EndgameEvaluator& operator=(EndgameEvaluator&& other);

        /**
         * @brief Führt eine statische Bewertung für das Endgame der
         * Spielpositon aus der Sicht des Spielers der am Zug ist durch.
         * 
         * @param b Das aktuelle Spielfeld.
         * @return Die Bewertung der Spielposition.
         * Je größer der Wert, desto besser ist die Spielposition für den Spieler der am Zug ist.
         * Je kleiner der Wert, desto besser ist die Spielposition für den Gegner des Spielers der am Zug ist.
         * Eine Bewertung von 0 bedeutet ein ausgeglichenes Spiel.
         */
        int32_t endgameEvaluation();

        /**
         * @brief Führt eine statische Bewertung der
         * Spielpositon aus der Sicht des Spielers der am Zug ist durch.
         * Die Bewertung ist eine Interpolation zwischen der Midgame- und der Endgame-Bewertung.
         * 
         * @param b Das aktuelle Spielfeld.
         * @return Die Bewertung der Spielposition.
         * Je größer der Wert, desto besser ist die Spielposition für den Spieler der am Zug ist.
         * Je kleiner der Wert, desto besser ist die Spielposition für den Gegner des Spielers der am Zug ist.
         * Eine Bewertung von 0 bedeutet ein ausgeglichenes Spiel.
         */
        int evaluate() override;

    private:
        /**
         * @brief Hier werden Konstanten für die statische Evaluation definiert,
         * sodass diese leichter angepasst werden können.
         * 
         * Konstanten mit dem Präfix MG sind für das Midgame,
         * während Konstanten mit dem Präfix EG für das Endgame sind.
         */

        static constexpr int16_t SIMPLE_PIECE_VALUE[7] = {
            0, // Empty
            100, // Pawn
            300, // Knight
            300, // Bishop
            500, // Rook
            900, // Queen
            0 // King
        };

        static constexpr int16_t EG_PIECE_VALUE[7] = {
            0, // Empty
            150, // Pawn
            350, // Knight
            360, // Bishop
            530, // Rook
            950, // Queen
            0 // King
        };

        // Springer sind mehr Wert, wenn mehr Bauern auf dem Feld sind
        static constexpr int32_t KNIGHT_PAWN_VALUE = 2;

        // Türme sind mehr Wert, wenn weniger Bauern auf dem Feld sind
        static constexpr int32_t ROOK_CAPTURED_PAWN_VALUE = 3;

        // Bonus für das Läuferpaar
        static constexpr int32_t EG_BISHOP_PAIR_VALUE = 15;

        // Bonus für Türme auf offenen/halboffenen Linien
        static constexpr int32_t EG_ROOK_SEMI_OPEN_FILE_VALUE = 0;
        static constexpr int32_t EG_ROOK_OPEN_FILE_VALUE = 0;

        // Bonus für Türme die eigene Freibauern von hinten decken
        static constexpr int32_t EG_ROOK_SUPPORTING_PASSED_PAWN_VALUE = 35;

        // Bonus für Türme die gegnerische Freibauern von hinten decken
        static constexpr int32_t EG_ROOK_BLOCKING_PASSED_PAWN_VALUE = 35;

        /**
         * @brief König und Bauern Endspiel
         */
        
        // Bonus für einen unaufhaltbaren Freibauern
        static constexpr int32_t EG_UNSTOPPABLE_PAWN_VALUE = 750;

        static constexpr Bitboard fileFacingEnemy[2][64] = {
                // White
                {
                        0x101010101010100,0x202020202020200,0x404040404040400,0x808080808080800,0x1010101010101000,0x2020202020202000,0x4040404040404000,0x8080808080808000,
                        0x101010101010000,0x202020202020000,0x404040404040000,0x808080808080000,0x1010101010100000,0x2020202020200000,0x4040404040400000,0x8080808080800000,
                        0x101010101000000,0x202020202000000,0x404040404000000,0x808080808000000,0x1010101010000000,0x2020202020000000,0x4040404040000000,0x8080808080000000,
                        0x101010100000000,0x202020200000000,0x404040400000000,0x808080800000000,0x1010101000000000,0x2020202000000000,0x4040404000000000,0x8080808000000000,
                        0x101010000000000,0x202020000000000,0x404040000000000,0x808080000000000,0x1010100000000000,0x2020200000000000,0x4040400000000000,0x8080800000000000,
                        0x101000000000000,0x202000000000000,0x404000000000000,0x808000000000000,0x1010000000000000,0x2020000000000000,0x4040000000000000,0x8080000000000000,
                        0x100000000000000,0x200000000000000,0x400000000000000,0x800000000000000,0x1000000000000000,0x2000000000000000,0x4000000000000000,0x8000000000000000,
                        0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                },
                // Black
                {
                        0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                        0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80,
                        0x101,0x202,0x404,0x808,0x1010,0x2020,0x4040,0x8080,
                        0x10101,0x20202,0x40404,0x80808,0x101010,0x202020,0x404040,0x808080,
                        0x1010101,0x2020202,0x4040404,0x8080808,0x10101010,0x20202020,0x40404040,0x80808080,
                        0x101010101,0x202020202,0x404040404,0x808080808,0x1010101010,0x2020202020,0x4040404040,0x8080808080,
                        0x10101010101,0x20202020202,0x40404040404,0x80808080808,0x101010101010,0x202020202020,0x404040404040,0x808080808080,
                        0x1010101010101,0x2020202020202,0x4040404040404,0x8080808080808,0x10101010101010,0x20202020202020,0x40404040404040,0x80808080808080,
                }
        };

        static constexpr Bitboard neighboringFiles[8] = {
                0x101010101010101, // A
                0x505050505050505, // B
                0xA0A0A0A0A0A0A0A, // C
                0x1414141414141414, // D
                0x2828282828282828, // E
                0x5050505050505050, // F
                0xA0A0A0A0A0A0A0A0, // G
                0x4040404040404040 // H
        };

        // Bonus für jeden Bauern, der neben mindestens einem anderen Bauern steht
        static constexpr Bitboard connectedPawnMasks[64] = {
                0x2,0x5,0xA,0x14,0x28,0x50,0xA0,0x40,
                0x200,0x500,0xA00,0x1400,0x2800,0x5000,0xA000,0x4000,
                0x20000,0x50000,0xA0000,0x140000,0x280000,0x500000,0xA00000,0x400000,
                0x2000000,0x5000000,0xA000000,0x14000000,0x28000000,0x50000000,0xA0000000,0x40000000,
                0x200000000,0x500000000,0xA00000000,0x1400000000,0x2800000000,0x5000000000,0xA000000000,0x4000000000,
                0x20000000000,0x50000000000,0xA0000000000,0x140000000000,0x280000000000,0x500000000000,0xA00000000000,0x400000000000,
                0x2000000000000,0x5000000000000,0xA000000000000,0x14000000000000,0x28000000000000,0x50000000000000,0xA0000000000000,0x40000000000000,
                0x200000000000000,0x500000000000000,0xA00000000000000,0x1400000000000000,0x2800000000000000,0x5000000000000000,0xA000000000000000,0x4000000000000000,
        };

        static constexpr int32_t EG_PAWN_CONNECTED_VALUE = 20;

        // Bonus für jeden Bauern, der mindestens einen anderen Bauern deckt
        static constexpr Bitboard pawnChainMasks[2][64] = {
                // White
                {
                        0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                        0x2,0x5,0xA,0x14,0x28,0x50,0xA0,0x40,
                        0x200,0x500,0xA00,0x1400,0x2800,0x5000,0xA000,0x4000,
                        0x20000,0x50000,0xA0000,0x140000,0x280000,0x500000,0xA00000,0x400000,
                        0x2000000,0x5000000,0xA000000,0x14000000,0x28000000,0x50000000,0xA0000000,0x40000000,
                        0x200000000,0x500000000,0xA00000000,0x1400000000,0x2800000000,0x5000000000,0xA000000000,0x4000000000,
                        0x20000000000,0x50000000000,0xA0000000000,0x140000000000,0x280000000000,0x500000000000,0xA00000000000,0x400000000000,
                        0x2000000000000,0x5000000000000,0xA000000000000,0x14000000000000,0x28000000000000,0x50000000000000,0xA0000000000000,0x40000000000000,
                },
                // Black
                {
                        0x200,0x500,0xA00,0x1400,0x2800,0x5000,0xA000,0x4000,
                        0x20000,0x50000,0xA0000,0x140000,0x280000,0x500000,0xA00000,0x400000,
                        0x2000000,0x5000000,0xA000000,0x14000000,0x28000000,0x50000000,0xA0000000,0x40000000,
                        0x200000000,0x500000000,0xA00000000,0x1400000000,0x2800000000,0x5000000000,0xA000000000,0x4000000000,
                        0x20000000000,0x50000000000,0xA0000000000,0x140000000000,0x280000000000,0x500000000000,0xA00000000000,0x400000000000,
                        0x2000000000000,0x5000000000000,0xA000000000000,0x14000000000000,0x28000000000000,0x50000000000000,0xA0000000000000,0x40000000000000,
                        0x200000000000000,0x500000000000000,0xA00000000000000,0x1400000000000000,0x2800000000000000,0x5000000000000000,0xA000000000000000,0x4000000000000000,
                        0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                }
        };

        static constexpr int32_t EG_PAWN_CHAIN_VALUE = 10;

        // Bestrafung für zwei oder mehrere Bauern in einer Spalte (doppelte Bauern)
        static constexpr int32_t EG_PAWN_DOUBLED_VALUE = -20;

        // Bestrafung für einen Bauern, der keine Nachbarn hat(keine Bauern in einer Nachbarspalte)
        static constexpr int32_t EG_PAWN_ISOLATED_VALUE = -10;

        // Bonus für jeden Freibauern(passed pawn)
        static constexpr Bitboard sentryMasks[2][64] = {
                // White
                {
                        0x303030303030300,0x707070707070700,0xE0E0E0E0E0E0E00,0x1C1C1C1C1C1C1C00,0x3838383838383800,0x7070707070707000,0xE0E0E0E0E0E0E000,0xC0C0C0C0C0C0C000,
                        0x303030303030000,0x707070707070000,0xE0E0E0E0E0E0000,0x1C1C1C1C1C1C0000,0x3838383838380000,0x7070707070700000,0xE0E0E0E0E0E00000,0xC0C0C0C0C0C00000,
                        0x303030303000000,0x707070707000000,0xE0E0E0E0E000000,0x1C1C1C1C1C000000,0x3838383838000000,0x7070707070000000,0xE0E0E0E0E0000000,0xC0C0C0C0C0000000,
                        0x303030300000000,0x707070700000000,0xE0E0E0E00000000,0x1C1C1C1C00000000,0x3838383800000000,0x7070707000000000,0xE0E0E0E000000000,0xC0C0C0C000000000,
                        0x303030000000000,0x707070000000000,0xE0E0E0000000000,0x1C1C1C0000000000,0x3838380000000000,0x7070700000000000,0xE0E0E00000000000,0xC0C0C00000000000,
                        0x303000000000000,0x707000000000000,0xE0E000000000000,0x1C1C000000000000,0x3838000000000000,0x7070000000000000,0xE0E0000000000000,0xC0C0000000000000,
                        0x300000000000000,0x700000000000000,0xE00000000000000,0x1C00000000000000,0x3800000000000000,0x7000000000000000,0xE000000000000000,0xC000000000000000,
                        0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                },
                // Black
                {
                        0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                        0x3,0x7,0xE,0x1C,0x38,0x70,0xE0,0xC0,
                        0x303,0x707,0xE0E,0x1C1C,0x3838,0x7070,0xE0E0,0xC0C0,
                        0x30303,0x70707,0xE0E0E,0x1C1C1C,0x383838,0x707070,0xE0E0E0,0xC0C0C0,
                        0x3030303,0x7070707,0xE0E0E0E,0x1C1C1C1C,0x38383838,0x70707070,0xE0E0E0E0,0xC0C0C0C0,
                        0x303030303,0x707070707,0xE0E0E0E0E,0x1C1C1C1C1C,0x3838383838,0x7070707070,0xE0E0E0E0E0,0xC0C0C0C0C0,
                        0x30303030303,0x70707070707,0xE0E0E0E0E0E,0x1C1C1C1C1C1C,0x383838383838,0x707070707070,0xE0E0E0E0E0E0,0xC0C0C0C0C0C0,
                        0x3030303030303,0x7070707070707,0xE0E0E0E0E0E0E,0x1C1C1C1C1C1C1C,0x38383838383838,0x70707070707070,0xE0E0E0E0E0E0E0,0xC0C0C0C0C0C0C0,
                }
        };

        static constexpr int32_t EG_PAWN_PASSED_BASE_VALUE = 24;

        // Wird mit der Anzahl der fortgeschrittenen Felder multipliziert
        static constexpr int32_t EG_PAWN_PASSED_RANK_ADVANCED_MULTIPLIER = 14;

        // Zusätzlicher Bonus für jeden Freibauern, der von einem anderen Bauern gedeckt wird
        static constexpr int32_t EG_PASSED_PAWN_PROTECTION_VALUE = 10;

        // Bonus für jedes Feld, dass von einer Figur angegriffen wird.
        // Felder, die von generischen Bauern angegriffen werden, werden ausgenommen.
        static constexpr int32_t EG_PAWN_MOBILITY_VALUE = 0;
        static constexpr int32_t EG_KNIGHT_MOBILITY_VALUE = 1;
        static constexpr int32_t EG_BISHOP_MOBILITY_VALUE = 2;
        static constexpr int32_t EG_ROOK_MOBILITY_VALUE = 2;
        static constexpr int32_t EG_QUEEN_MOBILITY_VALUE = 0;

        // Bestrafung für Figuren, die En Prise(ungeschützt) sind
        static constexpr int32_t EG_PIECE_EN_PRISE_VALUE = -2;

        /**
         * @brief Bewertung für die Distanz zwischen den Königen.
         * Wird addiert, wenn die eigene Farbe mehr Material(wie in EG_WINNING_MATERIAL_ADVANTAGE angegeben) besitzt und subtrahiert,
         * wenn die gegnerische Farbe mehr Material besitzt.
         */
        static constexpr int32_t EG_KING_DISTANCE_VALUE = 20;

        static constexpr int32_t EG_WINNING_MATERIAL_ADVANTAGE = 400;
};

#endif