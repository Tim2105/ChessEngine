#ifndef MINIMAX_ENGINE_H
#define MINIMAX_ENGINE_H

#include "core/engine/InterruptedEngine.h"
#include "core/engine/SearchDetails.h"
#include "core/utils/tables/HashTable.h"
#include "core/utils/tables/TranspositionTable.h"

#include <atomic>
#include <chrono>
#include <stdint.h>
#include <vector>

#define UNUSED(x) (void)(x)

#define _DEBUG_ 0

#define SCORE_MIN -32000
#define SCORE_MAX 32000

#define SCORE_MATE 21000
#define IS_MATE_SCORE(x) (std::abs(x) > SCORE_MATE - 1000)

#define PV_NODE 1
#define CUT_NODE 2

#define FW_NODE 0
#define NW_NODE 4

#define IS_REGULAR_NODE(x) (!((x) & 4))
#define IS_NW_NODE(x) ((x) & 4)
#define NODE_TYPE(x) ((x) & 3)

#define MVVLVA 0
#define SEE 1

#define PLY_ONE 6
#define PLY_ONE_SIXTH 1
#define PLY_ONE_THIRD 2
#define PLY_ONE_HALF 3
#define PLY_TWO_THIRDS 4
#define PLY_FIVE_SIXTHS 5

#define PLY_MAX 256

#define NULL_MOVE_R_VALUE 2

#define NODES_BETWEEN_CHECKUPS 1024

struct MoveScorePair {
    Move move;
    int32_t score;
};

template<>
struct std::greater<MoveScorePair> {
    bool operator()(const MoveScorePair& lhs, const MoveScorePair& rhs) const {
        return lhs.score > rhs.score;
    }
};

template<>
struct std::less<MoveScorePair> {
    bool operator()(const MoveScorePair& lhs, const MoveScorePair& rhs) const {
        return lhs.score < rhs.score;
    }
};

/**
 * @brief Eine Fail-Soft Implementation des Scout-Algorithmus mit anschließender Quieszenzsuche.
 * 
 * Der Scout-Algorithmus ist eine Erweiterung des Alpha-Beta-Algorithmus,
 * der die Nullfenster-Suche für alle Kinder eines Knotens außer dem Ersten verwendet.
 * 
 * Verwendete Optimierungen:
 * - Transpositionstabelle
 * - Killerzug-Heuristik
 * - Konterzug-Heuristik
 * - Vergangenheits-Heuristik
 * - Nullzug-Heuristik
 * - Asprirationsfenster
 */
class MinimaxEngine : public InterruptedEngine {
    private:

        /**
         * @brief Das Ersetzungsprädikat für die Transpositionstabelle.
         * Ein Eintrag wird ersetzt, wenn die Funktion true zurückgibt.
         * 
         * @param lhs Der neue Eintrag.
         * @param rhs Der alte Eintrag.
         */
        static constexpr bool ttReplacementPredicate(const TranspositionTableEntry& lhs,
                                                     const TranspositionTableEntry& rhs) {
                                                        
            return lhs.depth * 1.2f + lhs.age >= rhs.depth * 1.2f + rhs.age;
        };

        /**
         * @brief Enthält Informationen über bereits untersuchte Teilbäume.
         * 
         * Implementiert als flüchtige Hashtabelle, die eine 64-Bit Ganzzahl als Schlüssel verwendet.
         * 
         * @tparam Buckets Die Anzahl der Buckets.
         * @tparam Size Die Größe eines Buckets.
         */
        TranspositionTable<524288, ttReplacementPredicate> transpositionTable;

        /**
         * @brief Eine Kopie des, zu untersuchenden, Spielbretts.
        */
        Board searchBoard;
        std::atomic_bool searchRunning = false;

        /**
         * @brief Die momentane Suchtiefe.
        */
        int16_t currentMaxDepth;
        uint16_t currentAge;
        uint32_t nodesSearched;

        /**
         * Die Start- und Endzeit der Suche.
        */
        std::chrono::system_clock::time_point startTime;
        std::chrono::system_clock::time_point endTime;

        /**
         * @brief Die Tabelle, die die besten Züge für die aktuelle Position speichert.
         * Durch diese Tabelle können die vollständigen Variationen nachvollzogen werden.
        */
        Array<Move, 64> pvTable[64];

        /**
         * @brief Eine Tabelle mit, bis zu zwei, Killerzügen pro Suchtiefe.
         * 
         * Killerzüge sind leise Züge, die in vorherigen Suchtiefen einen Beta-Schnitt verursacht haben.
        */
        Move killerMoves[PLY_MAX][2];

        /**
         * @brief Eine Tabelle, die für jede Figur und jedes Feld einen Zug speichert, der als Antwort auf diesen Zug
         * einen Beta-Schnitt verursacht hat oder der beste Zug war.
        */
        Move counterMoves[15][64];

        /**
         * @brief Eine Tabelle, die für jeden Spieler, für jedes Ausgangsfeld und jedes Zielfeld die Bewertung des Zuges speichert.
        */
        int32_t relativeHistory[2][64][64];

        /**
         * Eine flüchtige Hashtabelle, die die SEE-Bewertungen von Zügen zwischenspeichert.
        */
        HashTable<Move, int32_t, 128, 4> seeCache;

        int16_t mateDistance;

        void clearRelativeHistory();

        void clearPvTable();

        void clearKillerMoves();

        void clearCounterMoves();

        void runSearch(bool timeControl = false, uint32_t minTime = 0, uint32_t maxTime = 0);

        bool extendSearchUnderTimeControl(std::vector<Variation> pvHistory, uint32_t minTime, uint32_t maxTime, uint32_t timeSpent);

        int32_t scoreMove(Move& move, int16_t ply);

        void sortMovesAtRoot(Array<Move, 256>& moves);

        void sortAndCutMovesForQuiescence(Array<Move, 256>& moves, int32_t minScore, int32_t moveEvalFunc);

        void sortMoves(Array<Move, 256>& moves, int16_t ply);

        int16_t rootSearch(int16_t depth, int16_t expectedScore);

        int16_t pvSearchRoot(int16_t depth, int16_t alpha, int16_t beta);

        int16_t pvSearch(int16_t depth, int16_t ply, int16_t alpha, int16_t beta, int32_t nullMoveCooldown);

        int16_t nwSearch(int16_t depth, int16_t ply, int16_t alpha, int16_t beta, int32_t nullMoveCooldown);

        int16_t quiescence(int16_t ply, int16_t alpha, int16_t beta);

        struct PruningVariables {
            Move lastMove;
            int32_t movedPiece;
            bool isCheck;
            int32_t seeScore;
            bool isPawnPush;
            bool isPassedPawnPush;
            bool isCaptureEvasion;
            bool threatensKing;
        };

        PruningVariables determinePruningVariables();

        int16_t determineExtension(PruningVariables& pruningVars, bool isCheckEvasion = false);

        int16_t determineReduction(int16_t depth, int16_t ply, int32_t moveCount, PruningVariables& pruningVars, bool isCheckEvasion = false);

        bool shouldPrune(int16_t depth, int16_t ply, int32_t moveCount, PruningVariables& pruningVars, bool isCheckEvasion = false);

        constexpr bool isMateLine() { return mateDistance != PLY_MAX; }

    protected:

        virtual inline void checkup() override {
            InterruptedEngine::checkup();

            if(getLastCheckupTime() >= endTime && currentMaxDepth > PLY_ONE)
                searchRunning = false;
        }

    public:
        MinimaxEngine() = delete;

        MinimaxEngine(Evaluator& e, uint32_t numVariations = 1, uint32_t checkupInterval = 2, std::function<void()> checkupCallback = nullptr)
                : InterruptedEngine(e, numVariations, checkupInterval, checkupCallback) {
            currentMaxDepth = 0;
            currentAge = board->getPly();
            nodesSearched = 0;
            mateDistance = PLY_MAX;
            searchRunning = false;
        }

        ~MinimaxEngine() {}

        virtual void search(uint32_t searchTime, bool treatAsTimeControl = false) override;

        virtual void stop() override;

        constexpr int16_t getLastSearchDepth() { return currentMaxDepth / PLY_ONE - 1; }

        constexpr uint32_t getNodesSearched() { return nodesSearched; }

        inline void setBoard(Board& b) override {
            stop();

            board = &b;
            evaluator.setBoard(b);

            transpositionTable.clear();
            clearRelativeHistory();
            clearPvTable();
            clearKillerMoves();
            variations.clear();
        }

        inline SearchDetails getSearchDetails() {
            SearchDetails details;
            details.variations = variations;
            details.nodesSearched = nodesSearched;
            details.timeTaken = std::chrono::duration_cast<std::chrono::milliseconds>(getLastCheckupTime() - startTime);
            details.depth = currentMaxDepth / PLY_ONE - 1;

            return details;
        }

    private:
        static constexpr int32_t ASP_WINDOW = 15;
        static constexpr int32_t ASP_STEP_FACTOR = 4;
        static constexpr int32_t ASP_MAX_DEPTH = 3;

        static constexpr int32_t DEFAULT_CAPTURE_MOVE_SCORE = 100;
        static constexpr int32_t DEFAULT_PASSED_PAWN_MOVE_SCORE = 100;

        static constexpr int32_t NEUTRAL_SEE_SCORE = 0;

        /**
         * @brief Masken unm Sentry-Bauern zu erkennen.
         * Sentry-Bauern sind Bauern, die gegnerische Bauern auf dem Weg zur Aufwertung blockieren oder schlagen können.
         */
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

        /**
         * @brief Enthält Bitboards für die umliegenden 2 Felder.
         */
        static constexpr Bitboard vicinity[64] = {
            0x70706ULL,0xf0f0dULL,0x1f1f1bULL,0x3e3e36ULL,0x7c7c6cULL,0xf8f8d8ULL,0xf0f0b0ULL,0xe0e060ULL,
            0x7070607ULL,0xf0f0d0fULL,0x1f1f1b1fULL,0x3e3e363eULL,0x7c7c6c7cULL,0xf8f8d8f8ULL,0xf0f0b0f0ULL,0xe0e060e0ULL,
            0x707060707ULL,0xf0f0d0f0fULL,0x1f1f1b1f1fULL,0x3e3e363e3eULL,0x7c7c6c7c7cULL,0xf8f8d8f8f8ULL,0xf0f0b0f0f0ULL,0xe0e060e0e0ULL,
            0x70706070700ULL,0xf0f0d0f0f00ULL,0x1f1f1b1f1f00ULL,0x3e3e363e3e00ULL,0x7c7c6c7c7c00ULL,0xf8f8d8f8f800ULL,0xf0f0b0f0f000ULL,0xe0e060e0e000ULL,
            0x7070607070000ULL,0xf0f0d0f0f0000ULL,0x1f1f1b1f1f0000ULL,0x3e3e363e3e0000ULL,0x7c7c6c7c7c0000ULL,0xf8f8d8f8f80000ULL,0xf0f0b0f0f00000ULL,0xe0e060e0e00000ULL,
            0x707060707000000ULL,0xf0f0d0f0f000000ULL,0x1f1f1b1f1f000000ULL,0x3e3e363e3e000000ULL,0x7c7c6c7c7c000000ULL,0xf8f8d8f8f8000000ULL,0xf0f0b0f0f0000000ULL,0xe0e060e0e0000000ULL,
            0x706070700000000ULL,0xf0d0f0f00000000ULL,0x1f1b1f1f00000000ULL,0x3e363e3e00000000ULL,0x7c6c7c7c00000000ULL,0xf8d8f8f800000000ULL,0xf0b0f0f000000000ULL,0xe060e0e000000000ULL,
            0x607070000000000ULL,0xd0f0f0000000000ULL,0x1b1f1f0000000000ULL,0x363e3e0000000000ULL,0x6c7c7c0000000000ULL,0xd8f8f80000000000ULL,0xb0f0f00000000000ULL,0x60e0e00000000000ULL,
        };

        /**
         * @brief Array für die Zeiteinteilung anhand der Anzahl der legalen Züge.
         */
        static constexpr double timeFactor[40] = {
                0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4,
                0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4,
                0.42, 0.44, 0.46, 0.48, 0.5, 0.52, 0.54, 0.56, 0.58, 0.6,
                0.64, 0.68, 0.72, 0.76, 0.8, 0.84, 0.88, 0.92, 0.96, 1.0
        };

        static constexpr int32_t timeFactorArraySize = sizeof(timeFactor) / sizeof(timeFactor[0]);
};

#endif