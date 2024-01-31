#include "core/engine/search/PVSEngine.h"
#include "core/chess/Referee.h"

#include "uci/Options.h"

#include <algorithm>
#include <math.h>

void PVSEngine::createHelperThreads(size_t numThreads, const UCI::SearchParams& params) {
    #if not defined(DISABLE_THREADS)
        for(size_t i = 0; i < numThreads; i++) {
            instances.push_back(new PVSSearchInstance(board, transpositionTable, stopFlag, startTime, stopTime,
                                                      nodesSearched, params.searchmoves, nullptr));

            instances[i]->setMainThread(false);
            instances[i]->setMersenneTwisterSeed(i);
        }
    #else
        UNUSED(numThreads);
        UNUSED(params);
    #endif
}

void PVSEngine::destroyHelperThreads() {
    #if not defined(DISABLE_THREADS)
        for(PVSSearchInstance* instance : instances)
            delete instance;

        instances.clear();
    #endif
}

void helperThreadLoop(PVSSearchInstance* instance, int16_t depth, int16_t alpha, int16_t beta) {
    int16_t score;

    do {
        score = instance->pvs(depth * ONE_PLY, 0, alpha, beta, false, PV_NODE);

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

            score = instance->pvs(depth * ONE_PLY, 0, alpha, beta, false, PV_NODE);
        }

        alpha = score - PVSEngine::ASPIRATION_WINDOW;
        beta = score + PVSEngine::ASPIRATION_WINDOW;
    } while(!instance->shouldStop());
}

void PVSEngine::startHelperThreads(int16_t depth, int16_t alpha, int16_t beta) {
    #if not defined(DISABLE_THREADS)
        for(PVSSearchInstance* instance : instances)
            threads.push_back(std::thread(helperThreadLoop, instance, depth, alpha, beta));
    #else
        UNUSED(depth);
        UNUSED(alpha);
        UNUSED(beta);
    #endif
}

void PVSEngine::stopHelperThreads() {
    #if not defined(DISABLE_THREADS)
        if(stopFlag.load()) {
            for(std::thread& thread: threads)
                thread.join();

            threads.clear();
        } else {
            stopFlag.store(true);

            for(std::thread& thread: threads)
                thread.join();

            threads.clear();
            stopFlag.store(false);
        }
    #endif
}

void PVSEngine::search(const UCI::SearchParams& params) {
    stopFlag.store(false);
    searching = true;
    isTimeControlled = params.useWBTime;
    isPondering.store(params.ponder);
    nodesSearched.store(0);
    maxDepthReached = 0;

    clearPVHistory();
    variations.clear();

    startTime.store(std::chrono::system_clock::now());
    calculateTimeLimits(params);
    stopTime.store(startTime.load() + timeMax);

    std::function<void()> checkupFunction = [&]() {
        if(isCheckupTime()) {
            lastCheckupTime = std::chrono::system_clock::now();

            if((lastCheckupTime >= stopTime.load() && maxDepthReached > 0) || // Die Zeit ist abgelaufen
               nodesSearched.load() >= params.nodes || // Die Knotenanzahl wurde erreicht
               (uint32_t)maxDepthReached >= params.depth || // Die Zieltiefe wurde erreicht
               (uint32_t)isMateIn(getBestMoveScore()) <= params.mate) // Das Zielmatt wurde gefunden
                stop();
            
            if(checkupCallback)
                checkupCallback();
        }
    };

    PVSSearchInstance mainInstance(board, transpositionTable, stopFlag, startTime, stopTime,
                                   nodesSearched, params.searchmoves, checkupFunction);

    mainInstance.setMainThread(true);

    size_t numAdditionalThreads = UCI::options["Threads"].getValue<size_t>() - 1;
    createHelperThreads(numAdditionalThreads, params);

    int16_t alpha = MIN_SCORE, beta = MAX_SCORE;

    for(int16_t depth = 1; depth <= MAX_PLY; depth++) {
        startHelperThreads(depth, alpha, beta);
        int16_t score = mainInstance.pvs(depth * ONE_PLY, 0, alpha, beta, false, PV_NODE);

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

            score = mainInstance.pvs(depth * ONE_PLY, 0, alpha, beta, false, PV_NODE);
        }

        stopHelperThreads();

        alpha = score - score * ASPIRATION_WINDOW_SCORE_FACTOR - ASPIRATION_WINDOW;
        beta = score + score * ASPIRATION_WINDOW_SCORE_FACTOR + ASPIRATION_WINDOW;

        variations.clear();

        std::vector<Move> pvMoves;
        for(Move move : mainInstance.getPV())
            pvMoves.push_back(move);

        variations.push_back({
            pvMoves,
            mainInstance.getPVScore()
        });

        if(stopFlag.load() && maxDepthReached > 0)
            break;
        else {
            maxDepthReached = depth;

            if(pvHistory.size() == 5)
                pvHistory.shiftLeft(0);

            pvHistory.push_back({
                getBestMove(),
                getBestMoveScore()
            });
        }

        score = getBestMoveScore();
        std::string scoreStr;
        if(isMateScore(score)) {
            scoreStr = "mate ";
            if(score < 0)
                scoreStr += "-";
            scoreStr += std::to_string(isMateIn(score));
        } else {
            scoreStr = "cp " + std::to_string(score);
        }

        std::chrono::milliseconds timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime.load());
        std::cout << "info depth " << depth << " score " << scoreStr << " nodes " << nodesSearched.load() <<
                    " time " << timeElapsed.count() << " nps " << (uint64_t)(nodesSearched.load() / (timeElapsed.count() / 1000.0)) <<
                    " pv ";
        for(Move move : variations[0].moves)
            std::cout << move.toString() << " ";

        std::cout << std::endl;

        if(!isPondering.load() && !extendSearch(isTimeControlled))
            break;
    }

    stopFlag.store(false);
    searching = false;

    destroyHelperThreads();

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

    std::chrono::milliseconds timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime.load());
    std::cout << "info depth " << maxDepthReached << " score " << scoreStr << " nodes " << nodesSearched.load() <<
                " time " << timeElapsed.count() << " nps " << (uint64_t)(nodesSearched.load() / (timeElapsed.count() / 1000.0)) <<
                " pv ";

    for(Move move : variations[0].moves)
        std::cout << move.toString() << " ";

    std::cout << "\n" << "bestmove " << getBestMove().toString();

    std::vector<Move> pv = getPrincipalVariation();
    if(pv.size() > 1)
        std::cout << " ponder " << pv[1].toString();

    std::cout << std::endl;
}

void PVSEngine::calculateTimeLimits(const UCI::SearchParams& params) {
    if(params.useMovetime) {
        timeMin = std::chrono::milliseconds(params.movetime);
        timeMax = std::chrono::milliseconds(params.movetime);
    } else if(params.useWBTime) {
        // Berechne die minimale und maximale Zeit, die die Suche dauern soll
        uint32_t time = board.getSideToMove() == WHITE ? params.wtime : params.btime;
        size_t numLegalMoves = board.generateLegalMoves().size();

        double oneThirtiethOfTime = time * 0.0333;
        double oneFourthOfTime = time * 0.25;

        uint32_t minTime = oneThirtiethOfTime - oneThirtiethOfTime * std::exp(-0.05  * numLegalMoves);
        uint32_t maxTime = oneFourthOfTime - oneFourthOfTime * std::exp(-0.05  * numLegalMoves);

        timeMin = std::chrono::milliseconds(minTime);
        timeMax = std::chrono::milliseconds(maxTime);
    } else {
        // Keine Zeitkontrolle
        timeMin = std::chrono::milliseconds(std::numeric_limits<uint32_t>::max());
        timeMax = std::chrono::milliseconds(std::numeric_limits<uint32_t>::max());
    }
}

bool PVSEngine::extendSearch(bool isTimeControlled) {
    if(stopFlag.load())
        return false;

    std::chrono::milliseconds timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime.load());

    if(timeElapsed >= timeMax)
        return false;
    else if(timeElapsed < timeMin || !isTimeControlled)
        return true;

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

    double timeFactor = ((double)timeElapsed.count() - (double)timeMin.count()) /
                        ((double)timeMax.count() - (double)timeMin.count());

    timeFactor = std::clamp(timeFactor, 0.0, 1.0);

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