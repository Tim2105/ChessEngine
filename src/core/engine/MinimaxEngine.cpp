#include "core/engine/MinimaxEngine.h"
#include "core/engine/PSQT.h"
#include "core/utils/MoveNotations.h"

#include <algorithm>
#include <cmath>

/**
 * @brief Setzt alle Vergangenheitsbewertungen auf 0 zurück.
 */
void MinimaxEngine::clearRelativeHistory() {
    for(int i = 0; i < 2; i++) {
        for(int j = 0; j < 64; j++) {
            for(int k = 0; k < 64; k++) {
                relativeHistory[i][j][k] = 0;
            }
        }
    }
}

/**
 * @brief Leert die PV-Tabelle.
 */
void MinimaxEngine::clearPvTable() {
    for(int i = 0; i < 64; i++) {
        pvTable[i].clear();
    }
}

/**
 * @brief Löscht alle Killerzüge.
 */
void MinimaxEngine::clearKillerMoves() {
    for(int i = 0; i < 256; i++) {
        killerMoves[i][0] = Move();
        killerMoves[i][1] = Move();
    }
}

/**
 * @brief Löscht alle Konterzüge.
 */
void MinimaxEngine::clearCounterMoves() {
    for(int i = 0; i < 15; i++)
        for(int j = 0; j < 64; j++)
            counterMoves[i][j] = Move();
}

/**
 * @brief Führt die Suche aus.
 * 
 * @param timeControl Gibt an, ob die Zeit eingeteilt werden soll.
 * @param minTime Die minimale Zeit, die die Suche dauern soll.
 * @param maxTime Die maximale Zeit, die die Suche dauern soll.
 */
void MinimaxEngine::runSearch(bool timeControl, uint32_t minTime, uint32_t maxTime) {
    searchRunning = true;

    // Enthält die PV aller vorherigen Suchtiefen
    std::vector<Variation> principalVariationHistory;

    int16_t score = 0;

    auto start = std::chrono::system_clock::now();

    for(int16_t depth = ONE_PLY; searchRunning && depth < (MAX_PLY * ONE_PLY); depth += ONE_PLY) {
        currentMaxDepth = depth;

        // Führe die eigentliche Suche durch
        score = rootSearch(depth, score);

        auto end = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        // Speichere die Ergebnisse der letzten Suche ab
        principalVariationHistory.push_back({
            getPrincipalVariation(),
            getBestMoveScore()
        });

        // Wenn die Zeit eingeteilt werden soll,
        // bestimme, ob die Suche verlängert werden soll
        if(timeControl && !extendSearchUnderTimeControl(principalVariationHistory, minTime, maxTime, elapsed))
            break;
    }

    // Wenn die Suche vorzeitig beendet wurde weil die maximale Tiefe erreicht wurde, dann wurde die letzte durchsuchte Tiefe vervollständigt
    if(searchRunning)
        currentMaxDepth += ONE_PLY;

    searchRunning = false;
}

bool MinimaxEngine::extendSearchUnderTimeControl(std::vector<Variation> pvHistory, uint32_t minTime, uint32_t maxTime, uint32_t timeSpent) {
    // Wenn die Suche weniger als 5 Durchläufe durchgeführt hat, dann wird die Suche nicht unterbrochen
    if(pvHistory.size() < 5)
        return true;

    // Wenn die Suche die minimale Zeit noch nicht erreicht hat, dann wird die Suche nicht unterbrochen
    if(timeSpent < minTime)
        return true;

    // Wenn die Suche die maximale Zeit erreicht hat, dann wird die Suche unterbrochen
    if(timeSpent >= maxTime)
        return false;
    
    // Berechne die Standardabweichung der Bewertungen der letzten 5 Durchläufe
    double scoreStandardDeviation = 0;

    for(size_t i = pvHistory.size() - 5; i < pvHistory.size(); i++) {
        scoreStandardDeviation += pow(pvHistory[i].score - pvHistory[pvHistory.size() - 1].score, 2);
    }

    scoreStandardDeviation = sqrt(scoreStandardDeviation / 5);

    // Bestimme, wie häufig der momentane beste Zug auch in den letzten 4 Durchläufen der Suche der beste Zug war
    int32_t bestMoveChanges = 0;

    for(size_t i = pvHistory.size() - 5; i < pvHistory.size() - 1; i++) {
        if(pvHistory[i].moves[0] != pvHistory[pvHistory.size() - 1].moves[0])
            bestMoveChanges++;
    }

    // Bestimme, ob es sich lohnt die Suche zu verlängern

    // Berechne anhand der Zeit, wie viel Spielraum bei der Standardabweichung erlaubt ist
    double timeFactor = ((double) timeSpent - (double) minTime) / ((double) maxTime - (double) minTime);

    // Begrenze den Spielraum auf 0.0 bis 1.0, falls maxTime = minTime
    if(timeFactor < 0.0)
        timeFactor = 0.0;
    else if(timeFactor > 1.0)
        timeFactor = 1.0;
    
    // Wenn sich der beste Zug in den letzten 5 Durchläufen mindestens 4 mal geändert hat, dann wird die Suche verlängert
    if(bestMoveChanges >= 4)
        return true;

    // Wenn sich der beste Zug in den letzten 5 Durchläufen mindestens 3 mal geändert hat, dann wird die Suche verlängert,
    // wenn die Standardabweichung der Bewertungen der letzten 5 Durchläufe ein wenig hoch ist
    if(bestMoveChanges >= 3 && scoreStandardDeviation > 40.0 * timeFactor)
        return true;

    // Wenn sich der beste Zug in den letzten 5 Durchläufen mindestens 2 mal geändert hat, dann wird die Suche verlängert,
    // wenn die Standardabweichung der Bewertungen der letzten 5 Durchläufe mittelhoch ist
    if(bestMoveChanges >= 2 && scoreStandardDeviation > 60.0 * timeFactor)
        return true;

    // Wenn sich der beste Zug in den letzten 5 Durchläufen mindestens 1 mal geändert hat, dann wird die Suche verlängert,
    // wenn die Standardabweichung der Bewertungen der letzten 5 Durchläufe hoch ist
    if(bestMoveChanges >= 1 && scoreStandardDeviation > 75.0 * timeFactor)
        return true;

    // Ansonsten wird die Suche nicht verlängert
    return false;
}

/**
 * @brief Startet die Suche.
 * 
 * @param searchTime Die Zeit, die die Suche dauern soll.
 * @param treatAsTimeControl Gibt an, ob die Zeit eingeteilt werden soll.
 */
void MinimaxEngine::search(uint32_t searchTime, bool treatAsTimeControl) {
    stop();

    // Setze die Suchmetriken zurück
    searchRunning = true;
    currentMaxDepth = 0;
    currentAge = board->getPly();
    nodesSearched = 0;
    mateDistance = MAX_PLY;

    // Erstelle eine Kopie des Spielbretts,
    // damit das Ausgangsbrett nicht verändert wird
    searchBoard = *board;
    evaluator.setBoard(searchBoard);

    // Leert sämtliche Tabellen und Listen,
    // die während der Suche verwendet werden
    clearRelativeHistory();
    clearKillerMoves();
    clearCounterMoves();
    variations.clear();

    startTime = std::chrono::system_clock::now();

    if(treatAsTimeControl) {
        // Berechne die minimale und maximale Zeit, die die Suche dauern soll
        int32_t numLegalMoves = searchBoard.generateLegalMoves().size();

        double oneThirtiethOfSearchTime = searchTime * 0.0333;
        double oneFourthOfSearchTime = searchTime * 0.25;

        uint64_t minTime = oneThirtiethOfSearchTime - oneThirtiethOfSearchTime * exp(-0.05  * numLegalMoves);
        uint64_t maxTime = oneFourthOfSearchTime - oneFourthOfSearchTime * exp(-0.05  * numLegalMoves);

        int32_t timeFacArrayIndex = std::min(numLegalMoves, timeFactorArraySize) - 1;

        minTime *= timeFactor[timeFacArrayIndex];
        maxTime *= timeFactor[timeFacArrayIndex];

        // Puffer von 10 ms einberechnen
        maxTime = std::max((uint64_t)0, maxTime - 10);

        if(minTime > maxTime)
            minTime = maxTime;

        endTime = startTime + std::chrono::milliseconds(maxTime);
        runSearch(true, minTime, maxTime);
    } else {
        // Berechne die Endzeit
        if(searchTime > 0)
            endTime = startTime + std::chrono::milliseconds(searchTime);
        else
            endTime = std::chrono::time_point<std::chrono::system_clock>::max();

        runSearch(false, 0, 0);
    }

    evaluator.setBoard(*board);
}

void MinimaxEngine::stop() {
    searchRunning = false;
}

/**
 * @brief Gibt eine schnelle Vorbewertung des Zuges zurück.
 * 
 * @param move Der, zu bewertende, Zug.
 * @param ply Die Suchtiefe.
 * @return Die Bewertung des Zuges.
 */
int32_t MinimaxEngine::scoreMove(Move& move, int16_t ply) {
    int32_t moveScore = 0;
    
    if(!move.isQuiet()) {
        // Alle, nicht leisen, Züge werden nach SEE bewertet
        int32_t seeScore = evaluator.evaluateMoveSEE(move);
        seeCache.put(move, seeScore);
        moveScore += seeScore + DEFAULT_CAPTURE_MOVE_SCORE;
    } else {
        // Leise Züge werden mit der Killerzug- und Konterzug-Heuristik bewertet
        bool isKillerMove = false;
        if(killerMoves[ply][0] == move) {
            moveScore += 80;
            isKillerMove = true;
        } else if(killerMoves[ply][1] == move) {
            moveScore += 70;  
            isKillerMove = true;
        } else if(ply >= 2) {
            if(killerMoves[ply - 2][0] == move) {
                moveScore += 60;
                isKillerMove = true;
            } else if(killerMoves[ply - 2][1] == move) {
                moveScore += 50;
                isKillerMove = true;
            }
        }

        // Alle Nicht-Killerzüge werden mit der Vergangenheitsbewertung bewertet
        if(!isKillerMove)
            moveScore += std::clamp(relativeHistory[(searchBoard.getSideToMove() ^ COLOR_MASK) / COLOR_MASK]
                            [move.getOrigin()][move.getDestination()] / ((currentMaxDepth / ONE_PLY) * (currentMaxDepth / ONE_PLY)),
                            -99, 49);

        Move lastMove = searchBoard.getLastMove();
        if(lastMove.exists() && counterMoves[searchBoard.pieceAt(lastMove.getOrigin())]
                                            [lastMove.getDestination()] == move)
            moveScore += 40;

        // Erhöhe die Bewertung von Freibauerzügen
        int32_t movedPieceType = TYPEOF(searchBoard.pieceAt(move.getOrigin()));
        if(movedPieceType == PAWN) {
            int32_t side = searchBoard.getSideToMove();
            int32_t otherSide = searchBoard.getSideToMove() ^ COLOR_MASK;

            if(!(sentryMasks[side / COLOR_MASK][move.getDestination()]
                & searchBoard.getPieceBitboard(otherSide | PAWN)))
                moveScore += DEFAULT_PASSED_PAWN_MOVE_SCORE;
        }
    }

    // Alle Züge werden mit den Figuren-Feldtabelle bewertet
    int32_t movedPieceType = TYPEOF(searchBoard.pieceAt(move.getOrigin()));
    int32_t side = searchBoard.getSideToMove();
    int32_t psqtOrigin = move.getOrigin();
    int32_t psqtDestination = move.getDestination();

    if(side == BLACK) {
        int32_t rank = psqtOrigin / 8;
        int32_t file = psqtOrigin % 8;

        psqtOrigin = (RANK_8 - rank) * 8 + file;

        rank = psqtDestination / 8;
        file = psqtDestination % 8;

        psqtDestination = (RANK_8 - rank) * 8 + file;
    }

    moveScore += MG_PSQT[movedPieceType][psqtDestination] - MG_PSQT[movedPieceType][psqtOrigin];

    return moveScore;
}

/**
 * @brief Führt eine (inplace) Vorsortierung der Züge durch.
 * Die Variante ist speziell für die Wurzel der Suche gedacht
 * und verwendet die Ergebnisse der letzten Suche (falls vorhanden).
 * 
 * @param moves Die, zu sortierenden, Züge.
 */
void MinimaxEngine::sortMovesAtRoot(Array<Move, 256>& moves) {
    Array<MoveScorePair, 256> msp;
    
    std::vector<Move> bestMovesFromLastDepth;

    for(Variation v : variations)
        bestMovesFromLastDepth.push_back(v.moves[0]);

    for(Move move : moves) {
        int32_t moveScore = 0;

        // Wenn der Zug in der letzten Suche zu den besten Zügen gehört hat, dann wird er mit einem Maximalwert bewertet
        size_t index = std::find(bestMovesFromLastDepth.begin(), bestMovesFromLastDepth.end(), move) - bestMovesFromLastDepth.begin();
        if(index < bestMovesFromLastDepth.size())
            moveScore += 30000 - index;
        else
            moveScore += scoreMove(move, 0); // Ansonsten wird er normal bewertet

        msp.push_back({move, moveScore});
    }

    std::sort(msp.begin(), msp.end(), std::greater<MoveScorePair>());

    moves.clear();

    for(MoveScorePair pair : msp)
        moves.push_back(pair.move);
}

/**
 * @brief Führt eine (inplace) Vorsortierung der Züge durch.
 * Diese Sortierung ist speziell für die Quieszenzsuche gedacht
 * und bekommt eine Minimalbewertung übergeben, die die Züge haben müssen,
 * um nicht aus der Liste entfernt zu werden.
 * 
 * @param moves Die, zu sortierenden, Züge.
 * @param minScore Die minimale Bewertung, die ein Zug haben muss, um nicht aus der Liste entfernt zu werden.
 */
void MinimaxEngine::sortAndCutMovesForQuiescence(Array<Move, 256>& moves, int32_t minScore, int32_t moveEvalFunc) {
    Array<MoveScorePair, 256> msp;

    for(Move move : moves) {
        int32_t moveScore = 0;

        // Führe eine statische Figurenabtauschbewertung durch (SEE oder MVVLVA)
        switch(moveEvalFunc) {
            case MVVLVA:
                moveScore += evaluator.evaluateMoveMVVLVA(move) + DEFAULT_CAPTURE_MOVE_SCORE;
                break;
            case SEE:
                int32_t seeScore = evaluator.evaluateMoveSEE(move) + DEFAULT_CAPTURE_MOVE_SCORE;
                moveScore += seeScore;
                break;
        }

        if(moveScore >= minScore)
            msp.push_back({move, moveScore});
    }

    std::sort(msp.begin(), msp.end(), std::greater<MoveScorePair>());

    moves.clear();

    for(MoveScorePair msPair : msp)
        moves.push_back(msPair.move);
}

/**
 * @brief Führt eine (inplace) Vorsortierung der Züge durch.
 * Diese Sortierung ist speziell für die Nullfenster- und PV-Suche gedacht
 * und bekommt eine Minimalbewertung übergeben, die die Züge haben müssen,
 * um nicht aus der Liste entfernt zu werden.
 * 
 * Außerdem verwendet diese Suche die Transpositionstabelle,
 * um die besten Züge aus der letzten Suche zu priorisieren.
 * 
 * @param moves Die, zu sortierenden, Züge.
 * @param ply Die Suchtiefe.
 * @param minScore Die minimale Bewertung, die ein Zug haben muss, um nicht aus der Liste entfernt zu werden.
 */
void MinimaxEngine::sortMoves(Array<Move, 256>& moves, int16_t ply) {
    Array<MoveScorePair, 256> msp;

    // Suche nach einem Eintrag zu dieser Position in der Transpositionstabelle
    TranspositionTableEntry ttEntry;
    transpositionTable.probe(searchBoard.getHashValue(), ttEntry);

    for(Move move : moves) {
        int32_t moveScore = 0;

        // Wenn dieser Zug in der Transpositionstabelle gespeichert ist,
        // war er in bei der letzten Betrachtung dieser Position der Beste.
        // Daher wird er mit einem Maximalwert bewertet.
        if(move == ttEntry.hashMove)
            moveScore += 30000;
        else
            moveScore += scoreMove(move, ply);

        msp.push_back({move, moveScore});
    }

    std::sort(msp.begin(), msp.end(), std::greater<MoveScorePair>());

    moves.clear();

    for(MoveScorePair pair : msp)
        moves.push_back(pair.move);
}

/**
 * @brief Startet die Suche an der Wurzel.
 * Dabei wird ein Aspirationsfenster verwendet,
 * dass bei fail-highs oder fail-lows entsprchend vergrößert wird.
 * 
 * @param depth Die Suchtiefe.
 * @param expectedScore Die erwartete Bewertung der Position (relevant für das Aspirationsfenster).
 */
int16_t MinimaxEngine::rootSearch(int16_t depth, int16_t expectedScore) {
    int32_t aspAlphaReduction = ASP_WINDOW, numAlphaReduction = 1;
    int32_t aspBetaReduction = ASP_WINDOW, numBetaReduction = 1;

    // Bestimme das Fenster der vorherigen Bewertungen
    // (im Multi-PV Modus muss das Aspirationsfenster eventuell vergrößert werden)
    int16_t upperExpectedScore = expectedScore;
    int16_t lowerExpectedScore = expectedScore;

    if(variations.size() > 0) {
        upperExpectedScore = variations.front().score;
        lowerExpectedScore = variations.back().score;
    }

    int16_t alpha = lowerExpectedScore - aspAlphaReduction;
    int16_t beta = upperExpectedScore + aspBetaReduction;

    int16_t score = pvSearchRoot(depth, alpha, beta);

    while((score <= alpha || score >= beta)) {
        if(score <= alpha) {
            // fail-high -> vergrößere das Aspirationsfenster nach unten
            if(numAlphaReduction >= ASP_MAX_DEPTH)
                alpha = MIN_SCORE;
            else {
                aspAlphaReduction *= ASP_STEP_FACTOR;
                alpha = lowerExpectedScore - aspAlphaReduction;
            }

            numAlphaReduction++;
        } else if(score >= beta) {
            // fail-low -> vergrößere das Aspirationsfenster nach oben
            if(numBetaReduction >= ASP_MAX_DEPTH)
                beta = MAX_SCORE;
            else {
                aspBetaReduction *= ASP_STEP_FACTOR;
                beta = upperExpectedScore + aspBetaReduction;
            }

            numBetaReduction++;
        }

        score = pvSearchRoot(depth, alpha, beta);
    }
    
    return variations.front().score;
}

/**
 * @brief Führt eine PV-Suche an der Wurzel durch.
 * 
 * @param depth 
 * @param alpha 
 * @param beta 
 * @return int16_t 
 */
int16_t MinimaxEngine::pvSearchRoot(int16_t depth, int16_t alpha, int16_t beta) {
    nodesSearched++;

    clearPvTable();
    seeCache.clear();

    int32_t pvNodes = numVariations;
    int16_t score, bestScore = MIN_SCORE, worstVariationScore = MIN_SCORE;
    Move bestMove;
    int16_t oldAlpha = alpha;

    std::vector<Variation> newVariations;

    int32_t moveNumber = 1;
    bool isCheckEvasion = searchBoard.isCheck();

    // Generiere alle Züge und wende eine Vorsortierung an
    Array<Move, 256> moves = searchBoard.generateLegalMoves();
    sortMovesAtRoot(moves);

    for(Move move : moves) {
        int16_t matePly = MAX_PLY;
        bool isMateVariation = false;

        // Überprüfe, ob diese Variante in einer vorherigen Suche eine Mattvariante war
        for(Variation variation : variations) {
            if(variation.moves.size() > 0 && variation.moves[0] == move &&
                IS_MATE_SCORE(variation.score)) {
                matePly = MATE_SCORE - std::abs(variation.score);
                isMateVariation = true;
                break;
            }
        }

        // Wenn dieser Zug zu keiner Mattvariante gehört, begrenze die Suchtiefe dieser Variante
        // auf die Matttiefe der schlechtesten Mattvariante der vorherigen Suche
        if(variations.size() > 0 && !isMateVariation) {
            int16_t bestVariationScore = variations.front().score;

            if(bestVariationScore < 0 && IS_MATE_SCORE(bestVariationScore))
                matePly = MATE_SCORE - std::abs(bestVariationScore);
            else if(worstVariationScore > 0 && newVariations.size() >= numVariations && IS_MATE_SCORE(worstVariationScore))
                matePly = MATE_SCORE - std::abs(worstVariationScore);
        }

        mateDistance = matePly;

        // Spiele den Zug
        searchBoard.makeMove(move);

        // Bestimme alle Variablen zur Abschnittsbestimmung
        PruningVariables pruningVars = determinePruningVariables();

        // Bestimme, um wie viel die Suchtiefe verkleinert werden soll
        int16_t extension = determineExtension(pruningVars, isCheckEvasion);
        int16_t nwReduction = determineReduction(depth, 0, moveNumber, pruningVars, isCheckEvasion) * 2 / 3;

        int16_t nwDepthDelta = -ONE_PLY - nwReduction + extension;

        if(pvNodes > 0) {
            // Führe eine PV-Suche auf dieser Variation durch, wenn die Anzahl der betrachteten Variationen
            // kleiner als die Anzahl der, zu bestimmenden, Variationen ist
            score = -pvSearch(depth - ONE_PLY + extension, 1, -beta, -alpha, NULL_MOVE_R_VALUE - 1);
        } else {
            // Führe ansonsten eine Nullfenster-Suche durch
            score = -nwSearch(depth + nwDepthDelta, 1, -alpha - 1, -alpha, NULL_MOVE_R_VALUE - 1);

            // Wenn die Nullfenstersuche einen Wert > alpha liefert, muss die Bewertung
            // dieser Variation exakt bestimmt werden
            if(score > worstVariationScore)
                score = -pvSearch(depth - ONE_PLY + extension, 1, -beta, -alpha, NULL_MOVE_R_VALUE - 1);
        }

        // Nehme den Zug zurück
        searchBoard.undoMove();

        if(!searchRunning) {
            if(numVariations > newVariations.size())
                return 0;
            else
                break;
        }
        
        // Aktualisiere die Vergangenheitsbewertung
        relativeHistory[(searchBoard.getSideToMove() ^ COLOR_MASK) / COLOR_MASK]
                           [move.getOrigin()][move.getDestination()] -= depth / ONE_PLY;

        // Ist dieser Knoten ein fail-high-Knoten?
        if(score >= beta) {
            // Erstelle ein Eintrag in der Transpositionstabelle
            transpositionTable.put(searchBoard.getHashValue(), {
                currentAge, depth, score, CUT_NODE, move
            });

            // Wenn dieser Zug ein ruhiger Zug ist, aktualisiere die Killerzug- und Konterzug-Heuristik
            if(move.isQuiet()) {
                if(killerMoves[0][0] != move) {
                    killerMoves[0][1] = killerMoves[0][0];
                    killerMoves[0][0] = move;
                }

                Move lastMove = searchBoard.getLastMove();
                if(lastMove.exists())
                    counterMoves[searchBoard.pieceAt(lastMove.getDestination())]
                                [lastMove.getOrigin()] = move;
            }

            // Erhöhe die Vergangenheitsbewertung
            relativeHistory[(searchBoard.getSideToMove() ^ COLOR_MASK) / COLOR_MASK]
                        [move.getOrigin()]
                        [move.getDestination()] += (depth / ONE_PLY) * (depth / ONE_PLY);

            return score;
        }

        if(score > bestScore) {
            bestScore = score;
            bestMove = move;
        }

        // Ist dieser Knoten ein PV-Knoten?
        if(score > worstVariationScore) {
            // Erstelle einen Eintrag in der Variationsliste
            std::vector<Move> variationMoves;
            variationMoves.push_back(move);
            variationMoves.insert(variationMoves.end(), pvTable[1].begin(), pvTable[1].end());

            Variation variation = {
                variationMoves,
                score
            };

            auto insertionIndex = std::upper_bound(
                newVariations.begin(),
                newVariations.end(),
                variation,
                std::greater<Variation>());

            if(newVariations.size() >= numVariations) {
                if(insertionIndex != newVariations.end()) {
                    newVariations.insert(insertionIndex, variation);
                    newVariations.pop_back();
                }
            } else {
                newVariations.insert(insertionIndex, variation);
            }

            if(newVariations.size() >= numVariations || newVariations.size() >= moves.size()) {
                // Aktualisiere alpha
                worstVariationScore = newVariations.back().score;

                if(worstVariationScore > oldAlpha)
                    alpha = worstVariationScore;
            }
        }

        pvNodes--;
        moveNumber++;
    }

    // Erstelle einen Eintrag in der Transpositionstabelle
    // für diese Position
    transpositionTable.put(searchBoard.getHashValue(), {
        currentAge, depth, bestScore, EXACT_NODE, bestMove
    });

    // Erhöhe die Vergangenheitsbewertung des besten Zuges
    relativeHistory[(searchBoard.getSideToMove() ^ COLOR_MASK) / COLOR_MASK]
                    [bestMove.getOrigin()]
                    [bestMove.getDestination()] += (depth / ONE_PLY) * (depth / ONE_PLY);
                
    // Überschreibe die Variationsliste mit der neuen Variationsliste, wenn unsere Bewertung
    // innerhalb [alpha, beta] liegt
    if(worstVariationScore > oldAlpha) {
        variations = newVariations;
    }

    return worstVariationScore;
}

/**
 * @brief Führt eine generische PV-Suche durch.
 * 
 * @param depth Die Suchtiefe.
 * @param ply Der Abstand zur Wurzel.
 * @param alpha Unterer Wert des Suchfensters.
 * @param beta Oberer Wert des Suchfensters.
 * @param nullMoveCooldown Die Anzahl der Züge, bis die Nullzugheuristik wieder angewendet werden darf.
 */
int16_t MinimaxEngine::pvSearch(int16_t depth, int16_t ply, int16_t alpha, int16_t beta, int32_t nullMoveCooldown) {
    // Überprüfe, ob die Suche unterbrochen werden soll
    if(nodesSearched % NODES_BETWEEN_CHECKUPS == 0 && isCheckupTime())
        checkup();
    
    if(!searchRunning)
        return 0;

    // Brich ab, wenn die Position ein Unentschieden durch
    // Positionswiederholung, 50-Züge-Regel oder Material ist
    if(evaluator.isDraw()) {
        pvTable[ply].clear();
        return 0;
    }

    // Brich ab, wenn die Entfernung zur Wurzel größer als das bisher schlechteste Matt ist
    if(mateDistance < ply) {
        pvTable[ply].clear();
        return MIN_SCORE + 1;
    }

    // Gehe in die Quieszenzsuche über, wenn das Ende der regulären Suche erreicht ist
    if(depth <= 0 || ply * ONE_PLY >= currentMaxDepth)
        return quiescence(ply + 1, alpha, beta);

    nodesSearched++;

    pvTable[ply + 1].clear();

    TranspositionTableEntry ttEntry;
    
    bool searchPv = true;
    int16_t score, bestScore = MIN_SCORE;
    Move bestMove;
    
    // Generiere alle legalen Züge
    Array<Move, 256> moves = searchBoard.generateLegalMoves();

    // Wenn keine Züge generiert werden konnten, dann ist die Position Matt oder Patt
    if(moves.size() == 0) {
        pvTable[ply].clear();

        if(searchBoard.isCheck())
            return -MATE_SCORE + ply; // Matt
        else
            return 0; // Patt
    }

    int32_t moveNumber = 1;
    bool isCheckEvasion = searchBoard.isCheck();

    // Wende eine Vorsortierung an
    sortMoves(moves, ply);

    for(Move move : moves) {
        // Spiele den Zug
        searchBoard.makeMove(move);

        // Bestimme alle Variablen zur Abschnittsbestimmung
        PruningVariables pruningVars = determinePruningVariables();

        // Bestimme, um wie viel die Suchtiefe verkleinert werden soll
        int16_t extension = determineExtension(pruningVars, isCheckEvasion);
        int16_t nwReduction = determineReduction(depth, ply, moveNumber, pruningVars, isCheckEvasion) * 2 / 3;

        int16_t nwDepthDelta = -ONE_PLY - nwReduction + extension;

        if(searchPv) {
            // Führe auf dem ersten Zug eine PV-Suche durch
            score = -pvSearch(depth - ONE_PLY + extension, ply + 1, -beta, -alpha, nullMoveCooldown - 1);
        } else {
            // Führe ansonsten eine Nullfenster-Suche durch
            score = -nwSearch(depth + nwDepthDelta, ply + 1, -alpha - 1, -alpha, nullMoveCooldown - 1);

            // Wenn die Nullfenstersuche einen Wert > alpha liefert, muss die Bewertung
            // dieser Variation exakt bestimmt werden
            if(score > alpha)
                score = -pvSearch(depth - ONE_PLY + extension, ply + 1, -beta, -alpha, nullMoveCooldown - 1);
        }

        // Nehme den Zug zurück
        searchBoard.undoMove();

        // Brich ab, wenn die Suche unterbrochen werden soll
        if(!searchRunning)
            return 0;
        
        // Aktualisiere die Vergangenheitsbewertung
        relativeHistory[(searchBoard.getSideToMove() ^ COLOR_MASK) / COLOR_MASK]
                           [move.getOrigin()][move.getDestination()] -= depth / ONE_PLY;
        
        // Ist dieser Knoten ein fail-high-Knoten?
        if(score >= beta) {
            // Erstelle ein Eintrag in der Transpositionstabelle
            bool tableHit = transpositionTable.probe(searchBoard.getHashValue(), ttEntry);

            // aber nur, wenn kein Eintrag mit höherer Suchtiefe oder eines PV-Knotens vorhanden ist
            if((!tableHit || (depth > ttEntry.depth)) &&
                             (ttEntry.type != (PV_NODE | EXACT_NODE)))
                transpositionTable.put(searchBoard.getHashValue(), {
                    currentAge, depth, score, CUT_NODE, move
                });

            // Wenn dieser Zug ein ruhiger Zug ist, aktualisiere die Killerzug- und Konterzug-Heuristik
            if(move.isQuiet()) {
                if(killerMoves[ply][0] != move) {
                    killerMoves[ply][1] = killerMoves[ply][0];
                    killerMoves[ply][0] = move;
                }

                Move lastMove = searchBoard.getLastMove();
                if(lastMove.exists())
                    counterMoves[searchBoard.pieceAt(lastMove.getDestination())]
                                [lastMove.getOrigin()] = move;
            }

            // Erhöhe die Vergangenheitsbewertung
            relativeHistory[(searchBoard.getSideToMove() ^ COLOR_MASK) / COLOR_MASK]
                        [move.getOrigin()]
                        [move.getDestination()] += (depth / ONE_PLY) * (depth / ONE_PLY);

            return score;
        }

        if(score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
        
        // Ist dieser Knoten ein PV-Knoten?
        if(score > alpha) {
            alpha = score;

            // Schreibe die neue PV in die PV-Tabelle
            if(ply < 63) {
                pvTable[ply].clear();
                pvTable[ply].push_back(move);
                pvTable[ply].push_back(pvTable[ply + 1]);
            }
        }

        searchPv = false;
        moveNumber++;
    }

    // Erstelle einen Eintrag in der Transpositionstabelle
    // für diese Position
    bool tableHit = transpositionTable.probe(searchBoard.getHashValue(), ttEntry);

    if(!tableHit || (depth > ttEntry.depth))
        transpositionTable.put(searchBoard.getHashValue(), {
            currentAge, depth, bestScore, EXACT_NODE, bestMove
        });

    // Erhöhe die Vergangenheitsbewertung des besten Zuges
    relativeHistory[(searchBoard.getSideToMove() ^ COLOR_MASK) / COLOR_MASK]
                    [bestMove.getOrigin()]
                    [bestMove.getDestination()] += (depth / ONE_PLY) * (depth / ONE_PLY);

    return bestScore;
}

/**
 * @brief Führt eine Nullfenstersuche durch.
 * 
 * @param ply Der Abstand zur Wurzel.
 * @param alpha Unterer Wert des Suchfensters.
 * @param beta Oberer Wert des Suchfensters.
 * @param nullMoveCooldown Die Anzahl der Züge, bis die Nullzugheuristik wieder angewendet werden darf.
 */
int16_t MinimaxEngine::nwSearch(int16_t depth, int16_t ply, int16_t alpha, int16_t beta, int32_t nullMoveCooldown) {
    // Überprüfe, ob die Suche unterbrochen werden soll
    if(nodesSearched % NODES_BETWEEN_CHECKUPS == 0 && isCheckupTime())
        checkup();

    if(!searchRunning)
        return 0;

    // Brich ab, wenn die Position ein Unentschieden durch
    // Positionswiederholung, 50-Züge-Regel oder Material ist
    if(evaluator.isDraw())
        return 0;

    // Brich ab, wenn die Entfernung zur Wurzel größer als das bisher schlechteste Matt ist
    if(mateDistance < ply)
        return MIN_SCORE + 1;

    // Gehe in die Quieszenzsuche über, wenn das Ende der regulären Suche erreicht ist
    if(depth <= 0 || ply * ONE_PLY >= currentMaxDepth)
        return quiescence(ply + 1, alpha, beta);

    nodesSearched++;

    // Suche in der Transpositionstabelle nach einem Eintrag zu dieser Position
    TranspositionTableEntry ttEntry = {0, 0, 0, 0, Move()};
    bool tableHit = transpositionTable.probe(searchBoard.getHashValue(), ttEntry);

    if(tableHit) {
        // Wenn ein Eintrag gefunden wurde, überprüfe, ob dieser Eintrag
        // für diese Suchtiefe gültig ist
        if(ttEntry.depth >= depth) {
            switch(NODE_TYPE(ttEntry.type)) {
                case EXACT_NODE:
                    return ttEntry.score;
                case CUT_NODE:
                    if(ttEntry.score >= beta)
                        return ttEntry.score;
            }
        }
    }

    bool isCheckEvasion = searchBoard.isCheck();

    // Wenn die Nullzugheuristik angewendet werden darf, dann wende sie an
    if(nullMoveCooldown <= 0 && !isCheckEvasion && !isMateLine()) {
        int32_t side = searchBoard.getSideToMove();
        Bitboard minorOrMajorPieces;

        if(side == WHITE)
            minorOrMajorPieces = searchBoard.getWhiteOccupiedBitboard() & ~searchBoard.getPieceBitboard(WHITE | PAWN);
        else
            minorOrMajorPieces = searchBoard.getBlackOccupiedBitboard() & ~searchBoard.getPieceBitboard(BLACK | PAWN);

        // Wende die Nullzugheuristik nur an, wenn wir midestens eine Leicht- /Schwerfigur haben.
        // Ansonsten liefert die Nullzugheuristik im Fall von Zugzwang sehr schlechte Ergebnisse.
        if(minorOrMajorPieces) {
            // Führe einen Nullzug durch
            Move nullMove = Move::nullMove();
            searchBoard.makeMove(nullMove);

            // Bestimme, um wie viel die Suchtiefe verkleinert werden soll
            int16_t depthReduction = 3 * ONE_PLY;

            if(depth >= 8 * ONE_PLY)
                depthReduction = 4 * ONE_PLY;

            int16_t nullMoveScore = -nwSearch(depth - depthReduction, ply + 1, -beta, -alpha, NULL_MOVE_R_VALUE);

            searchBoard.undoMove();

            // Wenn der Nullzug einen Wert >= beta liefert, befinden wir uns
            // höchstwahrscheinlich in einem fail-high-Knoten und können die Suche abbrechen
            if(nullMoveScore >= beta)
                return nullMoveScore;
        }
    }
    
    // Generiere alle legalen Züge
    Array<Move, 256> moves = searchBoard.generateLegalMoves();

    // Wenn keine Züge generiert werden konnten, dann ist die Position Matt oder Patt
    if(moves.size() == 0) {
        if(searchBoard.isCheck())
            return -MATE_SCORE + ply; // Matt
        else
            return 0; // Patt
    }

    // Wende eine Vorsortierung an
    sortMoves(moves, ply);

    int16_t bestScore = MIN_SCORE;
    Move bestMove;

    int32_t moveNumber = 1;

    for(Move move : moves) {
        // Spiele den Zug
        searchBoard.makeMove(move);

        // Bestimme alle Variablen zur Abschnittsbestimmung
        PruningVariables pruningVars = determinePruningVariables();

        if(shouldPrune(depth, ply, moveNumber, pruningVars, isCheckEvasion)) {
            nodesSearched++;
            // Nehme den Zug zurück
            searchBoard.undoMove();
            continue;
        }

        // Bestimme, um wie viel die Suchtiefe verkleinert werden soll
        int16_t extension = determineExtension(pruningVars, isCheckEvasion);
        int16_t nwReduction = determineReduction(depth, ply, moveNumber, pruningVars, isCheckEvasion);

        int16_t nwDepthDelta = -ONE_PLY - nwReduction + extension;

        int16_t score = -nwSearch(depth + nwDepthDelta, ply + 1, -beta, -alpha, nullMoveCooldown - 1);

        // Wenn eine reduzierte Suche eine Bewertung > alpha liefert, dann
        // wird eine nicht reduzierte Suche durchgeführt.
        if(score > alpha && nwReduction > 0)
            score = -nwSearch(depth - ONE_PLY + extension, ply + 1, -beta, -alpha, nullMoveCooldown - 1);

        // Nehme den Zug zurück
        searchBoard.undoMove();

        // Brich ab, wenn die Suche unterbrochen werden soll
        if(!searchRunning)
            return 0;
        
        // Aktualisiere die Vergangenheitsbewertung
        relativeHistory[(searchBoard.getSideToMove() ^ COLOR_MASK) / COLOR_MASK]
                        [move.getOrigin()][move.getDestination()] -= depth / ONE_PLY;

        // Ist dieser Knoten ein fail-high-Knoten?
        if(score >= beta) {
            // Erstelle ein Eintrag in der Transpositionstabelle
            bool tableHit = transpositionTable.probe(searchBoard.getHashValue(), ttEntry);

            // aber nur, wenn kein Eintrag mit höherer Suchtiefe oder eines PV-Knotens vorhanden ist
            if(!tableHit || (depth > ttEntry.depth &&
                               ttEntry.type == (NW_NODE | CUT_NODE)))
                transpositionTable.put(searchBoard.getHashValue(), {
                    currentAge, depth, score, NW_NODE | CUT_NODE, move
                });
            
            // Wenn dieser Zug ein ruhiger Zug ist, aktualisiere die Killerzug- und Konterzug-Heuristik
            if(move.isQuiet()) {
                if(killerMoves[ply][0] != move) {
                    killerMoves[ply][1] = killerMoves[ply][0];
                    killerMoves[ply][0] = move;
                }

                Move lastMove = searchBoard.getLastMove();
                if(lastMove.exists())
                    counterMoves[searchBoard.pieceAt(lastMove.getDestination())]
                                [lastMove.getOrigin()] = move;
            }

            // Erhöhe die Vergangenheitsbewertung
            relativeHistory[(searchBoard.getSideToMove() ^ COLOR_MASK) / COLOR_MASK]
                        [move.getOrigin()]
                        [move.getDestination()] += (depth / ONE_PLY) * (depth / ONE_PLY);

            return score;
        }
        
        if(score > bestScore) {
            bestScore = score;
            bestMove = move;
        }

        moveNumber++;
    }

    // Erstelle einen Eintrag in der Transpositionstabelle für diese Position
    // (nur wenn kein Eintrag mit höherer Suchtiefe oder eines PV-Knotens für diese Position existiert)
    if(!tableHit || (depth > ttEntry.depth &&
                        !IS_REGULAR_NODE(ttEntry.type)))
        transpositionTable.put(searchBoard.getHashValue(), {
            currentAge, depth, bestScore, NW_NODE | EXACT_NODE, bestMove
        });

    return bestScore;
}

MinimaxEngine::PruningVariables MinimaxEngine::determinePruningVariables() {
    PruningVariables state {
        Move::nullMove(),
        EMPTY,
        false,
        MIN_SCORE,
        false,
        false,
        false
    };

    state.lastMove = searchBoard.getLastMove();
    state.movedPiece = searchBoard.pieceAt(state.lastMove.getDestination());
    state.isCheck = searchBoard.isCheck();
    seeCache.probe(state.lastMove, state.seeScore);

    int32_t movedPieceType = TYPEOF(searchBoard.pieceAt(state.lastMove.getDestination()));
    state.isPawnPush = movedPieceType == PAWN;

    int32_t side = searchBoard.getSideToMove() ^ COLOR_MASK;
    int32_t otherSide = searchBoard.getSideToMove();
    if(state.isPawnPush) {
        if(!(sentryMasks[side / COLOR_MASK][state.lastMove.getDestination()]
            & searchBoard.getPieceBitboard(otherSide | PAWN)))
            state.isPassedPawnPush = true;
    }

    if(searchBoard.getAttackBitboard(otherSide).getBit(state.lastMove.getOrigin()))
        state.isCaptureEvasion = true;

    return state;
}

/**
 * @brief Bestimme, um die viel die Suchtiefe für diesen Zug erweitert werden soll.
 * 
 * @param isCheckEvasion Gibt an, ob es sich um eine Schachabwehr handelt.
 */
int16_t MinimaxEngine::determineExtension(PruningVariables& pruningVars, bool isCheckEvasion) {
    int16_t extension = 0;

    Move m = pruningVars.lastMove;

    bool isCheck = pruningVars.isCheck;

    // Schach oder Schachabwehr
    if(isCheckEvasion || isCheck)
        extension += ONE_PLY;
    
    // Schlagzug oder Bauernumwandlung
    if(m.isCapture() || m.isPromotion())
        extension += ONE_HALF_PLY;
    
    // Freibauerzüge
    if(pruningVars.isPassedPawnPush)
        extension += ONE_HALF_PLY;
    
    return extension;
}

/**
 * @brief Bestimmt, um wie viel die Suchtiefe für diesen Zug reduziert werden soll.
 * 
 * @param depth Die Suchtiefe.
 * @param ply Der Abstand zur Wurzel.
 * @param moveCount Die Anzahl der bereits generierten Züge.
 * @param isCheckEvasion Gibt an, ob es sich um eine Schachabwehr handelt.
 */
int16_t MinimaxEngine::determineReduction(int16_t depth, int16_t ply, int32_t moveCount, PruningVariables& pruningVars, bool isCheckEvasion) {
    int16_t reduction = 0;

    bool isCheck = pruningVars.isCheck;

    // Keine Reduktion bei dem ersten Zug
    int32_t unreducedMoves = 1;

    // oder bei Schach oder einer Schachabwehr
    if(moveCount <= unreducedMoves)
        return 0;

    if(ply <= 3)
        return 0;

    if(isCheck || isCheckEvasion || pruningVars.seeScore >= NEUTRAL_SEE_SCORE)
        reduction -= 2 * ONE_PLY;

    // Reduziere anhand einer logarithmischen Funktion,
    // die von der Anzahl der bereits bearbeiteten Züge abhängig ist
    reduction += (int16_t)((std::log(moveCount - unreducedMoves + 1) / std::log(2) + std::log(depth / ONE_PLY)) * ONE_PLY);

    // Höhere Reduktion bei einem Schlagzug mit SEE < 0
    if(pruningVars.lastMove.isCapture() && pruningVars.seeScore < NEUTRAL_SEE_SCORE)
        reduction += 2 * ONE_PLY;

    // Geringere Reduktion, wenn wir einem Schlagzug ausweichen
    if(pruningVars.isCaptureEvasion)
        reduction -= ONE_PLY;
    else if(!pruningVars.isPawnPush && pruningVars.lastMove.isQuiet())
        reduction += 2 * ONE_PLY;

    reduction -= relativeHistory[(searchBoard.getSideToMove() ^ COLOR_MASK) / COLOR_MASK]
                                [pruningVars.lastMove.getOrigin()]
                                [pruningVars.lastMove.getDestination()] / 25000.0 * ONE_PLY;

    // Reduziere weniger, wenn wir uns in einer Mattvariante befinden
    if(isMateLine()) {
        int16_t maxSearchPly = (int16_t)(currentMaxDepth / ONE_PLY);

        if(maxSearchPly > mateDistance + ply)
            reduction -= (maxSearchPly - mateDistance - ply) * ONE_PLY;
    }
    
    return std::max(reduction, (int16_t)0);
}

/**
 * @brief Überprüft, ob ein Knoten abgeschnitten werden soll.
 * 
 * @param depth Die momentane Tiefe.
 * @param ply Der Abstand zur Wurzel.
 * @param moveCount Die Anzahl der bereits generierten Züge.
 * @param isCheckEvasion Gibt an, ob es sich um eine Schachabwehr handelt.
 */
bool MinimaxEngine::shouldPrune(int16_t depth, int16_t ply, int32_t moveCount, PruningVariables& pruningVars, bool isCheckEvasion) {
    // Wir schneiden nie den ersten Zug ab
    if(moveCount <= 1)
        return false;

    if(ply <= 3)
        return false;

    // Wir schneiden nie einen Schlagzug oder eine Bauernumwandlung ab
    Move m = pruningVars.lastMove;
    if(m.isCapture() || m.isPromotion())
        return false;

    // Wir schneiden nie bei Schach oder Schachabwehr ab
    if(isCheckEvasion || pruningVars.isCheck)
        return false;

    // Wir schneiden nie Freibauerzüge ab
    if(pruningVars.isPassedPawnPush)
        return false;

    // Wir schneiden nie bei einem ausweichenden Zug ab
    if(pruningVars.isCaptureEvasion)
        return false;

    if(moveCount >= depth * 5 / (ONE_PLY * 3) + 2)
        return true;

    return false;
}

/**
 * @brief Führt eine Quieszenzsuche durch.
 * Die Quieszenzsuche erweitert die Blätter des Suchbaums so,
 * dass nur noch ruhige Züge betrachtet werden.
 * 
 * @param ply Der Abstand zur Wurzel.
 * @param alpha Unterer Wert des Suchfensters.
 * @param beta Oberer Wert des Suchfensters.
 */
int16_t MinimaxEngine::quiescence(int16_t ply, int16_t alpha, int16_t beta) {
    // Überprüfe, ob die Suche unterbrochen werden soll
    if(nodesSearched % NODES_BETWEEN_CHECKUPS == 0 && isCheckupTime())
        checkup();

    if(!searchRunning)
        return 0;

    nodesSearched++;

    int16_t score = MIN_SCORE;

    if(!searchBoard.isCheck()) {
        // "Stand-Pat"-Score:
        // Die Bewertung der momentanen Postion ohne weitere Züge zu betrachten.
        // Weil die Quieszenzsuche nicht alle Züge betrachtet, muss der Spieler,
        // der hier am Zug ist, nicht unbedingt einen Zug spielen, der hier betrachtet wird
        score = (int16_t)evaluator.evaluate();
    } else if(evaluator.isDraw()) {
        // Brich ab, wenn die Position ein Unentschieden durch
        // Positionswiederholung, 50-Züge-Regel oder Material ist
        // Ohne diese Abbruchbedingung würde die Quieszenzsuche in einer
        // Endlosschleife enden, wenn ein Spieler den anderen ewig im Schach halten kann
        return 0;
    }

    // Brich ab, wenn der Stand-Pat-Score >= beta ist
    if(score >= beta)
        return score;
    
    if(score > alpha)
        alpha = score;
    
    int16_t bestScore = score;
    
    Array<Move, 256> moves;

    // Generiere alle, zu betrachtenden, Züge
    if(searchBoard.isCheck()) {
        // Wenn wir im Schach stehen, betrachte alle Züge
        // (in diesem Fall sind alle Züge Schachabwehrzüge)
        moves = searchBoard.generateLegalMoves();
        if(moves.size() == 0)
            return -MATE_SCORE + ply;

        // Führe eine Vorsortierung durch
        sortAndCutMovesForQuiescence(moves, MIN_SCORE, MVVLVA);
    } else {
        // Wenn wir nicht im Schach stehen, betrachte nur Schlagzüge
        moves = searchBoard.generateLegalCaptures();

        // Führe eine Vorsortierung durch und schneide alle Züge ab,
        // die eine negative SEE-Bewertung haben
        sortAndCutMovesForQuiescence(moves, NEUTRAL_SEE_SCORE, SEE);
    }
    
    for(Move move : moves) {
        // Spiele den Zug
        searchBoard.makeMove(move);

        score = -quiescence(ply + 1, -beta, -alpha);

        // Nehme den Zug zurück
        searchBoard.undoMove();

        // Brich ab, wenn die Suche unterbrochen werden soll
        if(!searchRunning)
            return 0;

        // Ist dieser Knoten ein fail-high-Knoten?
        if(score >= beta)
            return score;

        // Ist dieser Knoten ein PV-Knoten?
        if(score > bestScore)
            bestScore = score;
        
        if(score > alpha)
            alpha = score;
    }

    return bestScore;
}