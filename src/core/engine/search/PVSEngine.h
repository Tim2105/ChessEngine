#ifndef PVS_ENGINE_H
#define PVS_ENGINE_H

#include <array>
#include <chrono>

#if not defined(DISABLE_THREADS)
    #include <condition_variable>
    #include <mutex>
    #include <thread>
#endif

#include "core/engine/search/PVSSearchInstance.h"
#include "core/engine/search/SearchDefinitions.h"
#include "core/engine/search/Variation.h"
#include "core/engine/evaluation/Evaluator.h"

#include "core/utils/Atomic.h"
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

        #if defined(USE_HCE)
        /**
         * @brief Die HCE-Parameter, die für die Suche verwendet werden.
         */
        const HCEParameters& hceParams;
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
         * Variable für die Kontrolle der Threads.
         */

        AtomicBool threadSleepFlag = true;
        AtomicBool exitSearch = false;

        /**
         * Variablen für die Zeitkontrolle.
         */

        bool isTimeControlled = false;
        AtomicBool searching = false;
        AtomicBool isPondering = false;
        Atomic<std::chrono::system_clock::time_point> startTime;
        Atomic<std::chrono::system_clock::time_point> stopTime;
        std::chrono::system_clock::time_point lastOutputTime;
        std::chrono::milliseconds timeMin;
        std::chrono::milliseconds timeMax;

        /**
         * @brief Die Anzahl der bisher durchsuchten Knoten.
         * Diese Variable wird von allen Suchinstanzen geteilt.
         */
        AtomicU64 nodesSearched = 0;

        /**
         * @brief Die maximale Suchtiefe, die bisher erreicht wurde.
         */
        int maxDepthReached = 0;

        /**
         * @brief Enthält den besten Zug und seine Bewertung
         * der letzten Iterationen. Diese Informationen werden
         * für die dynamische Zeitkontrolle verwendet.
         */
        Array<MoveScorePair, 5> pvHistory;

        #if not defined(DISABLE_THREADS)
        /**
         * @brief Ein Mutex, der für die
         * Bedingungsvariablen verwendet wird.
         */
        std::mutex cvMutex;

        /**
         * @brief Eine Bedingungsvariable, die verwendet
         * wird, damit die Threads aufgeweckt werden können.
         */
        std::condition_variable cv;

        /**
         * @brief Eine Bedingungsvariable, die verwendet wird,
         * damit der Hauptthread aufgeweckt werden kann sobald
         * alle Helper-Threads zurückgekehrt sind.
         */
        std::condition_variable cvMain;

        /**
         * @brief Die dazugehörige Variable, die zählt,
         * wie viele Helper-Threads noch laufen.
         */
        std::atomic_size_t numThreadsBusy;

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

        /*
         * Suchparameter für die zusätzlichen Suchinstanzen.
         */

        int currentDepth;
        int currentAlpha;
        int currentBeta;

        #endif

        /**
         * @brief Ein Pointer auf die Haupt-Suchinstanz.
         */
        PVSSearchInstance* mainInstance = nullptr;

        /**
         * @brief Bestimmt, ob die Ausgabe der Suchinformationen
         * nach dem UCI-Protokoll erfolgen soll.
         */
        bool uciOutput = true;

        /**
         * @brief Die Funktion, die von den
         * Helper-Threads ausgeführt wird.
         */
        void helperThreadLoop(size_t instanceIdx);

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
         * @brief Übergibt Suchparameter an die zusätzlichen Suchinstanzen
         * und startet sie. Wenn exitSearch auf true gesetzt ist,
         * werden die Helper-Threads zurückgegeben.
         * 
         * @param depth Die Suchtiefe.
         * @param alpha Der untere Wert des Suchfensters.
         * @param beta Der obere Wert des Suchfensters.
         * @param searchMoves Die, zu durchsuchenden, Züge.
         */
        void startHelperThreads(int depth, int alpha, int beta, const Array<Move, 256>& searchMoves);

        /**
         * @brief Pausiert alle Helper-Threads,
         * d.h. die Threads werden in den Schlafmodus
         * versetzt (nicht beendet!).
         */
        void pauseHelperThreads();

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
               stop();
        }

        inline void checkup() {
            lastCheckupTime = std::chrono::system_clock::now();

            updateStopFlag();

            if(checkupCallback)
                checkupCallback();
        }

    public:
        #if defined(USE_HCE)
        /**
         * @brief Konstruktor
         * 
         * @param board Das, zu betrachtende, Schachbrett.
         * @param checkupInterval Die Anzahl an Millisekunden, die zwischen
         * zwei Checkups vergehen sollen.
         * @param checkupCallback Die Funktion, die bei jedem Checkup aufgerufen
         * werden soll. Wenn keine Funktion aufgerufen werden soll, kann dieser
         * Parameter weggelassen werden.
         * @param uciOutput Bestimmt, ob nach dem UCI-Protokoll ausgegeben werden soll.
         */
        PVSEngine(Board& board, uint64_t checkupInterval = 2, std::function<void()> checkupCallback = nullptr, bool uciOutput = true)
                : board(board), hceParams(HCE_PARAMS), checkupInterval(checkupInterval), checkupCallback(checkupCallback), uciOutput(uciOutput) {}

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
         * @param uciOutput Bestimmt, ob nach dem UCI-Protokoll ausgegeben werden soll.
         */
        PVSEngine(Board& board, const HCEParameters& hceParams, uint64_t checkupInterval = 2, std::function<void()> checkupCallback = nullptr, bool uciOutput = true)
                : board(board), hceParams(hceParams), checkupInterval(checkupInterval), checkupCallback(checkupCallback), uciOutput(uciOutput) {}
        #else
        /**
         * @brief Konstruktor
         * 
         * @param board Das, zu betrachtende, Schachbrett.
         * @param checkupInterval Die Anzahl an Millisekunden, die zwischen
         * zwei Checkups vergehen sollen.
         * @param checkupCallback Die Funktion, die bei jedem Checkup aufgerufen
         * werden soll. Wenn keine Funktion aufgerufen werden soll, kann dieser
         * Parameter weggelassen werden.
         * @param uciOutput Bestimmt, ob nach dem UCI-Protokoll ausgegeben werden soll.
         */
        PVSEngine(Board& board, uint64_t checkupInterval = 2, std::function<void()> checkupCallback = nullptr, bool uciOutput = true)
                : board(board), checkupInterval(checkupInterval), checkupCallback(checkupCallback), uciOutput(uciOutput) {}
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
            exitSearch.store(true);
            threadSleepFlag.store(true);
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

        inline void setUCIOutput(bool uciOutput) {
            this->uciOutput = uciOutput;
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

        inline int getBestMoveScore() {
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

        constexpr int getMaxDepthReached() const {
            return maxDepthReached;
        }

        inline int64_t getElapsedTime() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime.load()).count();
        }

        inline bool isSearching() const {
            return searching.load();
        }

        /**
         * @brief Die (halbe) größe des Aspirationsfensters.
         */
        static constexpr int ASPIRATION_WINDOW = 15;

        /**
         * @brief Die (halbe) größe des erweiterten Aspirationsfensters.
         * Im Falle eines fail-high oder fail-low wird die Größe des
         * Aspirationsfensters auf diesen Wert erweitert.
         */
        static constexpr int WIDENED_ASPIRATION_WINDOW = 100;

        /**
         * @brief Ein sehr schmales Aspirationsfenster schlägt in einseitigen
         * Positionen oft fehl. Um dies zu verhindern, wird das Aspirationsfenster
         * in solchen Positionen anhand der Bewertung der letzten Iteration erweitert.
         */
        static constexpr double ASPIRATION_WINDOW_SCORE_FACTOR = 0.1;

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
        inline int getSelectiveDepth() {
            int selDepth = mainInstance->getSelectiveDepth();

            #if not defined(DISABLE_THREADS)
            for(PVSSearchInstance* instance : instances)
                selDepth = std::max(selDepth, instance->getSelectiveDepth());
            #endif

            return selDepth;
        }
};

#endif