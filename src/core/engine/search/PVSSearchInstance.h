#ifndef PVS_SEARCH_INSTANCE_H
#define PVS_SEARCH_INSTANCE_H

#include "core/chess/Board.h"

#include "core/engine/evaluation/StaticEvaluator.h"
#include "core/engine/evaluation/NNUEEvaluator.h"
#include "core/engine/search/SearchDefinitions.h"

#include "core/utils/tables/TranspositionTable.h"

#include "uci/Options.h"

#include <array>
#include <atomic>
#include <chrono>
#include <functional>
#include <random>

/**
 * @brief Eine Klasse, die alle Informationen für eine Hauptvariantensuche
 * (Principal Variation Search) enthält.
 */
class PVSSearchInstance {
    private:
        /**
         * @brief Enthält alle Knoteninformationen,
         * auf die Kindknoten zugreifen können sollen.
         */
        struct SearchStackEntry {
            Array<MoveScorePair, 256> moveScorePairs;
            Move hashMove = Move::nullMove();
            int16_t staticEvaluation = 0;
            bool staticEvaluationValid = false;
        };

        Board board;
        #if defined(USE_HCE)
            StaticEvaluator evaluator;
        #else
            NNUEEvaluator evaluator;
        #endif

        /**
         * @brief Eine Referenz auf die Transpositionstabelle,
         * die von dieser Instanz verwendet werden soll.
         */
        TranspositionTable& transpositionTable;

        /**
         * Globale Informationen, die innerhalb einer
         * Suchinstanz gespeichert werden.
         */

        /**
         * @brief Speichert zwei Killerzüge pro Tiefe.
         * Killerzüge sind ruhige Züge (keine Schlagzüge oder Bauernaufwertungen),
         * die einen Beta-Schnitt verursacht haben.
         */
        Move killerMoves[MAX_PLY][2];

        /**
         * @brief Speichert die relative Vergangenheitsbewertung
         * für jedes Tupel (Farbe, Ursprungsfeld, Zielfeld).
         * Die relative Vergangenheitsbewertung ist eine ganzzahlige
         * Approximation der Wahrscheinlichkeit, dass ein Zug
         * nicht zu einer Bewertung <= alpha führt. Je höher die
         * Zahl, desto häufiger führte der Zug zu einer Bewertung
         * > alpha.
         */
        int32_t historyTable[2][64][64];

        /**
         * @brief Eine Zugtabelle, um die Hauptvariante (die Zugfolge,
         * die zu der Bewertung des Wurzelknotens führt) zu rekonstruieren.
         */
        Array<Move, MAX_PLY> pvTable[MAX_PLY];

        /**
         * @brief Die Bewertung der Hauptvariante.
         */
        int16_t pvScore;

        /**
         * @brief Eine Referenz auf die Stop-Flag, der von der
         * Suchinstanz verwendet werden soll. Sobald diese Variable
         * auf true gesetzt wird, bringt die Suchinstanz ihre Suche
         * ab und gibt aus pvs() zurück.
         */
        std::atomic_bool& stopFlag;

        /**
         * @brief Gibt an, ob die Suchinstanz die Hauptinstanz innerhalb
         * des Lazy SMP-Algorithmus ist.
         */
        bool isMainThread = false;

        /**
         * Eine Referenz auf die Start- und Stoppzeit der momentanen Suche.
         */

        std::atomic<std::chrono::system_clock::time_point>& startTime;
        std::atomic<std::chrono::system_clock::time_point>& stopTime;

        /**
         * Verfolgt die Anzahl der Knoten (Schachpositionen),
         * die während der aktuellen Suche betrchet wurden.
         */

        std::atomic_uint64_t& nodesSearched;
        uint64_t localNodeCounter = 0;
        int16_t currentSearchDepth = 0;

        /**
         * @brief Der Suchstapel, der von der Suchinstanz verwendet wird.
         * Der Suchstapel enthält alle Knoteninformationen, die von
         * Vaterknoten für Kindknoten bereitgestellt werden sollen.
         */
        SearchStackEntry searchStack[MAX_PLY];

        /**
         * @brief Eine Referenz auf die Liste der Züge, auf die sich die
         * Suchinstanz im Wurzelknoten beschränken soll. Wenn die Liste leer
         * ist, betrachtet die Instanz alle Züge.
         */
        const Array<Move, 256>& searchMoves;

        /**
         * @brief Speichert die Anzahl der Threads, die gleichzeitig
         * den Spielbaum durchsuchen. Diese Variable wird automatisch
         * im Konstruktor auf den Wert der UCI-Option "Threads" gesetzt.
         */
        size_t numThreads = 1;

        /**
         * @brief Ein Mersenne-Twister-Zufallszahlengenerator, der von
         * der Suchinstanz verwendet wird, um die Zugvorsortierung
         * (leicht) zu beeinflussen. Diese Maßnahme hilft den Threads des
         * Lazy SMP-Algorithmus dabei, voneinander zu divergieren.
         * 
         * Hauptinstanzen verwenden den Zufallszahlengenerator nicht.
         */
        std::mt19937 mersenneTwister;

        /**
         * @brief Eine Funktion, die von der Suchinstanz während der Suche
         * regelmaßig aufgerufen wird.
         */
        std::function<void()> checkupFunction;

        /**
         * @brief Führt eine Quieszenzsuche durch. Die Quieszenzsuche
         * ist eine spezielle Form des Alpha-Beta-Algorithmus, die
         * nur "interessante" Züge betrachtet.
         * 
         * @param ply Der Abstand zum Wurzelknoten.
         * @param alpha Die untere Schranke des Suchfensters.
         * @param beta Die obere Schranke des Suchfensters.
         * @return Die Bewertung der Position.
         */
        int16_t quiescence(int16_t ply, int16_t alpha, int16_t beta);

        /**
         * @brief Bestimmt, wie stark die Tiefe eines Knotens für die
         * Late Move Reduction-Heuristik (LMR) zusätzlich reduziert werden soll.
         * 
         * @param moveCount Die Platzierung des Knotens in der Zugvorsortierung.
         * @param moveScore Die Bewertung des letzten Zuges durch die Zugvorsortierung.
         * @return Die Tiefe, um die der Knoten zusätzlich reduziert werden soll.
         */
        int16_t determineLMR(int16_t moveCount, int16_t moveScore);

        /**
         * @brief Bestimme die Tiefe, um die ein Knoten für die ProbCut-Heuristik
         * (mit beta) zusätzlich reduziert werden soll.
         * 
         * @param staticEval Die statistische Bewertung der Position.
         * @param beta Die obere Schranke des Suchfensters.
         * @return Die Tiefe, um die der Knoten zusätzlich reduziert werden soll.
         */
        int16_t determineProbCutReduction(int16_t staticEval, int16_t beta);

        /**
         * @brief Überprüft anhand der momentanen Position, ob das
         * Null Move Pruning durchgeführt werden darf.
         */
        bool deactivateNullMove();

        /**
         * @brief Generiert alle Züge, die in der momentanen Position
         * möglich sind und fügt sie sortiert in die Zugliste im Suchstapel ein.
         * 
         * @param ply Der Abstand zum Wurzelknoten (Index des Suchstapels).
         * @param useIID Gibt an, ob ein Hashzug "on the fly" bestimmt werden soll,
         * wenn kein Eintrag in der Transpositionstabelle gefunden wurde.
         * Diese Option führt eine reduzierte aber sonst vollständige Suche durch
         * und sollte nur in wichtigen Knoten verwendet werden.
         * @param depth Die verbleibende Suchtiefe.
         */
        void addMovesToSearchStack(uint16_t ply, bool useIID, int16_t depth);

        /**
         * @brief Generiert alle Züge, die in der momentanen Position
         * für die Quieszenzsuche relevant sind und fügt sie sortiert in die
         * Zugliste im Suchstapel ein.
         *
         * @param ply Der Abstand zum Wurzelknoten (Index des Suchstapels).
         * @param minMoveScore Die minimale Bewertung, die ein Zug haben muss,
         * um in die Vorsortierung aufgenommen zu werden.
         * @param includeHashMove Gibt an, ob der Hashzug in die Zugliste
         * aufgenommen werden soll (falls vorhanden und nicht bereits in der Liste).
         */
        void addMovesToSearchStackInQuiescence(uint16_t ply, int16_t minMoveScore, bool includeHashMove);

        /**
         * @brief Bewertet alle Züge in der Zugliste, sortiert sie
         * und hängt sie an die Zugliste im Suchstapel an.
         * 
         * @param moves Die Liste der Züge, die bewertet werden sollen.
         * @param ply Der Abstand zum Wurzelknoten (Index des Suchstapels).
         */
        void scoreMoves(const Array<Move, 256>& moves, uint16_t ply);

        /**
         * @brief Bewertet alle Züge in der Zugliste für die Quiessenzsuche,
         * sortiert sie und hängt sie an die Zugliste im Suchstapel an.
         * 
         * @param moves Die Liste der Züge, die bewertet werden sollen.
         * @param ply Der Abstand zum Wurzelknoten (Index des Suchstapels).
         * @param minMoveScore Die minimale Bewertung, die ein Zug haben muss,
         * um in die Vorsortierung aufgenommen zu werden.
         */
        void scoreMovesForQuiescence(const Array<Move, 256>& moves, uint16_t ply, int16_t minMoveScore);

    public:

        /**
         * @brief Konstruktor für eine Suchinstanz.
         */
        PVSSearchInstance(Board& board, TranspositionTable& transpositionTable,
                          std::atomic_bool& stopFlag, std::atomic<std::chrono::system_clock::time_point>& startTime,
                          std::atomic<std::chrono::system_clock::time_point>& stopTime, std::atomic_uint64_t& nodesSearched,
                          const Array<Move, 256>& searchMoves, std::function<void()> checkupFunction) :
            board(board), evaluator(this->board), transpositionTable(transpositionTable), stopFlag(stopFlag), startTime(startTime), 
            stopTime(stopTime), nodesSearched(nodesSearched), searchStack(), searchMoves(searchMoves), checkupFunction(checkupFunction) {

            // Leere die Killerzüge und die Vergangenheitsbewertung.

            for(int16_t i = 0; i < MAX_PLY; i++) {
                killerMoves[i][0] = Move::nullMove();
                killerMoves[i][1] = Move::nullMove();
            }

            for(int16_t i = 0; i < 2; i++)
                for(int16_t j = 0; j < 64; j++)
                    for(int16_t k = 0; k < 64; k++)
                        historyTable[i][j][k] = 0;

            // Leere die PV-Tabelle.
            for(int16_t i = 0; i < MAX_PLY; i++)
                pvTable[i].clear();

            // Setze die Anzahl der Threads.
            numThreads = UCI::options["Threads"].getValue<size_t>();

            // Initialisiere den Zufallszahlengenerator mit einem festen Seed.
            mersenneTwister.seed(0);
        }

        /**
         * @brief Führt eine vollständige Hauptvariantensuche durch.
         * 
         * @param depth Die Suchtiefe.
         * @param ply Der Abstand zum Wurzelknoten. Bei einem Aufruf
         * von außerhalb der Suchinstanz sollte dieser Wert 0 sein.
         * @param alpha Die untere Schranke des Suchfensters. Wenn keine
         * untere Schranke bekannt ist, sollte MIN_SCORE übergeben werden.
         * @param beta Die obere Schranke des Suchfensters. Wenn keine
         * obere Schranke bekannt ist, sollte MAX_SCORE übergeben werden.
         * @param allowNullMove Gibt an, ob Null Move Pruning verwendet werden darf.
         * Von außerhalb der Suchinstanz sollte false übergeben werden.
         * @param nodeType Der erwartete Typ des Knotens. Von außerhalb der
         * Suchinstanz sollte PV_NODE übergeben werden.
         * @return Die Bewertung der Position (oder 0, wenn die Suche vorzeitig über die Stop-Flag abgebrochen wurde).
         * Die Hauptvariante kann über die Methode getPV() ausgelesen werden.
         */
        int16_t pvs(int16_t depth, uint16_t ply, int16_t alpha, int16_t beta, bool allowNullMove, uint8_t nodeType);

        /**
         * @brief Sagt der Suchinstanz, ob sie die Hauptinstanz
         * innerhalb des Lazy SMP-Algorithmus ist.
         */
        inline void setMainThread(bool isMainThread) {
            this->isMainThread = isMainThread;
        }

        /**
         * @brief Setzt den Seed des Zufallszahlengenerators.
         */
        inline void setMersenneTwisterSeed(uint_fast32_t mersenneTwisterSeed) {
            mersenneTwister.seed(mersenneTwisterSeed);
        }

        /**
         * @brief Überprüft, ob die Stop-Flag gesetzt ist.
         */
        inline bool shouldStop() {
            return stopFlag.load();
        }

        /**
         * @brief Gibt die gefundene Hauptvariante der Instanz
         * zurück. Die Hauptvariante ist die Liste von Zügen,
         * die zu der Bewertung des Wurzelknotens geführt hat.
         */
        inline Array<Move, MAX_PLY>& getPV() {
            return pvTable[0];
        }

        /**
         * @brief Gibt die Bewertung der Hauptvariante zurück.
         */
        inline int16_t getPVScore() {
            return pvScore;
        }

    private:
        inline void addPVMove(uint16_t ply, Move move) {
            pvTable[ply].clear();
            pvTable[ply].push_back(move);

            if(ply < MAX_PLY - 1)
                pvTable[ply].push_back(pvTable[ply + 1]);
        }

        constexpr void clearPVTable(uint16_t ply) {
            pvTable[ply].clear();
        }

        constexpr void clearMovesInSearchStack(uint16_t ply) {
            searchStack[ply].moveScorePairs.clear();
            searchStack[ply].hashMove = Move::nullMove();
        }

        constexpr void clearStaticEvaluationInSearchStack(uint16_t ply) {
            searchStack[ply].staticEvaluation = 0;
            searchStack[ply].staticEvaluationValid = false;
        }

        constexpr void clearSearchStack(uint16_t ply) {
            clearMovesInSearchStack(ply);
            clearStaticEvaluationInSearchStack(ply);
        }

        constexpr int16_t getStaticEvalInSearchStack(uint16_t ply) {
            if(!searchStack[ply].staticEvaluationValid) {
                searchStack[ply].staticEvaluation = evaluator.evaluate();
                searchStack[ply].staticEvaluationValid = true;
            }

            return searchStack[ply].staticEvaluation;
        }

        constexpr void addKillerMove(uint16_t ply, Move move) {
            if(move != killerMoves[ply][1]) {
                killerMoves[ply][0] = killerMoves[ply][1];
                killerMoves[ply][1] = move;
            }
        }

        constexpr bool isKillerMove(uint16_t ply, Move move) {
            return move == killerMoves[ply][0] || move == killerMoves[ply][1];
        }

        constexpr void incrementHistoryScore(Move move, int16_t depth) {
            historyTable[board.getSideToMove() / COLOR_MASK]
                        [move.getOrigin()]
                        [move.getDestination()] += (depth / ONE_PLY) * (depth / ONE_PLY);
        }

        constexpr void decrementHistoryScore(Move move, int16_t depth) {
            historyTable[board.getSideToMove() / COLOR_MASK]
                        [move.getOrigin()]
                        [move.getDestination()] -= depth / ONE_PLY;
        }

        constexpr int32_t getHistoryScore(Move move) {
            return historyTable[board.getSideToMove() / COLOR_MASK]
                               [move.getOrigin()]
                               [move.getDestination()];
        }

        constexpr int32_t getHistoryScore(Move move, int32_t side) {
            return historyTable[side / COLOR_MASK]
                               [move.getOrigin()]
                               [move.getDestination()];
        }

        /**
         * @brief Eine Konstante für das Delta-Pruning.
         * Gibt den größtmöglichen Materialgewinn in einem Zug
         * (+kleiner Puffer) an.
         */
        static constexpr int16_t DELTA_MARGIN = 2000;

        /**
         * @brief Gibt die (halbe) Breite des Aspirationsfenster für
         * die interne Iterative Tiefensuche an.
         */
        static constexpr int16_t IID_ASPIRATION_WINDOW_SIZE = 15;

        /**
         * @brief Gibt die (halbe) Breite des erweiterten Aspirationsfenster für
         * die interne Iterative Tiefensuche an.
         */
        static constexpr int16_t IID_WIDENED_ASPIRATION_WINDOW_SIZE = 150;

        /**
         * @brief Masken um Sentry-Bauern zu erkennen.
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
};

#endif