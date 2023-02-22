#ifndef NEW_EVALUATOR_H
#define NEW_EVALUATOR_H

#include "core/chess/Board.h"
#include "core/engine/Evaluator.h"
#include "core/engine/EvaluationDefinitons.h"
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
            100, // Pawn
            400, // Knight
            410, // Bishop
            600, // Rook
            1200, // Queen
            0 // King
        };

        static constexpr int16_t EG_PIECE_VALUE[7] = {
            0, // Empty
            150, // Pawn
            400, // Knight
            410, // Bishop
            600, // Rook
            1200, // Queen
            0 // King
        };

        // Springer sind mehr Wert, wenn mehr Bauern auf dem Feld sind
        static constexpr int32_t KNIGHT_PAWN_VALUE = 2;

        // Türme sind mehr Wert, wenn weniger Bauern auf dem Feld sind
        static constexpr int32_t ROOK_CAPTURED_PAWN_VALUE = 7;

        // Bonus für das Läuferpaar
        static constexpr int32_t MG_BISHOP_PAIR_VALUE = 50;
        static constexpr int32_t EG_BISHOP_PAIR_VALUE = 50;

        // Bonus für Farbdominanz mit einem Läufer
        static constexpr int32_t MG_BISHOP_COLOR_DOMINANCE_VALUE = 50;

        // Bonus für Türme auf offenen/halboffenen Linien
        static constexpr int32_t MG_ROOK_SEMI_OPEN_FILE_VALUE = 30;
        static constexpr int32_t MG_ROOK_OPEN_FILE_VALUE = 45;

        static constexpr int32_t EG_ROOK_SEMI_OPEN_FILE_VALUE = 40;
        static constexpr int32_t EG_ROOK_OPEN_FILE_VALUE = 40;

        // Bonus für Türme die eigene Freibauern von hinten decken(nur Endgame)
        static constexpr int32_t EG_ROOK_SUPPORTING_PASSED_PAWN_VALUE = 50;

        // Bonus für Türme die gegnerische Freibauern von hinten decken(nur Endgame)
        static constexpr int32_t EG_ROOK_BLOCKING_PASSED_PAWN_VALUE = 40;

        // Bestrafung für entwickelte Dame, wenn die eigenen Leichtfiguren noch nicht entwickelt sind
        static constexpr int32_t MG_DEVELOPED_QUEEN_VALUE = -21;

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

        static constexpr int32_t MG_PAWN_CONNECTED_VALUE = 3;
        static constexpr int32_t EG_PAWN_CONNECTED_VALUE = 1;

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

        static constexpr int32_t MG_PAWN_CHAIN_VALUE = 16;
        static constexpr int32_t EG_PAWN_CHAIN_VALUE = 12;

        // Bestrafung für zwei oder mehrere Bauern in einer Spalte (doppelte Bauern)
        static constexpr int32_t MG_PAWN_DOUBLED_VALUE = -13;
        static constexpr int32_t EG_PAWN_DOUBLED_VALUE = -20;

        // Bestrafung für einen Bauern, der keine Nachbarn hat(keine Bauern in einer Nachbarspalte)
        static constexpr int32_t MG_PAWN_ISOLATED_VALUE = -8;
        static constexpr int32_t EG_PAWN_ISOLATED_VALUE = -16;

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

        static constexpr int32_t MG_PAWN_PASSED_BASE_VALUE = 10;
        static constexpr int32_t EG_PAWN_PASSED_BASE_VALUE = 30;

        // Wird mit der Anzahl der fortgeschrittenen Felder multipliziert
        static constexpr int32_t MG_PAWN_PASSED_RANK_ADVANCED_MULTIPLIER = 20;
        static constexpr int32_t EG_PAWN_PASSED_RANK_ADVANCED_MULTIPLIER = 45;

        // Von anderen Bauern beschütze Freibauern sind noch besser
        static constexpr int32_t MG_PASSED_PAWN_PROTECTION_VALUE = 30;
        static constexpr int32_t EG_PASSED_PAWN_PROTECTION_VALUE = 60;

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
                          0,  0,   0,   0,   0,   0,   0,   0,
                        -35, -1, -20, -23, -15,  24,  38, -22,
                        -30, -4,  -4, -10,   3,   3,  33, -18,
                        -27, -2,  -5,  12,  17,   6,  10, -25,
                        -14, 13,   6,   5,   6,  12,  17, -23,
                        -6,   7,  13,  -4,  -8,  27,  12, -10,
                        49,  67,  30,  47,  34,  63,  17,  -6,
                         0,   0,   0,   0,   0,   0,   0,   0,
                },
                // Knight
                {
                        -105, -21, -58, -33, -17, -28, -19,  -23,
                         -29, -53, -12,  -3,  -1,  18, -14,  -19,
                          11,  -9,  12,  10,  19,  17,  25,  -16,
                          -8,  21,  19,  28,  13,  16,   4,  -13,
                          22,  18,  69,  37,  53,  19,  17,  -9,
                          44,  73, 129,  84,  65,  37,  60,  -47,
                         -17,   7,  62,  23,  36,  72, -41,  -73,
                        -107, -15, -97,  61, -49, -34, -89, -167,
                },
                // Bishop
                {
                        -33,  -3, -14, -21, -13, -12, -39, -21,
                          4,  15,  16,   0,   7,  21,  33,   1,
                          0,  15,  15,  15,  14,  27,  18,  10,
                         -6,  13,  13,  26,  34,  12,  10,   4,
                         -4,   5,  19,  50,  37,  37,   7,  -2,
                        -16,  37,  43,  40,  35,  50,  37,  -2,
                        -26,  16, -18, -13,  30,  59,  18, -47,
                        -29,   4, -82, -37, -25, -42,   7,  -8,
                },
                // Rook
                {
                        -19, -13,   1,  17, 16,  7, -37, -26,
                        -44, -16, -20,  -9, -1, 11,  -6, -71,
                        -45, -25, -16, -17,  3,  0,  -5, -33,
                        -36, -26, -12,  -1,  9, -7,   6, -23,
                        -24, -11,   7,  26, 24, 35,  -8, -20,
                         -5,  19,  26,  36, 17, 45,  61,  16,
                         27,  32,  58,  62, 80, 67,  26,  44,
                         32,  42,  32,  51, 63,  9,  31,  43,
                },
                // Queen
                {
                        -10, -28,  -9,  10, -15, -25, -41, -60,
                        -45, -18,  11,   2,   8,  15, -13,  -9,
                        -24,  -8, -11,  -2,  -5,   2,   4,  -5,
                        -19, -36,  -9, -10,  -2,  -4,  -7, -13,
                        -37, -37, -16, -16,  -1,  17, -12, -91,
                        -23, -27,   7,   8,  29,  56,  37,  47,
                        -34, -49,  -5,   1, -16,  57,  18,  44,
                        -38, -10,  29,  12,  59,  44,  33,  35,
                },
                // King
                {
                        -15,  36,  12, -54,   8, -28,  24,  14,
                          1,   7,  -8, -64, -43, -16,   9,   8,
                        -14, -14, -22, -46, -44, -30, -15, -27,
                        -49,  -1, -27, -39, -46, -44, -33, -51,
                        -17, -20, -12, -27, -30, -25, -14, -36,
                         -9,  24,   2, -16, -20,   6,  22, -22,
                         29,  -1, -20,  -7,  -8,  -4, -38, -29,
                        -65,  23,  16, -15, -56, -34,   2,  13,
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
                          0,   0,   0,   0,   0,   0,   0,   0,
                         13,   8,   8,  10,  13,   0,   2,  -7,
                          4,   7,  -6,   1,   0,  -5,  -1,  -8,
                         13,   9,  -3,  -7,  -7,  -8,   3,  -1,
                         32,  24,  13,   5,  -2,   4,  17,  17,
                         94, 100,  85,  67,  56,  53,  82,  84,
                        178, 173, 158, 134, 147, 132, 165, 187,
                          0,   0,   0,   0,   0,   0,   0,   0,
                },
                // Knight
                {
                        -29, -51, -23, -15, -22, -18, -50, -64,
                        -42, -20, -10,  -5,  -2, -20, -23, -44,
                        -23,  -3,  -1,  15,  10,  -3, -20, -22,
                        -18,  -6,  16,  25,  16,  17,   4, -18,
                        -17,   3,  22,  22,  22,  11,   8, -18,
                        -24, -20,  10,   9,  -1,  -9, -19, -41,
                        -25,  -8, -25,  -2,  -9, -25, -24, -52,
                        -58, -38, -13, -28, -31, -27, -63, -99,
                },
                // Bishop
                {
                        -23,  -9, -23,  -5, -9, -16,  -5, -17,
                        -14, -18,  -7,  -1,  4,  -9, -15, -27,
                        -12,  -3,   8,  10, 13,   3,  -7, -15,
                         -6,   3,  13,  19,  7,  10,  -3,  -9,
                         -3,   9,  12,   9, 14,  10,   3,   2,
                          2,  -8,   0,  -1, -2,   6,   0,   4,
                         -8,  -4,   7, -12, -3, -13,  -4, -14,
                        -14, -21, -11,  -8, -7,  -9, -17, -24,
                },
                // Rook
                {
                        -9,   2,   3,  -1,  -5, -13,   4, -20,
                        -6, -6,   0,   2,  -9,  -9, -11,  -3,
                        -4,   0,  -5,  -1,  -7, -12,  -8, -16,
                         3,   5,   8,   4,  -5,  -6,  -8, -11,
                         4,   3,  13,   1,   2,   1,  -1,   2,
                         7,   7,   7,   5,   4,  -3,  -5,  -3,
                        11,  13,  13,  11,  -3,   3,   8,   3,
                        13,  10,  18,  15,  12,  12,   8,   5,
                },
                // Queen
                {
                        -33, -28, -22, -43,  -5, -32, -20, -41,
                        -22, -23, -30, -16, -16, -23, -36, -32,
                        -16, -27,  15,   6,   9,  17,  10,   5,
                        -18,  28,  19,  47,  31,  34,  39,  23,
                          3,  22,  24,  45,  57,  40,  57,  36,
                        -20,   6,   9,  49,  47,  35,  19,   9,
                        -17,  20,  32,  41,  58,  25,  30,   0,
                         -9,  22,  22,  27,  27,  19,  10,  20,
                },
                // King
                {
                        -53, -34, -21, -11, -28, -14, -24, -43,
                        -27, -11,   4,  13,  14,   4,  -5, -17,
                        -19,  -3,  11,  21,  23,  16,   7,  -9,
                        -18,  -4,  21,  24,  27,  23,   9, -11,
                         -8,  22,  24,  27,  26,  33,  26,   3,
                         10,  17,  23,  15,  20,  45,  44,  13,
                        -12,  17,  14,  17,  17,  38,  23,  11,
                        -74, -35, -18, -18, -11,  15,   4, -17,
                }
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