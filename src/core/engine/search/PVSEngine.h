#ifndef PVS_ENGINE_H
#define PVS_ENGINE_H

#include <array>
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

#include "core/engine/search/PVSSearchInstance.h"
#include "core/engine/search/SearchDefinitions.h"
#include "core/engine/search/Variation.h"
#include "core/engine/evaluation/Evaluator.h"

#include "core/utils/tables/TranspositionTable.h"

#include "uci/UCI.h"

#define UNUSED(x) (void)(x)

class PVSEngine {
    private:
        /**
         * @brief Eine Referenz auf das zu betrachtende Schachbrett.
         * Das Objekt wird während der Suche nicht verändert.
         */
        Board& board;

        #if defined(USE_HCE) && defined(TUNE)
        /**
         * @brief Die HCE-Parameter, die für die Suche verwendet werden.
         */
        HCEParameters hceParams;
        #endif

        /**
         * @brief Speichert die gefundenen Varianten.
         * Im regulären Betrieb (also kein MultiPV)
         * hat dieser Vektor die Größe 1.
         */
        std::vector<Variation> variations;

        /**
         * @brief Die Transpositionstabelle, die während der Suche
         * von allen Suchinstanzen geteilt wird.
         */
        TranspositionTable transpositionTable;

        /**
         * Variablen für den regelmäßigen Checkup.
         */

        std::chrono::system_clock::time_point lastCheckupTime;
        std::chrono::milliseconds checkupInterval;
        std::function<void()> checkupCallback;

        /**
         * Variablen für die Zeitkontrolle.
         */

        std::atomic_bool stopFlag = true;
        bool isTimeControlled = false;
        bool searching = false;
        std::atomic_bool isPondering = false;
        std::atomic<std::chrono::system_clock::time_point> startTime;
        std::atomic<std::chrono::system_clock::time_point> stopTime;
        std::chrono::system_clock::time_point lastOutputTime;
        std::chrono::milliseconds timeMin;
        std::chrono::milliseconds timeMax;

        /**
         * @brief Die Anzahl der bisher durchsuchten Knoten.
         * Diese Variable wird von allen Suchinstanzen geteilt.
         */
        std::atomic_uint64_t nodesSearched = 0;

        /**
         * @brief Die maximale Suchtiefe, die bisher erreicht wurde.
         */
        int16_t maxDepthReached = 0;

        /**
         * @brief Enthält den besten Zug und seine Bewertung
         * der letzten Iterationen. Diese Informationen werden
         * für die dynamische Zeitkontrolle verwendet.
         */
        Array<MoveScorePair, 5> pvHistory;

        /**
         * @brief Ein Vektor mit allen Threads, auf denen
         * eine Suchinstanz läuft. Die Instanz auf dem Hauptthread
         * wird nicht in diesem Vektor gespeichert.
         */
        std::vector<std::thread> threads;

        /**
         * @brief Die zusätzlichen Suchinstanzen, die auf den
         * Helper-Threads laufen. Die Hauptinstanz läuft auf dem
         * aufrufenden Thread.
         */
        std::vector<PVSSearchInstance*> instances;

        /**
         * @brief Ein Pointer auf die Haupt-Suchinstanz.
         */
        PVSSearchInstance* mainInstance = nullptr;

        /**
         * @brief Erstellt die zusätzlichen Suchinstanzen.
         * 
         * @param numInstances Die Anzahl der zu erstellenden Instanzen.
         */
        void createHelperInstances(size_t numInstances);

        /**
         * @brief Zerstört die Helper-Threads und gibt die
         * Speicherbereiche der Suchinstanzen frei.
         */
        void destroyHelperInstances();

        /**
         * @brief Erstellt für jede Suchinstanz einen Thread und
         * startet die Suche.
         * 
         * @param depth Die Suchtiefe.
         * @param alpha Der untere Wert des Suchfensters.
         * @param beta Der obere Wert des Suchfensters.
         * @param searchMoves Die, zu durchsuchenden, Züge.
         */
        void startHelperThreads(int16_t depth, int16_t alpha, int16_t beta, const Array<Move, 256>& searchMoves);

        /**
         * @brief Stoppt alle Suchinstanzen auf den
         * Helper-Threads. Diese Funktion gibt erst zurück,
         * wenn alle Instanzen zurückgekehrt sind.
         */
        void stopHelperThreads();

        /**
         * Funktionen für die Ausgabe von Informationen
         * nach dem UCI-Protokoll.
         */

        void outputSearchInfo();
        void outputNodesInfo();
        void outputMultiPVInfo(size_t pvIndex);

        /**
         * @brief Bestimmte die Zeitlimits für die Suche.
         */
        void calculateTimeLimits(const UCI::SearchParams& params);

        /**
         * @brief Überprüft, ob die Suche abgebrochen werden soll.
         * 
         * @param isTimeControlled Gibt an, ob die Suche mit dynamischer
         * Zeitkontrolle durchgeführt wird.
         */
        bool extendSearch(bool isTimeControlled);

        inline bool isCheckupTime() {
            return std::chrono::system_clock::now() >= lastCheckupTime + checkupInterval;
        }

        inline void updateStopFlag() {
            if(std::chrono::system_clock::now() >= startTime.load() + timeMax &&
               maxDepthReached > 0)
                stopFlag.store(true);
        }

        inline void checkup() {
            lastCheckupTime = std::chrono::system_clock::now();

            updateStopFlag();

            if(checkupCallback)
                checkupCallback();
        }

    public:
        /**
         * @brief Konstruktor
         * 
         * @param board Das, zu betrachtende, Schachbrett.
         * @param checkupInterval Die Anzahl an Millisekunden, die zwischen
         * zwei Checkups vergehen sollen.
         * @param checkupCallback Die Funktion, die bei jedem Checkup aufgerufen
         * werden soll. Wenn keine Funktion aufgerufen werden soll, kann dieser
         * Parameter weggelassen werden.
         */
        PVSEngine(Board& board, uint32_t checkupInterval = 2, std::function<void()> checkupCallback = nullptr)
                : board(board), checkupInterval(checkupInterval), checkupCallback(checkupCallback) {}

        #if defined(USE_HCE) && defined(TUNE)
        /**
         * @brief Konstruktor mit HCE-Parametern.
         * 
         * @param board Das, zu betrachtende, Schachbrett.
         * @param hceParams Die HCE-Parameter, die für die Suche verwendet werden.
         * @param checkupInterval Die Anzahl an Millisekunden, die zwischen
         * zwei Checkups vergehen sollen.
         * @param checkupCallback Die Funktion, die bei jedem Checkup aufgerufen
         * werden soll. Wenn keine Funktion aufgerufen werden soll, kann dieser
         * Parameter weggelassen werden.
         */
        PVSEngine(Board& board, HCEParameters& hceParams, uint32_t checkupInterval = 2, std::function<void()> checkupCallback = nullptr)
                : board(board), hceParams(hceParams), checkupInterval(checkupInterval), checkupCallback(checkupCallback) {}
        #endif

        PVSEngine(const PVSEngine& other) = delete;
        PVSEngine& operator=(const PVSEngine& other) = delete;

        /**
         * @brief Startet die Suche.
         * 
         * @param params Die Suchparameter.
         */
        void search(const UCI::SearchParams& params);

        /**
         * @brief Stoppt die Suche.
         */
        inline void stop() {
            stopFlag.store(true);
        }

        /**
         * @brief Verändert die Kapazität der Transpositionstabelle.
         */
        inline void setHashTableCapacity(size_t capacity) {
            transpositionTable.resize(capacity);
        }

        /**
         * @brief Gibt die Kapazität der Transpositionstabelle zurück.
         */
        inline size_t getHashTableCapacity() {
            return transpositionTable.getCapacity();
        }

        /**
         * @brief Gibt die Anzahl der geschriebenen Einträge in der
         * Transpositionstabelle zurück.
         */
        inline size_t getHashTableSize() {
            return transpositionTable.getEntriesWritten();
        }

        /**
         * @brief Löscht alle Einträge in der Transpositionstabelle.
         */
        inline void clearHashTable() {
            transpositionTable.clear();
        }

        /**
         * @brief Setzt das Schachbrett, das betrachtet werden soll.
         */
        inline void setBoard(Board& board) {
            this->board = board;
        }

        /**
         * @brief Setzt die Funktion, die bei jedem Checkup aufgerufen werden soll.
         */
        inline void setCheckupCallback(std::function<void()> checkupCallback) {
            this->checkupCallback = checkupCallback;
        }

        inline void setPondering(bool isPondering) {
            this->isPondering.store(isPondering);
        }

        inline Board& getBoard() {
            return board;
        }

        inline Move getBestMove() {
            return variations.empty() ? Move() : variations[0].moves[0];
        }

        inline int16_t getBestMoveScore() {
            return variations.empty() ? 0 : variations[0].score;
        }

        inline std::vector<Variation> getVariations() {
            return variations;
        }

        inline std::vector<Move> getPrincipalVariation() {
            return variations.empty() ? std::vector<Move>() : variations[0].moves;
        }

        inline uint64_t getNodesSearched() const {
            return nodesSearched.load();
        }

        constexpr int16_t getMaxDepthReached() const {
            return maxDepthReached;
        }

        inline int64_t getElapsedTime() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime.load()).count();
        }

        inline bool isSearching() const {
            return searching;
        }

        /**
         * @brief Die (halbe) größe des Aspirationsfensters.
         */
        static constexpr int16_t ASPIRATION_WINDOW = 15;

        /**
         * @brief Die (halbe) größe des erweiterten Aspirationsfensters.
         * Im Falle eines fail-high oder fail-low wird die Größe des
         * Aspirationsfensters auf diesen Wert erweitert.
         */
        static constexpr int16_t WIDENED_ASPIRATION_WINDOW = 150;

        /**
         * @brief Ein sehr schmales Aspirationsfenster schlägt in einseitigen
         * Positionen oft fehl. Um dies zu verhindern, wird das Aspirationsfenster
         * in solchen Positionen anhand der Bewertung der letzten Iteration erweitert.
         */
        static constexpr double ASPIRATION_WINDOW_SCORE_FACTOR = 0.15;

        /**
         * @brief Die maximale Dauer in Millisekunden, die zwischen zwei
         * Ausgaben vergehen darf.
         */
        static constexpr uint64_t MAX_TIME_BETWEEN_OUTPUTS = 2000;

    private:
        constexpr void clearPVHistory() {
            pvHistory.clear();
        }

        /**
         * @brief Bestimmt die tiefste Suchtiefe aller Instanzen.
         */
        inline uint16_t getSelectiveDepth() {
            uint16_t selDepth = mainInstance->getSelectiveDepth();

            for(PVSSearchInstance* instance : instances)
                selDepth = std::max(selDepth, instance->getSelectiveDepth());

            return selDepth;
        }
};

#endif