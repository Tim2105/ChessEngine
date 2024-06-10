#include "core/engine/search/PVSEngine.h"
#include "core/chess/Referee.h"

#include "uci/Options.h"

#include <algorithm>
#include <math.h>

void PVSEngine::createHelperInstances(size_t numThreads) {
    #if not defined(DISABLE_THREADS)
        for(size_t i = 0; i < numThreads; i++) {
            #if defined(USE_HCE) && defined(TUNE)
            // Erstelle eine Hilfsinstanz mit HCE-Parametern
            instances.push_back(new PVSSearchInstance(board, hceParams, transpositionTable, stopFlag, startTime,
                                                      stopTime, nodesSearched, nullptr));
            #else
            // Erstelle eine Hilfsinstanz
            instances.push_back(new PVSSearchInstance(board, transpositionTable, stopFlag, startTime,
                                                      stopTime, nodesSearched, nullptr));
            #endif

            instances[i]->setMainThread(false);
        }
    #else
        UNUSED(numThreads);
    #endif
}

void PVSEngine::destroyHelperInstances() {
    #if not defined(DISABLE_THREADS)
        for(PVSSearchInstance* instance : instances)
            delete instance;

        instances.clear();
    #endif
}

void helperThreadLoop(PVSSearchInstance* instance, int16_t depth, int16_t alpha, int16_t beta) {
    int16_t score;

    // Schleife, die die Aspirationsfenster erweitert,
    // wenn die Bewertung außerhalb des Fensters liegt
    do {
        score = instance->pvs(depth * ONE_PLY, 0, alpha, beta, 0, PV_NODE);

        bool alphaAlreadyWidened = false, betaAlreadyWidened = false;

        while(score <= alpha || score >= beta) {
            if(score <= alpha) {
                if(alphaAlreadyWidened)
                    alpha = MIN_SCORE;
                else {
                    alphaAlreadyWidened = true;
                    alpha -= PVSEngine::WIDENED_ASPIRATION_WINDOW - PVSEngine::ASPIRATION_WINDOW;
                }
            } else {
                if(betaAlreadyWidened)
                    beta = MAX_SCORE;
                else {
                    betaAlreadyWidened = true;
                    beta += PVSEngine::WIDENED_ASPIRATION_WINDOW - PVSEngine::ASPIRATION_WINDOW;
                }
            }

            score = instance->pvs(depth * ONE_PLY, 0, alpha, beta, 0, PV_NODE);
        }

        alpha = score - PVSEngine::ASPIRATION_WINDOW;
        beta = score + PVSEngine::ASPIRATION_WINDOW;
    } while(!instance->shouldStop());
}

void PVSEngine::startHelperThreads(int16_t depth, int16_t alpha, int16_t beta, const Array<Move, 256>& searchMoves) {
    #if not defined(DISABLE_THREADS)
        for(PVSSearchInstance* instance : instances) {
            instance->resetSelectiveDepth();
            instance->setSearchMoves(searchMoves);

            // Starte den Hilfsthread
            threads.push_back(std::thread(helperThreadLoop, instance, depth, alpha, beta));
        }
    #else
        UNUSED(depth);
        UNUSED(alpha);
        UNUSED(beta);
        UNUSED(searchMoves);
    #endif
}

void PVSEngine::stopHelperThreads() {
    #if not defined(DISABLE_THREADS)
        if(stopFlag.load()) {
            // Fall 1: Die Suche wurde global gestoppt

            for(std::thread& thread: threads)
                thread.join();

            threads.clear();
        } else {
            // Fall 2: Die Suche wurde nicht global gestoppt, sondern
            // nur die Hilfsthreads sollen gestoppt werden

            stopFlag.store(true);

            for(std::thread& thread: threads)
                thread.join();

            threads.clear();
            stopFlag.store(false);
        }
    #endif
}

void PVSEngine::outputSearchInfo() {
    // Bestimme die textuelle Repräsentation der Bewertung.
    // Mattbewertungen werden in der Form "mate x" ausgegeben,
    // wobei x die Anzahl der Züge bis zum Matt ist.
    int16_t score = getBestMoveScore();
    std::string scoreStr;
    if(isMateScore(score)) {
        scoreStr = "mate ";
        if(score < 0)
            scoreStr += "-";
        scoreStr += std::to_string(isMateIn(score));
    } else {
        scoreStr = "cp " + std::to_string(score);
    }

    // Bestimme die selektive Tiefe.
    uint16_t selectiveDepth = variations.size() > 0 ? variations[0].selectiveDepth : 0;
    for(size_t i = 1; i < variations.size(); i++)
        selectiveDepth = std::max(selectiveDepth, variations[i].selectiveDepth);

    std::chrono::milliseconds timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime.load());

    // Gebe die Informationen zur Suche aus.
    std::cout << "info depth " << maxDepthReached << " seldepth " << selectiveDepth << " score " << scoreStr << " nodes " << nodesSearched.load() <<
                 " time " << timeElapsed.count() << " nps " << (uint64_t)(nodesSearched.load() / (timeElapsed.count() / 1000.0)) <<
                 " hashfull " << (uint32_t)((double)transpositionTable.getEntriesWritten() / (double)transpositionTable.getCapacity() * 1000.0) <<
                 " pv ";

    // Gebe die Hauptvariante aus.
    for(Move move : variations[0].moves)
        std::cout << move.toString() << " ";

    std::cout << std::endl;

    lastOutputTime = std::chrono::system_clock::now();
}

void PVSEngine::outputNodesInfo() {
    std::chrono::milliseconds timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime.load());

    // Gebe nur die Anzahl der Knoten, die Suchzeit, die Knoten pro Sekunde
    // und wie voll die Transpositionstabelle ist, aus.
    std::cout << "info nodes " << nodesSearched.load() << " time " << timeElapsed.count() << " nps " <<
                 (uint64_t)(nodesSearched.load() / (timeElapsed.count() / 1000.0)) <<
                 " hashfull " << (uint32_t)((double)transpositionTable.getEntriesWritten() / (double)transpositionTable.getCapacity() * 1000.0) << std::endl;

    lastOutputTime = std::chrono::system_clock::now();
}

void PVSEngine::outputMultiPVInfo(size_t pvIndex) {
    // Bestimme die textuelle Repräsentation der Bewertung.
    int16_t score = variations[pvIndex].score;
    std::string scoreStr;
    if(isMateScore(score)) {
        scoreStr = "mate ";
        if(score < 0)
            scoreStr += "-";
        scoreStr += std::to_string(isMateIn(score));
    } else {
        scoreStr = "cp " + std::to_string(score);
    }

    // Gebe eine Variante im Multi-PV-Modus aus.
    std::cout << "info multipv " << pvIndex + 1 << " score " << scoreStr << " pv ";

    for(Move move : variations[pvIndex].moves)
        std::cout << move.toString() << " ";

    std::cout << std::endl;
}

void PVSEngine::search(const UCI::SearchParams& params) {
    // Setze die Flags und Variablen für die Suche zurück.
    stopFlag.store(false);
    searching = true;
    isTimeControlled = params.useWBTime;
    isPondering.store(params.ponder);
    nodesSearched.store(0);
    maxDepthReached = 0;
    clearPVHistory();
    variations.clear();

    // Bestimme die Start- und Stopzeit der Suche.
    startTime.store(std::chrono::system_clock::now());
    calculateTimeLimits(params);
    stopTime.store(startTime.load() + timeMax);
    lastOutputTime = startTime.load();

    // Generiere die Liste der legalen Züge in der aktuellen Position.
    Array<Move, 256> legalMoves;
    board.generateLegalMoves(legalMoves);

    // Wenn keine legalen Züge vorhanden sind,
    // gebe den Nullzug aus und beende die Suche.
    if(legalMoves.size() == 0) {
        #if not defined(TUNE)
        std::cout << "bestmove 0000" << std::endl;
        #endif
        return;
    }

    // Die Funktion, die in regelmäßigen Abständen aufgerufen wird,
    // um zu überprüfen, ob die Suche abgebrochen werden soll.
    std::function<void()> checkupFunction = [&]() {
        if(isCheckupTime()) {
            lastCheckupTime = std::chrono::system_clock::now();

            if((lastCheckupTime >= stopTime.load() && maxDepthReached > 4) || // Die Zeit ist abgelaufen (und es wurde mindestens Tiefe 5 erreicht)
               nodesSearched.load() >= params.nodes) // Die Knotenanzahl wurde erreicht
                stop();

            // Mindestens alle 2 Sekunden die Ausgabe aktualisieren
            #if not defined(TUNE)
            if(lastCheckupTime >= lastOutputTime + std::chrono::milliseconds(MAX_TIME_BETWEEN_OUTPUTS))
                outputNodesInfo();
            #endif
            
            if(checkupCallback)
                checkupCallback();
        }
    };

    // Erstelle die Hauptinstanz, die die Suche durchführt.
    #if defined(USE_HCE) && defined(TUNE)
    // Erstelle die Hauptinstanz mit HCE-Parametern für das Tuning
    PVSSearchInstance mainInstance(board, hceParams, transpositionTable, stopFlag, startTime,
                                   stopTime, nodesSearched, checkupFunction);
    #else
    PVSSearchInstance mainInstance(board, transpositionTable, stopFlag, startTime,
                                   stopTime, nodesSearched, checkupFunction);
    #endif
    
    this->mainInstance = &mainInstance;
    mainInstance.setMainThread(true);

    // Erstelle die Hilfsinstanzen, die die Hauptinstanz unterstützen.
    size_t numAdditionalInstances = UCI::options["Threads"].getValue<size_t>() - 1;
    createHelperInstances(numAdditionalInstances);

    // Bestimme die Multi-PV-Einstellung.
    size_t multiPV = UCI::options["MultiPV"].getValue<size_t>();
    multiPV = std::min(multiPV, legalMoves.size());
    if(params.searchmoves.size() > 0)
        multiPV = std::min(multiPV, params.searchmoves.size());

    /**
     * Iterative Tiefensuche:
     * Starte die Suche mit einer Tiefe von 1 und erhöhe die Tiefe
     * bis Suche durch die Zeitkontrolle oder einem anderen Kriterium
     * abgebrochen wird. Wenn die maximale Tiefe erreicht wurde, wird
     * die Suche ebenfalls beeendet.
     */
    for(int16_t depth = 1; depth < MAX_PLY; depth++) {

        // Eine Liste von Zügen, die bestimmt welche Züge in
        // einem Durchlauf betrachtet werden dürfen.
        Array<Move, 256> searchMoves;
        if(params.searchmoves.size() > 0)
            searchMoves = params.searchmoves;
        else
            searchMoves = legalMoves;

        // Wenn die Multi-PV-Einstellung aktiviert ist, wird für
        // jede PV eine eigene Suche durchgeführt.
        for(size_t pv = 0; pv < multiPV; pv++) {
            int16_t alpha = MIN_SCORE, beta = MAX_SCORE;

            // In allen Durchläufen mit Tiefe > 1 wird die Suche mit
            // einem Aspirationsfenster gestartet.
            if(depth > 1) {
                int32_t prevScore = variations[pv].score;
                alpha = prevScore - prevScore * ASPIRATION_WINDOW_SCORE_FACTOR - ASPIRATION_WINDOW;
                beta = prevScore + prevScore * ASPIRATION_WINDOW_SCORE_FACTOR + ASPIRATION_WINDOW;
            }

            // Starte die Hilfsthreads.
            startHelperThreads(depth, alpha, beta, searchMoves);

            // Initialisiere die Hauptinstanz für diesen Durchlauf.
            mainInstance.resetSelectiveDepth();
            mainInstance.setSearchMoves(searchMoves);

            // Führe die Suche in der Hauptinstanz durch.
            int16_t score = mainInstance.pvs(depth * ONE_PLY, 0, alpha, beta, 0, PV_NODE);

            // Aspirationsfenster erweitern, wenn die Bewertung außerhalb des Fensters liegt.
            bool alphaAlreadyWidened = false, betaAlreadyWidened = false;
            while(score <= alpha || score >= beta) {
                if(score <= alpha) {
                    if(alphaAlreadyWidened)
                        alpha = MIN_SCORE;
                    else {
                        alphaAlreadyWidened = true;
                        alpha -= WIDENED_ASPIRATION_WINDOW - ASPIRATION_WINDOW;
                    }
                } else {
                    if(betaAlreadyWidened)
                        beta = MAX_SCORE;
                    else {
                        betaAlreadyWidened = true;
                        beta += WIDENED_ASPIRATION_WINDOW - ASPIRATION_WINDOW;
                    }
                }

                score = mainInstance.pvs(depth * ONE_PLY, 0, alpha, beta, 0, PV_NODE);
            }

            // Stoppe die Hilfsthreads.
            stopHelperThreads();

            // Speichere die Hauptvariante und die Bewertung.
            std::vector<Move> pvMoves;
            for(Move move : mainInstance.getPV())
                pvMoves.push_back(move);

            Move bestMove = pvMoves[0];

            // Im Multi-PV-Modus muss die Hauptvariante sortiert in
            // die Liste der Varianten eingefügt werden.
            size_t variationSize = variations.size();
            for(size_t i = 0; i <= variationSize; i++) {
                if(i == variations.size()) {
                    if(variations.size() < multiPV) {
                        variations.push_back({
                            pvMoves,
                            mainInstance.getPVScore(),
                            (uint16_t)depth,
                            getSelectiveDepth()
                        });
                    } else {
                        variations[i - 1] = {
                            pvMoves,
                            mainInstance.getPVScore(),
                            (uint16_t)depth,
                            getSelectiveDepth()
                        };
                    }
                } else if(variations[i].moves.front() == bestMove) {
                    variations[i] = {
                        pvMoves,
                        mainInstance.getPVScore(),
                        (uint16_t)depth,
                        getSelectiveDepth()
                    };
                    break;
                }
            }

            std::stable_sort(variations.begin(), variations.end(), [](const Variation& a, const Variation& b) {
                if(a.depth == b.depth)
                    return a.score > b.score;
                else
                    return a.depth > b.depth;
            });

            // Entferne den besten Zug aus der Liste der Züge,
            // die betrachtet werden sollen.
            // Wenn wir das nicht machen, enthält im Multi-PV-Modus
            // jede Variante die gleichen Züge.
            searchMoves.remove_first(bestMove);

            // Soll die Suche abgebrochen werden?
            if(stopFlag.load() && maxDepthReached > 4)
                break;
        }

        // Soll die Suche abgebrochen werden?
        if(stopFlag.load() && maxDepthReached > 4)
            break;

        // Wir haben eine Tiefe vollständig durchsucht.
        maxDepthReached = depth;

        // Aktualisiere die PV-Historie für
        // die dynamische Zeitkontrolle.
        if(pvHistory.size() == 5)
            pvHistory.shiftLeft(0);

        pvHistory.push_back({
            getBestMove(),
            getBestMoveScore()
        });

        #if not defined(TUNE)
        // Im Multi-PV-Modus werden Informationen zu den
        // einzelnen Varianten ausgegeben.
        if(multiPV > 1)
            for(size_t i = 0; i < multiPV; i++)
                outputMultiPVInfo(i);

        // Gebe die Suchinformationen zu dieser Tiefe aus.
        outputSearchInfo();
        #endif

        // Sagt die dynamische Zeitkontrolle, dass die Suche
        // abgebrochen werden soll?
        if(!isPondering.load() && !extendSearch(isTimeControlled))
            break;

        // Soll die Suche aufgrund anderer Kriterien abgebrochen werden?
        if((uint32_t)maxDepthReached >= params.depth || // Die Zieltiefe wurde erreicht
           (uint32_t)isMateIn(getBestMoveScore()) <= params.mate) // Das Zielmatt wurde gefunden
            break;
    }

    // Wir suchen nicht mehr.
    stopFlag.store(false);
    searching = false;

    // Gebe alle Hilfsinstanzen frei.
    destroyHelperInstances();
    
    #if not defined(TUNE)
    // Finale Ausgabe der Suchinformationen.
    outputSearchInfo();

    std::cout << "\n" << "bestmove " << getBestMove().toString();

    if(UCI::options["Ponder"].getValue<bool>()) {
        std::vector<Move> pv = getPrincipalVariation();
        if(pv.size() > 1)
            std::cout << " ponder " << pv[1].toString();
    }

    std::cout << std::endl;
    #endif
}

void PVSEngine::calculateTimeLimits(const UCI::SearchParams& params) {
    if(params.useMovetime) {
        // Wir sollen eine feste Zeit suchen.
        timeMin = std::chrono::milliseconds(params.movetime);
        timeMax = std::chrono::milliseconds(params.movetime);
    } else if(params.useWBTime) {
        // Wir sollen mit dynamischer Zeitkontrolle suchen.

        // Berechne die minimale und maximale Zeit, die die Suche dauern soll.
        uint32_t time = board.getSideToMove() == WHITE ? params.wtime : params.btime;
        size_t numLegalMoves = board.generateLegalMoves().size();

        double oneThirtiethOfTime = time * 0.0333;
        double oneFourthOfTime = time * 0.25;

        uint32_t minTime = oneThirtiethOfTime - oneThirtiethOfTime * std::exp(-0.05  * numLegalMoves);
        uint32_t maxTime = oneFourthOfTime - oneFourthOfTime * std::exp(-0.05  * numLegalMoves);

        timeMin = std::chrono::milliseconds(minTime);
        timeMax = std::chrono::milliseconds(maxTime);
    } else {
        // Keine Zeitkontrolle.
        timeMin = std::chrono::milliseconds(std::numeric_limits<uint32_t>::max());
        timeMax = std::chrono::milliseconds(std::numeric_limits<uint32_t>::max());
    }
}

bool PVSEngine::extendSearch(bool isTimeControlled) {
    if(maxDepthReached < 5)
        return true;

    if(stopFlag.load())
        return false;

    std::chrono::milliseconds timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime.load());

    // Überprüfe, ob wir außerhalb [timeMin, timeMax] liegen.
    if(timeElapsed >= timeMax)
        return false;
    else if(timeElapsed < timeMin || !isTimeControlled)
        return true;

    // Bestimme die Varianz der Bewertungen der letzten 5 Tiefen
    // und wie häufig sich der beste Zug geändert hat.
    int32_t meanScore = 0;
    for(MoveScorePair& pair : pvHistory)
        meanScore += pair.score;

    if(pvHistory.size() > 0)
        meanScore /= pvHistory.size();

    int32_t scoreVariance = 0;
    for(MoveScorePair& pair : pvHistory)
        scoreVariance += (pair.score - meanScore) *
                         (pair.score - meanScore);

    if(pvHistory.size() > 1)
        scoreVariance /= pvHistory.size() - 1;

    int32_t bestMoveChanges = 0;

    for(size_t i = 0; i < pvHistory.size() - 1; i++)
        if(pvHistory[i].move != pvHistory[i + 1].move)
            bestMoveChanges++;

    // Bestimme, wie weit wir zwischen timeMin und timeMax liegen.
    double timeFactor = ((double)timeElapsed.count() - (double)timeMin.count()) /
                        ((double)timeMax.count() - (double)timeMin.count());

    timeFactor = std::clamp(timeFactor, 0.0, 1.0);

    // Überprüfe, ob die Suche verlängert werden soll.
    if(bestMoveChanges >= 4)
        return true;

    if(bestMoveChanges >= 3 && scoreVariance >= 40 * 40 * timeFactor)
        return true;

    if(bestMoveChanges >= 2 && scoreVariance >= 60 * 60 * timeFactor)
        return true;

    if(bestMoveChanges >= 1 && scoreVariance >= 80 * 80 * timeFactor)
        return true;

    return false;
}