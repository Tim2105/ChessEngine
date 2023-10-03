#ifndef STATIC_EVALUATOR_H
#define STATIC_EVALUATOR_H

#include "core/chess/Board.h"
#include "core/engine/EvaluationDefinitons.h"
#include "core/engine/Evaluator.h"
#include "core/utils/Bitboard.h"
#include "core/utils/tables/HashTable.h"
#include "core/utils/tables/HeapHashTable.h"

/**
 * @brief Die Klasse BoardEvaluator ist für die statische Bewertung eines Spielfeldes zuständig.
 * Parameter können in den Dateien "EvaluationDefinitions.h" und "EvaluationDefinitions.cpp" angepasst werden.
 */
class StaticEvaluator : public Evaluator {

    private:
        // Eine Hash-Tabelle, in der die Bewertung von Stellungen gespeichert wird,
        // um sie bei der nächsten Bewertung(bei gleicher Stellung) wieder zu verwenden.
        HeapHashTable<uint64_t, int32_t, 131072, 4> evaluationTable;

        // Die Bewertung von Bauernstrukturen ist sehr aufwendig,
        // weshalb berechnete Bewertungen von Bauernstrukturen in einer Hash-Tabelle gespeichert werden,
        // um sie bei der nächsten Bewertung(bei gleicher Bauernstruktur) wieder zu verwenden.
        // Weil Bauernstrukturen sich nicht zu häufig ändern, bekommt man hier eine hohe Trefferquote(Durchschnitt ca. 75%).
        HeapHashTable<uint64_t, Score, 8192, 4> pawnStructureTable;

        /**
         * @brief Überprüft ob eine Position anhand des Materials wahrscheinlich ein Unentschieden ist.
         * Die Methode wird false zurückgeben, solange mindestens ein Bauer auf dem Spielfeld ist.
         */
        bool isLikelyDraw();

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
         * @brief Die Methode evalMaterial bewertet die Materialstärke der beiden Spieler im Mittelspiel.
         */
        int32_t evalMGMaterial();

        /**
         * @brief Die Methode evalMaterial bewertet die Materialstärke der beiden Spieler im Endspiel.
         */
        int32_t evalEGMaterial();

        /**
         * @brief Die Methode evalMGPieces bewertet die Stärke der Figuren der beiden Spieler im Mittelspiel.
         */
        int32_t evalMGPieces();

        /**
         * @brief Die Methode evalMGKnights bewertet die Stärke der Springer der beiden Spieler im Mittelspiel.
        */
        inline int32_t evalMGKnights(const Bitboard& ownKnights, const Bitboard& pawns);

        /**
         * @brief Die Methode evalMGBishops bewertet die Stärke der Läufer der beiden Spieler im Mittelspiel.
        */
        inline int32_t evalMGBishops(const Bitboard& ownBishops, const Bitboard& otherBishops, const Bitboard& pawns);

        /**
         * @brief Die Methode evalMGRooks bewertet die Stärke der Türme der beiden Spieler im Mittelspiel.
         */
        inline int32_t evalMGRooks(const Bitboard& ownRooks, const Bitboard& ownPawns, const Bitboard& otherPawns, int32_t side);

        /**
         * @brief Die Methode evalMGQueens bewertet die Stärke der Damen der beiden Spieler im Mittelspiel.
         */
        inline int32_t evalMGQueens(const Bitboard& ownQueens, const Bitboard& ownKnights, const Bitboard& ownBishops, int32_t side);

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
         * @brief Die Methode evalMG_PSQT bewertet die Figurenpositionen der beiden Spieler
         * mit dem Midgame-PSQT-Array.
         */
        inline int32_t evalMG_PSQT();

        /**
         * @brief Die Methode evalEG_PSQT bewertet die Figurenpositionen der beiden Spieler
         * mit dem Endgame-PSQT-Array.
         */
        inline int32_t evalEG_PSQT();

        /**
         * @brief Versucht die Bewertung einer Stellung aus der Hash-Tabelle zu laden.
         */
        bool probeEvaluationTable(int32_t& score);

        /**
         * @brief Speichert die Bewertung einer Stellung in der Hash-Tabelle.
         */
        void storeEvaluationTable(int32_t score);

        /**
         * @brief Versucht die Bewertung einer Bauernstruktur aus der Hash-Tabelle zu laden.
         * 
         * @param b Das aktuelle Spielfeld.
         * @param score Hier wird die Bewertung der Bauernstruktur(falls gefunden) gespeichert.
         * @return true, wenn die Bewertung gefunden wurde.
         * @return false, wenn die Bewertung nicht gefunden wurde.
         */
        bool probePawnStructure(Score& score);

        /**
         * @brief Speichert die Bewertung einer Bauernstruktur in der Hash-Tabelle.
         * 
         * @param b Das aktuelle Spielfeld.
         * @param score Die Bewertung der Bauernstruktur.
         */
        void storePawnStructure(const Score& score);

        /**
         * @brief Die Methode evalPawnStructure bewertet die Bauernstruktur der beiden Spieler.
         */
        Score evalPawnStructure(int32_t side);

        inline Score evalDoublePawns(Bitboard doublePawns);
        inline Score evalIsolatedPawns(Bitboard isolatedPawns);
        inline Score evalPassedPawns(Bitboard passedPawns, const Bitboard& ownPawnAttacks, int32_t side);
        inline Score evalPawnChains(Bitboard pawnChains);
        inline Score evalConnectedPawns(Bitboard connectedPawns);
        
        /**
         * @brief Die Methode evalMGKingSafety bewertet die Sicherheit des Königs der beiden Spieler im Midgame.
         */
        int32_t evalMGKingSafety();
        inline int32_t evalMGPawnShield(int32_t kingSquare, const Bitboard& ownPawns, int32_t side);
        inline int32_t evalMGPawnStorm(int32_t otherKingSquare, const Bitboard& ownPawns, int32_t side);
        int32_t evalMGKingAttackZone(int32_t side);

        /**
         * @brief Die Methode evalMobility bewertet die Mobilität der beiden Spieler.
         */
        int32_t evalMGMobility();
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
        StaticEvaluator() = delete;

        StaticEvaluator(Board& b) : Evaluator(b) {};

        ~StaticEvaluator() override = default;

        StaticEvaluator(const StaticEvaluator& other) = delete;
        StaticEvaluator& operator=(const StaticEvaluator& other) = delete;

        StaticEvaluator(StaticEvaluator&& other);
        StaticEvaluator& operator=(StaticEvaluator&& other);

        /**
         * @brief Führt eine statische Bewertung für das Midgame der
         * Spielpositon aus der Sicht des Spielers der am Zug ist durch.
         * 
         * @param b Das aktuelle Spielfeld.
         * @return Die Bewertung der Spielposition.
         * Je größer der Wert, desto besser ist die Spielposition für den Spieler der am Zug ist.
         * Je kleiner der Wert, desto besser ist die Spielposition für den Gegner des Spielers der am Zug ist.
         * Eine Bewertung von 0 bedeutet ein ausgeglichenes Spiel.
         */
        int32_t middlegameEvaluation();

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
        int32_t evaluate() override;

        /**
         * @brief Bestimmt die Spielphase.
         * Ein Wert von 0 bedeutet Mittelspiel, ein Wert von 1 bedeutet Endspiel.
         */
        double getGamePhase() const;

    private:
        /**
         * @brief Hier werden Konstanten für die statische Evaluation definiert,
         * sodass diese leichter angepasst werden können.
         * 
         * Konstanten mit dem Präfix MG sind für das Midgame,
         * während Konstanten mit dem Präfix EG für das Endgame sind.
         */

        static constexpr int16_t MG_PIECE_VALUE[7] = {
            0, // Empty
            120, // Pawn
            400, // Knight
            410, // Bishop
            600, // Rook
            1100, // Queen
            0 // King
        };

        static constexpr int16_t EG_PIECE_VALUE[7] = {
            0, // Empty
            175, // Pawn
            400, // Knight
            410, // Bishop
            600, // Rook
            1250, // Queen
            0 // King
        };

        // Springer sind mehr Wert, wenn mehr Bauern auf dem Feld sind
        static constexpr int32_t KNIGHT_PAWN_VALUE = 2;

        // Türme sind mehr Wert, wenn weniger Bauern auf dem Feld sind
        static constexpr int32_t ROOK_CAPTURED_PAWN_VALUE = 4;

        // Bonus für das Läuferpaar
        static constexpr int32_t MG_BISHOP_PAIR_VALUE = 40;
        static constexpr int32_t EG_BISHOP_PAIR_VALUE = 40;

        // Bonus für Farbdominanz mit einem Läufer
        static constexpr int32_t MG_BISHOP_COLOR_DOMINANCE_VALUE = 70;

        // Bonus für Türme auf offenen/halboffenen Linien
        static constexpr int32_t MG_ROOK_SEMI_OPEN_FILE_VALUE = 20;
        static constexpr int32_t MG_ROOK_OPEN_FILE_VALUE = 35;

        static constexpr int32_t EG_ROOK_SEMI_OPEN_FILE_VALUE = 15;
        static constexpr int32_t EG_ROOK_OPEN_FILE_VALUE = 15;

        // Bonus für Türme die eigene Freibauern von hinten decken(nur Endgame)
        static constexpr int32_t EG_ROOK_SUPPORTING_PASSED_PAWN_VALUE = 35;

        // Bonus für Türme die gegnerische Freibauern von hinten decken(nur Endgame)
        static constexpr int32_t EG_ROOK_BLOCKING_PASSED_PAWN_VALUE = 35;

        // Bestrafung für entwickelte Dame, wenn die eigenen Leichtfiguren noch nicht entwickelt sind
        static constexpr int32_t MG_DEVELOPED_QUEEN_VALUE = -36;

        /**
         * @brief König und Bauern Endspiel
         */
        
        // Bonus für einen unaufhaltbaren Freibauern
        static constexpr int32_t EG_UNSTOPPABLE_PAWN_VALUE = 1000;

        static constexpr Bitboard fileMasks[64] = {
                0x101010101010101,0x202020202020202,0x404040404040404,0x808080808080808,0x1010101010101010,0x2020202020202020,0x4040404040404040,0x8080808080808080,
                0x101010101010101,0x202020202020202,0x404040404040404,0x808080808080808,0x1010101010101010,0x2020202020202020,0x4040404040404040,0x8080808080808080,
                0x101010101010101,0x202020202020202,0x404040404040404,0x808080808080808,0x1010101010101010,0x2020202020202020,0x4040404040404040,0x8080808080808080,
                0x101010101010101,0x202020202020202,0x404040404040404,0x808080808080808,0x1010101010101010,0x2020202020202020,0x4040404040404040,0x8080808080808080,
                0x101010101010101,0x202020202020202,0x404040404040404,0x808080808080808,0x1010101010101010,0x2020202020202020,0x4040404040404040,0x8080808080808080,
                0x101010101010101,0x202020202020202,0x404040404040404,0x808080808080808,0x1010101010101010,0x2020202020202020,0x4040404040404040,0x8080808080808080,
                0x101010101010101,0x202020202020202,0x404040404040404,0x808080808080808,0x1010101010101010,0x2020202020202020,0x4040404040404040,0x8080808080808080,
                0x101010101010101,0x202020202020202,0x404040404040404,0x808080808080808,0x1010101010101010,0x2020202020202020,0x4040404040404040,0x8080808080808080,
        };

        static constexpr Bitboard extendedCenter = 0x3C3C3C3C0000;

        static constexpr Bitboard lightSquares = 0x55AA55AA55AA55AA;
        static constexpr Bitboard darkSquares = 0xAA55AA55AA55AA55;

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

        static constexpr Bitboard centerSquares = 0x0000001818000000;

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

        static constexpr int32_t MG_PAWN_CONNECTED_VALUE = 9;
        static constexpr int32_t EG_PAWN_CONNECTED_VALUE = 5;

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

        static constexpr int32_t MG_PAWN_CHAIN_VALUE = 10;
        static constexpr int32_t EG_PAWN_CHAIN_VALUE = 5;

        // Bestrafung für zwei oder mehrere Bauern in einer Spalte (doppelte Bauern)
        static constexpr int32_t MG_PAWN_DOUBLED_VALUE = -16;
        static constexpr int32_t EG_PAWN_DOUBLED_VALUE = -30;

        // Bestrafung für einen Bauern, der keine Nachbarn hat(keine Bauern in einer Nachbarspalte)
        static constexpr int32_t MG_PAWN_ISOLATED_VALUE = -21;
        static constexpr int32_t EG_PAWN_ISOLATED_VALUE = -39;

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

        static constexpr int32_t MG_PAWN_PASSED_BASE_VALUE = 7;
        static constexpr int32_t EG_PAWN_PASSED_BASE_VALUE = 18;

        // Wird mit der Anzahl der fortgeschrittenen Felder multipliziert
        static constexpr int32_t MG_PAWN_PASSED_RANK_ADVANCED_MULTIPLIER = 2;
        static constexpr int32_t EG_PAWN_PASSED_RANK_ADVANCED_MULTIPLIER = 12;

        // Zusätzlicher Bonus für jeden Freibauern, der von einem anderen Bauern gedeckt wird
        static constexpr int32_t MG_PASSED_PAWN_PROTECTION_VALUE = 20;
        static constexpr int32_t EG_PASSED_PAWN_PROTECTION_VALUE = 55;

        // Bonus für jedes Feld, dass von einer Figur angegriffen wird.
        // Felder, die von generischen Bauern angegriffen werden, werden ausgenommen.
        static constexpr int32_t MG_PAWN_MOBILITY_VALUE = 0;
        static constexpr int32_t EG_PAWN_MOBILITY_VALUE = 0;
        static constexpr int32_t MG_KNIGHT_MOBILITY_VALUE = 2;
        static constexpr int32_t EG_KNIGHT_MOBILITY_VALUE = 1;
        static constexpr int32_t MG_BISHOP_MOBILITY_VALUE = 4;
        static constexpr int32_t EG_BISHOP_MOBILITY_VALUE = 2;
        static constexpr int32_t MG_ROOK_MOBILITY_VALUE = 4;
        static constexpr int32_t EG_ROOK_MOBILITY_VALUE = 3;
        static constexpr int32_t MG_QUEEN_MOBILITY_VALUE = 1;
        static constexpr int32_t EG_QUEEN_MOBILITY_VALUE = 1;

        // Bestrafung für Figuren, die En Prise(ungeschützt) sind
        static constexpr int32_t MG_PIECE_EN_PRISE_VALUE = -6;
        static constexpr int32_t EG_PIECE_EN_PRISE_VALUE = -3;

        // Angriffszone des Königs
        static constexpr Bitboard kingAttackZoneMask[2][64] = {
                // White
                {
                        0x3030302,0x7070705,0xE0E0E0A,0x1C1C1C14,0x38383828,0x70707050,0xE0E0E0A0,0xC0C0C040,
                        0x303030200,0x707070500,0xE0E0E0A00,0x1C1C1C1400,0x3838382800,0x7070705000,0xE0E0E0A000,0xC0C0C04000,
                        0x30303020000,0x70707050000,0xE0E0E0A0000,0x1C1C1C140000,0x383838280000,0x707070500000,0xE0E0E0A00000,0xC0C0C0400000,
                        0x3030302000000,0x7070705000000,0xE0E0E0A000000,0x1C1C1C14000000,0x38383828000000,0x70707050000000,0xE0E0E0A0000000,0xC0C0C040000000,
                        0x303030200000000,0x707070500000000,0xE0E0E0A00000000,0x1C1C1C1400000000,0x3838382800000000,0x7070705000000000,0xE0E0E0A000000000,0xC0C0C04000000000,
                        0x303020000000003,0x707050000000007,0xE0E0A000000000E,0x1C1C14000000001C,0x3838280000000038,0x7070500000000070,0xE0E0A000000000E0,0xC0C04000000000C0,
                        0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                        0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                },
                // Black
                {
                        0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                        0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                        0x300000000020303,0x700000000050707,0xE000000000A0E0E,0x1C00000000141C1C,0x3800000000283838,0x7000000000507070,0xE000000000A0E0E0,0xC00000000040C0C0,
                        0x2030303,0x5070707,0xA0E0E0E,0x141C1C1C,0x28383838,0x50707070,0xA0E0E0E0,0x40C0C0C0,
                        0x203030300,0x507070700,0xA0E0E0E00,0x141C1C1C00,0x2838383800,0x5070707000,0xA0E0E0E000,0x40C0C0C000,
                        0x20303030000,0x50707070000,0xA0E0E0E0000,0x141C1C1C0000,0x283838380000,0x507070700000,0xA0E0E0E00000,0x40C0C0C00000,
                        0x2030303000000,0x5070707000000,0xA0E0E0E000000,0x141C1C1C000000,0x28383838000000,0x50707070000000,0xA0E0E0E0000000,0x40C0C0C0000000,
                        0x203030300000000,0x507070700000000,0xA0E0E0E00000000,0x141C1C1C00000000,0x2838383800000000,0x5070707000000000,0xA0E0E0E000000000,0x40C0C0C000000000,
                }
        };

        // Gewährt eine Punktzahl, je stärker der gegnerische König angegriffen wird
        static constexpr int16_t kingSafetyTable[100] = {
                0,  0,   1,   2,   3,   5,   7,   9,  12,  15,
                18,  22,  26,  30,  35,  39,  44,  50,  56,  62,
                68,  75,  82,  85,  89,  97, 105, 113, 122, 131,
                140, 150, 169, 180, 191, 202, 213, 225, 237, 248,
                260, 272, 283, 295, 307, 319, 330, 342, 354, 366,
                377, 389, 401, 412, 424, 436, 448, 459, 471, 483,
                494, 500, 500, 500, 500, 500, 500, 500, 500, 500,
                500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
                500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
                500, 500, 500, 500, 500, 500, 500, 500, 500, 500
        };
        static constexpr int32_t kingSafetyTableSize = sizeof(kingSafetyTable) / sizeof(kingSafetyTable[0]);

        // Für jedes Feld in der gegnerischen Königszone, die von Leicht- oder Schwerfiguren angegriffen werden,
        // wird der Wert der entsprechenden Figur auf eine Summe addiert, die als Index für das obige Array dient.
        static constexpr int32_t MG_KING_SAFETY_KNIGHT_THREAT_VALUE = 2;
        static constexpr int32_t MG_KING_SAFETY_BISHOP_THREAT_VALUE = 2;
        static constexpr int32_t MG_KING_SAFETY_ROOK_THREAT_VALUE = 3;
        static constexpr int32_t MG_KING_SAFETY_QUEEN_THREAT_VALUE = 5;

        /**
         * @brief Bauernschilder und -stürme werden nur für das Midgame bewertet.
         */

        // Bonus für jeden Bauern, der den König schützt.
        static constexpr Bitboard pawnShieldMask[2][64] = {
                // White
                {
                        0x30300,0x70700,0xE0E00,0x1C1C00,0x383800,0x707000,0xE0E000,0xC0C000,
                        0x3030000,0x7070000,0xE0E0000,0x1C1C0000,0x38380000,0x70700000,0xE0E00000,0xC0C00000,
                        0x303000000,0x707000000,0xE0E000000,0x1C1C000000,0x3838000000,0x7070000000,0xE0E0000000,0xC0C0000000,
                        0x30300000000,0x70700000000,0xE0E00000000,0x1C1C00000000,0x383800000000,0x707000000000,0xE0E000000000,0xC0C000000000,
                        0x3030000000000,0x7070000000000,0xE0E0000000000,0x1C1C0000000000,0x38380000000000,0x70700000000000,0xE0E00000000000,0xC0C00000000000,
                        0x303000000000000,0x707000000000000,0xE0E000000000000,0x1C1C000000000000,0x3838000000000000,0x7070000000000000,0xE0E0000000000000,0xC0C0000000000000,
                        0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                        0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                },
                // Black
                {
                        0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                        0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                        0x303,0x707,0xE0E,0x1C1C,0x3838,0x7070,0xE0E0,0xC0C0,
                        0x30300,0x70700,0xE0E00,0x1C1C00,0x383800,0x707000,0xE0E000,0xC0C000,
                        0x3030000,0x7070000,0xE0E0000,0x1C1C0000,0x38380000,0x70700000,0xE0E00000,0xC0C00000,
                        0x303000000,0x707000000,0xE0E000000,0x1C1C000000,0x3838000000,0x7070000000,0xE0E0000000,0xC0C0000000,
                        0x30300000000,0x70700000000,0xE0E00000000,0x1C1C00000000,0x383800000000,0x707000000000,0xE0E000000000,0xC0C000000000,
                        0x3030000000000,0x7070000000000,0xE0E0000000000,0x1C1C0000000000,0x38380000000000,0x70700000000000,0xE0E00000000000,0xC0C00000000000,
                }
        };

        static constexpr int32_t MG_PAWN_SHIELD_VALUE = 21;

        // Bonus für jeden Bauern, der den König angreift(oder einen Angriff droht).
        static constexpr Bitboard pawnStormMask[2][64] = {
                // White
                {
                        0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                        0x3,0x7,0xE,0x1C,0x38,0x70,0xE0,0xC0,
                        0x303,0x707,0xE0E,0x1C1C,0x3838,0x7070,0xE0E0,0xC0C0,
                        0x30303,0x70707,0xE0E0E,0x1C1C1C,0x383838,0x707070,0xE0E0E0,0xC0C0C0,
                        0x3030303,0x7070707,0xE0E0E0E,0x1C1C1C1C,0x38383838,0x70707070,0xE0E0E0E0,0xC0C0C0C0,
                        0x303030303,0x707070707,0xE0E0E0E0E,0x1C1C1C1C1C,0x3838383838,0x7070707070,0xE0E0E0E0E0,0xC0C0C0C0C0,
                        0x30303030303,0x70707070707,0xE0E0E0E0E0E,0x1C1C1C1C1C1C,0x383838383838,0x707070707070,0xE0E0E0E0E0E0,0xC0C0C0C0C0C0,
                        0x3030303030303,0x7070707070707,0xE0E0E0E0E0E0E,0x1C1C1C1C1C1C1C,0x38383838383838,0x70707070707070,0xE0E0E0E0E0E0E0,0xC0C0C0C0C0C0C0,
                },
                // Black
                {
                        0x303030303030300,0x707070707070700,0xE0E0E0E0E0E0E00,0x1C1C1C1C1C1C1C00,0x3838383838383800,0x7070707070707000,0xE0E0E0E0E0E0E000,0xC0C0C0C0C0C0C000,
                        0x303030303030000,0x707070707070000,0xE0E0E0E0E0E0000,0x1C1C1C1C1C1C0000,0x3838383838380000,0x7070707070700000,0xE0E0E0E0E0E00000,0xC0C0C0C0C0C00000,
                        0x303030303000000,0x707070707000000,0xE0E0E0E0E000000,0x1C1C1C1C1C000000,0x3838383838000000,0x7070707070000000,0xE0E0E0E0E0000000,0xC0C0C0C0C0000000,
                        0x303030300000000,0x707070700000000,0xE0E0E0E00000000,0x1C1C1C1C00000000,0x3838383800000000,0x7070707000000000,0xE0E0E0E000000000,0xC0C0C0C000000000,
                        0x303030000000000,0x707070000000000,0xE0E0E0000000000,0x1C1C1C0000000000,0x3838380000000000,0x7070700000000000,0xE0E0E00000000000,0xC0C0C00000000000,
                        0x303000000000000,0x707000000000000,0xE0E000000000000,0x1C1C000000000000,0x3838000000000000,0x7070000000000000,0xE0E0000000000000,0xC0C0000000000000,
                        0x300000000000000,0x700000000000000,0xE00000000000000,0x1C00000000000000,0x3800000000000000,0x7000000000000000,0xE000000000000000,0xC000000000000000,
                        0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                }
        };

        static constexpr int32_t MG_PAWN_STORM_BASE_VALUE = 5;

        // Wird mit der Anzahl der fortgeschrittenen Felder multipliziert
        static constexpr int32_t MG_PAWN_STORM_DISTANCE_MULTIPLIER = 20;

        /**
         * @brief Bewertung für die Distanz zwischen den Königen.
         * Wird addiert, wenn die eigene Farbe mehr Material(wie in EG_WINNING_MATERIAL_ADVANTAGE angegeben) besitzt und subtrahiert,
         * wenn die gegnerische Farbe mehr Material besitzt.
         */
        static constexpr int32_t EG_KING_DISTANCE_VALUE = 20;

        static constexpr int32_t EG_WINNING_MATERIAL_ADVANTAGE = 500;

        /**
         * @brief Die PSQT aus der Sicht der weißen Figuren für das Midgame.
         * 
         * Sehr ähnlich wie https://www.chessprogramming.org/PeSTO%27s_Evaluation_Function.
         */
        static constexpr int16_t MG_PSQT[7][64] = {
                // Empty
                {},
                // Pawn
                {
                        0, 0, 0, 0, 0, 0, 0, 0,
                        -18, 0, -10, -12, -7, 12, 19, -11,
                        -15, -2, -2, -5, 1, 1, 17, -9,
                        -14, -1, -2, 6, 8, 3, 5, -13,
                        -7, 6, 3, 3, 3, 6, 8, -12,
                        -3, 4, 6, -2, -4, 13, 6, -5,
                        24, 33, 15, 24, 17, 31, 8, -3,
                        0, 0, 0, 0, 0, 0, 0, 0,
                },
                // Knight
                {
                        -52, -10, -29, -17, -8, -14, -10, -12,
                        -15, -27, -6, -2, -1, 9, -7, -10,
                        5, -5, 6, 5, 9, 8, 12, -8,
                        -4, 10, 10, 14, 7, 8, 2, -7,
                        11, 9, 34, 18, 27, 9, 9, -5,
                        22, 36, 64, 42, 32, 18, 30, -24,
                        -8, 4, 31, 12, 18, 36, -20, -37,
                        -54, -7, -49, 30, -24, -17, -45, -83,
                },
                // Bishop
                {
                        -16, -1, -7, -10, -6, -6, -20, -10,
                        2, 7, 8, 0, 3, 10, 16, 0,
                        0, 7, 7, 7, 7, 13, 9, 5,
                        -3, 6, 6, 13, 17, 6, 5, 2,
                        -2, 2, 10, 25, 18, 18, 3, -1,
                        -8, 18, 21, 20, 17, 25, 18, -1,
                        -13, 8, -9, -6, 15, 29, 9, -24,
                        -15, 2, -41, -19, -13, -21, 3, -4,
                },
                // Rook
                {
                        -9, -7, 0, 8, 8, 3, -18, -13,
                        -22, -8, -10, -4, 0, 5, -3, -36,
                        -22, -13, -8, -8, 1, 0, -2, -17,
                        -18, -13, -6, 0, 4, -3, 3, -12,
                        -12, -6, 4, 13, 12, 17, -4, -10,
                        -2, 9, 13, 18, 8, 23, 30, 8,
                        13, 16, 29, 31, 40, 33, 13, 22,
                        16, 21, 16, 26, 32, 4, 15, 21,
                },
                // Queen
                {
                        -5, -14, -5, 5, -8, -13, -21, -30,
                        -22, -9, 5, 1, 4, 7, -6, -5,
                        -12, -4, -5, -1, -3, 1, 2, -3,
                        -10, -18, -5, -5, -1, -2, -4, -7,
                        -19, -19, -8, -8, 0, 8, -6, -46,
                        -11, -13, 3, 4, 14, 28, 18, 24,
                        -17, -25, -3, 1, -8, 28, 9, 22,
                        -19, -5, 14, 6, 29, 22, 17, 18,
                },
                // King
                {
                        -7, 18, 6, -27, 4, -14, 12, 7,
                        0, 4, -4, -32, -21, -8, 5, 4,
                        -7, -7, -11, -23, -22, -15, -7, -14,
                        -24, 0, -13, -20, -23, -22, -17, -26,
                        -9, -10, -6, -14, -15, -12, -7, -18,
                        -5, 12, 1, -8, -10, 3, 11, -11,
                        14, 0, -10, -4, -4, -2, -19, -15,
                        -33, 11, 8, -8, -28, -17, 1, 6,
                }
        };

        /**
         * @brief Die PSQT aus der Sicht der weißen Figuren für das Endgame.
         *
         * Sehr ähnlich wie https://www.chessprogramming.org/PeSTO%27s_Evaluation_Function.
         */
        static constexpr int16_t EG_PSQT[7][64] = {
                // Empty
                {},
                // Pawn
                {
                        0, 0, 0, 0, 0, 0, 0, 0,
                        6, 4, 4, 5, 6, 0, 1, -4,
                        2, 4, -3, 0, 0, -2, 0, -4,
                        6, 4, -2, -4, -4, -4, 2, -1,
                        16, 12, 6, 2, -1, 2, 9, 9,
                        47, 50, 43, 34, 28, 27, 41, 42,
                        89, 87, 79, 67, 74, 66, 82, 93,
                        0, 0, 0, 0, 0, 0, 0, 0,
                },
                // Knight
                {
                        -15, -26, -12, -8, -11, -9, -25, -32,
                        -21, -10, -5, -3, -1, -10, -12, -22,
                        -11, -2, -1, 7, 5, -2, -10, -11,
                        -9, -3, 8, 12, 8, 8, 2, -9,
                        -9, 2, 11, 11, 11, 6, 4, -9,
                        -12, -10, 5, 4, 0, -4, -10, -20,
                        -12, -4, -12, -1, -5, -12, -12, -26,
                        -29, -19, -7, -14, -15, -13, -32, -50,
                },
                // Bishop
                {
                        -12, -5, -12, -3, -5, -8, -3, -9,
                        -7, -9, -4, -1, 2, -4, -8, -13,
                        -6, -1, 4, 5, 7, 1, -4, -8,
                        -3, 1, 6, 10, 3, 5, -2, -5,
                        -1, 4, 6, 4, 7, 5, 1, 1,
                        1, -4, 0, -1, -1, 3, 0, 2,
                        -4, -2, 3, -6, -2, -6, -2, -7,
                        -7, -10, -6, -4, -4, -5, -8, -12,
                },
                // Rook
                {
                        -4, 1, 2, -1, -3, -7, 2, -10,
                        -3, -3, 0, 1, -5, -5, -6, -2,
                        -2, 0, -2, 0, -4, -6, -4, -8,
                        1, 2, 4, 2, -3, -3, -4, -5,
                        2, 2, 7, 0, 1, 1, -1, 1,
                        3, 3, 3, 2, 2, -1, -2, -1,
                        5, 6, 6, 5, -1, 1, 4, 1,
                        6, 5, 9, 7, 6, 6, 4, 3,
                },
                // Queen
                {
                        -17, -14, -11, -21, -3, -16, -10, -20,
                        -11, -11, -15, -8, -8, -12, -18, -16,
                        -8, -14, 7, 3, 4, 8, 5, 2,
                        -9, 14, 9, 23, 16, 17, 20, 12,
                        1, 11, 12, 23, 29, 20, 29, 18,
                        -10, 3, 4, 24, 24, 17, 9, 4,
                        -9, 10, 16, 20, 29, 12, 15, 0,
                        -5, 11, 11, 13, 13, 9, 5, 10,
                },
                // King
                {
                        -27, -17, -11, -6, -14, -7, -12, -22,
                        -14, -6, 2, 7, 7, 2, -2, -9,
                        -10, -2, 5, 10, 11, 8, 4, -5,
                        -9, -2, 10, 12, 13, 11, 4, -6,
                        -4, 11, 12, 13, 13, 17, 13, 2,
                        5, 9, 12, 8, 10, 22, 22, 7,
                        -6, 9, 7, 9, 9, 19, 12, 6,
                        -37, -18, -9, -9, -5, 7, 2, -9,
                },
        };


        /**
         * @brief Konstanten zur Berechnung der Spielphase.
         * Eine Phase von 0 bedeutet, Midgame und eine Phase von 1 bedeutet Endgame.
         * Diese Phase wird benutzt um zwischen Midgame- und Endgameevaluation zu interpolieren.
         */

        // Figurengewichte
        static constexpr int32_t PAWN_WEIGHT = 0;
        static constexpr int32_t KNIGHT_WEIGHT = 1;
        static constexpr int32_t BISHOP_WEIGHT = 1;
        static constexpr int32_t ROOK_WEIGHT = 2;
        static constexpr int32_t QUEEN_WEIGHT = 4;

        // Phasengrenzen, können unter 0 oder über 1 sein,
        // die berechnete Phase wird aber zwischen 0 und 1 eingeschränkt
        static constexpr double MIN_PHASE = -0.5;
        static constexpr double MAX_PHASE = 1.25;
};

#endif