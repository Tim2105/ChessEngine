#include "core/engine/search/PVSEngine.h"
#include "core/chess/Referee.h"

#include "uci/Options.h"

#include <algorithm>
#include <math.h>

void PVSEngine::helperThreadLoop(size_t instanceIdx) {
    #if not defined(DISABLE_THREADS)
        int score;

        do {
            // Warte auf die Bedingungsvariable
            std::unique_lock<std::mutex> lock(cvMutex);
            cv.wait(lock, [&]() { return !threadSleepFlag.load(); });

            // Der Thread ist jetzt beschäftigt
            numThreadsBusy.fetch_add(1);
            lock.unlock();

            // Hole die Instanz, die der Thread ausführen soll
            PVSSearchInstance* instance = instances[instanceIdx];

            // Kopiere die aktuellen Suchparameter in lokale Variablen
            int depth = currentDepth;
            int alpha = currentAlpha;
            int beta = currentBeta;

            // Schleife, die die Aspirationsfenster erweitert,
            // wenn die Bewertung außerhalb des Fensters liegt
            while(!(instance->shouldStop() || exitSearch.load())) {
                score = instance->pvs(depth, 0, alpha, beta, PV_NODE);

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

                    score = instance->pvs(depth, 0, alpha, beta, PV_NODE);
                }

                alpha = score - PVSEngine::ASPIRATION_WINDOW;
                beta = score + PVSEngine::ASPIRATION_WINDOW;
            }

            // Der Thread ist fertig und wartet auf die nächste Suche
            cvMutex.lock();
            numThreadsBusy.fetch_sub(1);
            cvMutex.unlock();
            cvMain.notify_one();

        } while(!exitSearch.load());
    #else
        UNUSED(instance);
    #endif
}

void PVSEngine::createHelperInstances(size_t numThreads) {
    #if not defined(DISABLE_THREADS)
        for(size_t i = 0; i < numThreads; i++) {
            #if defined(USE_HCE) && defined(TUNE)
            // Erstelle eine Hilfsinstanz mit HCE-Parametern
            instances.push_back(new PVSSearchInstance(board, hceParams, transpositionTable, threadSleepFlag, startTime,
                                                      stopTime, nodesSearched, nullptr));
            #else
            // Erstelle eine Hilfsinstanz
            instances.push_back(new PVSSearchInstance(board, transpositionTable, threadSleepFlag, startTime,
                                                      stopTime, nodesSearched, nullptr));
            #endif

            instances[i]->setMainThread(false);

            // Erstelle einen Hilfsthread
            threads.push_back(std::thread(&PVSEngine::helperThreadLoop, this, i));
        }
    #else
        UNUSED(numThreads);
    #endif
}

void PVSEngine::destroyHelperInstances() {
    #if not defined(DISABLE_THREADS)
        // Wenn die Threads noch laufen, stoppe sie
        if(!exitSearch.load()) {
            exitSearch.store(true);

            cvMutex.lock();
            threadSleepFlag.store(false);
            cvMutex.unlock();
            cv.notify_all();
        }

        // Joine alle Hilfsthreads
        for(std::thread& thread : threads)
            thread.join();

        threads.clear();

        for(PVSSearchInstance* instance : instances)
            delete instance;

        instances.clear();
    #endif
}

void PVSEngine::startHelperThreads(int depth, int alpha, int beta, const Array<Move, 256>& searchMoves) {
    #if not defined(DISABLE_THREADS)
        currentDepth = depth;
        currentAlpha = alpha;
        currentBeta = beta;

        // Bereite die Hilfsinstanzen für die nächste Suche vor
        for(PVSSearchInstance* instance : instances) {
            instance->resetSelectiveDepth();
            instance->setSearchMoves(searchMoves);
        }

        // Wecke alle Hilfsthreads auf
        cvMutex.lock();
        threadSleepFlag.store(false);
        cvMutex.unlock();
        cv.notify_all();
    #else
        UNUSED(depth);
        UNUSED(alpha);
        UNUSED(beta);
        UNUSED(searchMoves);

        threadSleepFlag.store(false);
    #endif
}

void PVSEngine::pauseHelperThreads() {
    #if not defined(DISABLE_THREADS)
        threadSleepFlag.store(true);

        // Warte, bis alle Hilfsthreads zurückgekehrt sind
        std::unique_lock<std::mutex> lock(cvMutex);
        cvMain.wait(lock, [&]() { return numThreadsBusy.load() == 0; });
        lock.unlock();
    #else
        threadSleepFlag.store(true);
    #endif
}

int threadValue(PVSSearchInstance* instance, int worstScore) {
    return (instance->getPVScore() - worstScore + 10) * (instance->getPV().size() > 0);
}

void PVSEngine::outputSearchInfo() {
    // Bestimme die textuelle Repräsentation der Bewertung.
    // Mattbewertungen werden in der Form "mate x" ausgegeben,
    // wobei x die Anzahl der Züge bis zum Matt ist.
    int score = getBestMoveScore();
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
    int selectiveDepth = variations.size() > 0 ? variations[0].selectiveDepth : 0;
    for(size_t i = 1; i < variations.size(); i++)
        selectiveDepth = std::max(selectiveDepth, variations[i].selectiveDepth);

    std::chrono::milliseconds timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime.load());

    // Gebe die Informationen zur Suche aus.
    std::cout << "info depth " << maxDepthReached << " seldepth " << selectiveDepth << " score " << scoreStr << " nodes " << nodesSearched.load() <<
                 " time " << timeElapsed.count() << " nps " << (uint64_t)(nodesSearched.load() / (timeElapsed.count() / 1000.0)) <<
                 " hashfull " << (unsigned int)((double)transpositionTable.getEntriesWritten() / (double)transpositionTable.getCapacity() * 1000.0) <<
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
                 " hashfull " << (unsigned int)((double)transpositionTable.getEntriesWritten() / (double)transpositionTable.getCapacity() * 1000.0) << std::endl;

    lastOutputTime = std::chrono::system_clock::now();
}

void PVSEngine::outputMultiPVInfo(size_t pvIndex) {
    // Bestimme die textuelle Repräsentation der Bewertung.
    int score = variations[pvIndex].score;
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
    exitSearch.store(false);
    searching.store(true);
    isTimeControlled = params.useWBTime;
    isPondering.store(params.ponder);
    nodesSearched.store(0);
    maxDepthReached = 0;
    clearPVHistory();
    variations.clear();

    #if not defined(DISABLE_THREADS)
        numThreadsBusy.store(0);
    #endif

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
        mainInstance = new PVSSearchInstance(board, hceParams, transpositionTable, threadSleepFlag, startTime,
                                             stopTime, nodesSearched, checkupFunction);
    #else
        mainInstance = new PVSSearchInstance(board, transpositionTable, threadSleepFlag, startTime,
                                             stopTime, nodesSearched, checkupFunction);
    #endif
    
    mainInstance->setMainThread(true);

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
    for(int depth = 1; depth < MAX_PLY; depth++) {

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
            int alpha = MIN_SCORE, beta = MAX_SCORE;

            // In allen Durchläufen mit Tiefe > 1 wird die Suche mit
            // einem Aspirationsfenster gestartet.
            if(depth > 1) {
                int prevScore = variations[pv].score;
                alpha = prevScore - prevScore * ASPIRATION_WINDOW_SCORE_FACTOR - ASPIRATION_WINDOW;
                beta = prevScore + prevScore * ASPIRATION_WINDOW_SCORE_FACTOR + ASPIRATION_WINDOW;

                // Set the first PV-Table entry as the hash move,
                // so that it is searched first in the next iteration.
                if(variations[pv].moves.size() > 0) {
                    Move hashMove = variations[pv].moves[0];
                    mainInstance->setBestRootMoveHint(hashMove);

                    for(size_t i = 0; i < numAdditionalInstances; i++)
                        instances[i]->setBestRootMoveHint(hashMove);
                }
            }

            // Starte die Hilfsthreads.
            startHelperThreads(depth, alpha, beta, searchMoves);

            // Initialisiere die Hauptinstanz für diesen Durchlauf.
            mainInstance->resetSelectiveDepth();
            mainInstance->setSearchMoves(searchMoves);

            // Führe die Suche in der Hauptinstanz durch.
            int score = mainInstance->pvs(depth, 0, alpha, beta, PV_NODE);

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

                score = mainInstance->pvs(depth, 0, alpha, beta, PV_NODE);
            }

            // Stoppe die Hilfsthreads.
            pauseHelperThreads();

            // Bestimme die Hauptvariante durch Voting.
            int voteMap[64][64] = {0};

            // Bestimme die schlechteste Bewertung.
            int worstScore = mainInstance->getPVScore();
            voteMap[mainInstance->getPV()[0].getOrigin()][mainInstance->getPV()[0].getDestination()] = 0;
            for(size_t i = 0; i < numAdditionalInstances; i++) {
                worstScore = std::min(worstScore, instances[i]->getPVScore());
                voteMap[instances[i]->getPV()[0].getOrigin()][instances[i]->getPV()[0].getDestination()] = 0;
            }

            // Fülle die Voting-Map.
            voteMap[mainInstance->getPV()[0].getOrigin()][mainInstance->getPV()[0].getDestination()] = threadValue(mainInstance, worstScore);
            for(size_t i = 0; i < numAdditionalInstances; i++)
                voteMap[instances[i]->getPV()[0].getOrigin()][instances[i]->getPV()[0].getDestination()] += threadValue(instances[i], worstScore);

            // Bestimme die beste Instanz.
            int bestInstanceIdx = -1;
            int voteMapPeak = voteMap[mainInstance->getPV()[0].getOrigin()][mainInstance->getPV()[0].getDestination()];
            int bestPVScore = mainInstance->getPVScore();
            for(size_t i = 0; i < numAdditionalInstances; i++) {
                int pvScore = instances[i]->getPVScore();
                if(isMateScore(pvScore) || isMateScore(bestPVScore)) {
                    // Wähle immer das schnellste Matt für uns
                    // oder das langsamste Matt gegen uns.
                    if(pvScore > bestPVScore) {
                        bestInstanceIdx = i;
                        bestPVScore = pvScore;
                    }
                } else {
                    // Verwende sonst die Voting-Heuristik.
                    int voteScore = voteMap[instances[i]->getPV()[0].getOrigin()][instances[i]->getPV()[0].getDestination()];
                    PVSSearchInstance* bestInstance = bestInstanceIdx != -1 ? instances[bestInstanceIdx] : mainInstance;
                    if(voteScore > voteMapPeak ||
                      (voteScore == voteMapPeak && threadValue(instances[i], worstScore) > threadValue(bestInstance, worstScore))) {
                        bestInstanceIdx = i;
                        voteMapPeak = voteScore;
                        bestPVScore = pvScore;
                    }
                }
            }

            // Tausche die Hauptinstanz mit der besten Instanz aus.
            if(bestInstanceIdx != -1) {
                PVSSearchInstance* bestInstance = instances[bestInstanceIdx];
                instances[bestInstanceIdx] = mainInstance;
                instances[bestInstanceIdx]->setMainThread(false);
                instances[bestInstanceIdx]->setCheckupFunction(nullptr);
                mainInstance = bestInstance;
                mainInstance->setMainThread(true);
                mainInstance->setCheckupFunction(checkupFunction);
            }

            // Speichere die Hauptvariante und die Bewertung.
            std::vector<Move> pvMoves;
            for(Move move : mainInstance->getPV())
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
                            mainInstance->getPVScore(),
                            depth,
                            getSelectiveDepth()
                        });
                    } else {
                        variations[i - 1] = {
                            pvMoves,
                            mainInstance->getPVScore(),
                            depth,
                            getSelectiveDepth()
                        };
                    }
                } else if(variations[i].moves.front() == bestMove) {
                    variations[i] = {
                        pvMoves,
                        mainInstance->getPVScore(),
                        depth,
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
            if(exitSearch.load() && maxDepthReached > 4)
                break;
        }

        // Soll die Suche abgebrochen werden?
        if(exitSearch.load() && maxDepthReached > 4)
            break;

        // Wir haben eine Tiefe vollständig durchsucht.
        maxDepthReached = depth;

        // Aktualisiere die PV-Historie für
        // die dynamische Zeitkontrolle.
        if(pvHistory.size() == 5)
            pvHistory.shiftLeft(0);

        pvHistory.push_back({
            getBestMove(),
            (int16_t)getBestMoveScore()
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
        if(maxDepthReached >= params.depth || // Die Zieltiefe wurde erreicht
           isMateIn(getBestMoveScore()) <= params.mate) // Das Zielmatt wurde gefunden
            break;
    }

    // Gebe alle Hilfsinstanzen frei.
    destroyHelperInstances();

    // Gebe die Hauptinstanz frei.
    delete mainInstance;

    // Wir suchen nicht mehr.
    searching.store(false);
    
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

    if(exitSearch.load())
        return false;

    std::chrono::milliseconds timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime.load());

    // Überprüfe, ob wir außerhalb [timeMin, timeMax] liegen.
    if(timeElapsed >= timeMax)
        return false;
    else if(timeElapsed < timeMin || !isTimeControlled)
        return true;

    // Bestimme die Varianz der Bewertungen der letzten 5 Tiefen
    // und wie häufig sich der beste Zug geändert hat.
    int meanScore = 0;
    for(MoveScorePair& pair : pvHistory)
        meanScore += pair.score;

    if(pvHistory.size() > 0)
        meanScore /= pvHistory.size();

    int scoreVariance = 0;
    for(MoveScorePair& pair : pvHistory)
        scoreVariance += (pair.score - meanScore) *
                         (pair.score - meanScore);

    if(pvHistory.size() > 1)
        scoreVariance /= pvHistory.size() - 1;

    int bestMoveChanges = 0;

    for(size_t i = 0; i < pvHistory.size() - 1; i++)
        if(pvHistory[i].move != pvHistory[i + 1].move)
            bestMoveChanges++;

    // Bestimme, wie weit wir zwischen timeMin und timeMax liegen.
    double timeFactor = (double)(timeElapsed.count() - timeMin.count()) /
                        (double)(timeMax.count() - timeMin.count());

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