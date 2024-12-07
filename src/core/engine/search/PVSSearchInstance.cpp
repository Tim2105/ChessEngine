#include "core/engine/search/PVSSearchInstance.h"

int PVSSearchInstance::pvs(int depth, int ply, int alpha, int beta, unsigned int nodeType, int nullMoveCooldown, int singularExtCooldown, bool skipHashMove) {
    // Setze die momentane Suchtiefe, wenn wir uns im Wurzelknoten befinden.
    if(ply == 0) {
        currentSearchDepth = std::max(depth, 1);
        rootAge = board.getAge();
    }

    // Führe die Checkup-Funktion regelmäßig aus.
    if(localNodeCounter >= NODES_PER_CHECKUP && checkupFunction) {
        localNodeCounter = 0;
        checkupFunction();
    }

    // Prüfe, ob die Suche abgebrochen werden soll.
    if(stopFlag.load() && currentSearchDepth > 1)
        return 0;

    // Wenn die Suchtiefe 0 oder die maximale Suchdistanz erreicht wurde, bestimme die Bewertung
    // des aktuellen Knotens mit einer Quieszenzsuche.
    if(depth <= 0 || ply >= MAX_PLY)
        return quiescence(ply, alpha, beta);

    // Wir betrachten diesen Knoten.
    nodesSearched.fetch_add(1);
    localNodeCounter++;
    selectiveDepth = std::max(selectiveDepth, (int)ply);

    // Überprüfe, ob wir uns in einer Remisstellung befinden.
    if(ply > 0 && evaluator.isDraw()) {
        clearPVTable(ply);
        return DRAW_SCORE;
    }

    /**
     * Mate Distance Pruning:
     * 
     * Wenn wir bereits ein schnelleres Matt in einem
     * anderen Pfad gefunden haben, können wir die Suche
     * abbrechen.
     */
    if(alpha >= MATE_SCORE - ply)
        return alpha; // fail-hard

    if(beta <= -MATE_SCORE + ply)
        return beta; // fail-hard

    // Suche in der Transpositionstabelle nach einem Eintrag für
    // die aktuelle Position.
    TranspositionTableEntry entry;
    bool entryExists = transpositionTable.probe(board.getHashValue(), entry);
    if(entryExists && nodeType != PV_NODE && !skipHashMove) {
        // Ein Eintrag kann nur verwendet werden, wenn die eingetragene
        // Suchtiefe mindestens so groß ist wie unsere Suchtiefe.
        if(entry.depth >= depth) {
            if(entry.type == TranspositionTableEntry::EXACT) {
                // Der Eintrag speichert eine exakte Bewertung,
                // d.h. er stammt aus einem PV-Knoten.
                return entry.score;
            } else if(entry.type == TranspositionTableEntry::LOWER_BOUND) {
                // Der Eintrag speichert eine untere Schranke,
                // d.h. er stammt aus einem Cut-Knoten.
                if(entry.score >= beta)
                    return entry.score;
                else if(entry.score > alpha)
                    alpha = entry.score;
            } else if(entry.type == TranspositionTableEntry::UPPER_BOUND) {
                // Der Eintrag speichert eine obere Schranke,
                // d.h. er stammt aus einem All-Knoten.
                if(entry.score <= alpha)
                    return entry.score;
                else if(entry.score < beta)
                    beta = entry.score;
            }
        }
    }

    // Leere den Eintrag im Suchstack für den aktuellen Abstand zum Wurzelknoten.
    clearSearchStack(ply);

    /**
     * Ermittele die vorläufige Bewertung der Position.
     * Die vorläufige Bewertung ist die eingetragene Bewertung
     * in der Transpositionstabelle oder die statische Bewertung,
     * wenn kein Eintrag existiert.
     */
    searchStack[ply].preliminaryScore = entryExists ? entry.score : evaluator.evaluate();

    /**
     * Überprüfe, ob wir uns gegenüber dem letzten Zug verbessert haben.
     */
    bool isImproving = ply < 2 || searchStack[ply].preliminaryScore > searchStack[ply - 2].preliminaryScore + IMPROVING_THRESHOLD;

    /**
     * Null-Move-Pruning:
     * Wir gehen davon aus, dass ein Spieler mit einem Zug
     * immer seine Position verbessern kann (=> Nullzugobservation).
     * Wenn wir also unseren Zug überspringen (=> Nullzug) und in einer
     * reduzierten Suche immer noch >= beta sind,
     * gucken wir uns diesen Knoten gar nicht an.
     * 
     * In Positionen mit Zugzwang hält die Nullzugobservation
     * nicht, daher deaktivieren wir die Heuristik wenn wir
     * nur noch einen König und Bauern haben.
     */
    if(nullMoveCooldown <= 0 && !board.isCheck() && nodeType != PV_NODE &&
       depth > 1 && !deactivateNullMove()) {

        int depthReduction = calculateNullMoveReduction(depth, searchStack[ply].preliminaryScore, beta, isImproving);
        if(depthReduction > 1 && depth - depthReduction > 0) {
            Move nullMove = Move::nullMove();
            board.makeMove(nullMove);

            int score = -pvs(depth - depthReduction, ply + 1, -beta, -beta + 1, CUT_NODE, NULL_MOVE_COOLDOWN, std::max(singularExtCooldown - 1, 1));

            board.undoMove();

            if(score >= beta) {
                if(!verifyNullMove() || depth <= 2)
                    return score;

                score = pvs(depth - 2, ply, beta - 1, beta, CUT_NODE, NULL_MOVE_COOLDOWN, singularExtCooldown);
                if(score >= beta)
                    return score;
            }
        }
    }

    clearPVTable(ply + 1);

    // Lade den Eintrag aus der Transpositionstabelle neu, da
    // bei der Suche mit mehreren Threads ein neuer Eintrag
    // vorliegen könnte.
    transpositionTable.probe(board.getHashValue(), entry);
    if(entryExists && nodeType != PV_NODE && !skipHashMove) {
        if(entry.depth >= depth) {
            if(entry.type == TranspositionTableEntry::EXACT) {
                return entry.score;
            } else if(entry.type == TranspositionTableEntry::LOWER_BOUND) {
                if(entry.score >= beta)
                    return entry.score;
                else if(entry.score > alpha)
                    alpha = entry.score;
            } else if(entry.type == TranspositionTableEntry::UPPER_BOUND) {
                if(entry.score <= alpha)
                    return entry.score;
                else if(entry.score < beta)
                    beta = entry.score;
            }
        }
    }

    /**
     * Singular Reply Extension (nur in Cut-Knoten):
     * 
     * Wir haben diese Position bereits einmal betrachtet und
     * mit einer Bewertung > alpha bewertet. Wir wollen jetzt
     * herausfinden, ob das der einzig gute Zug ist.
     * Dazu werden alle übrigen Züge mit reduzierter Suchtiefe betrachtet.
     * Wenn alle anderen Züge weit unter alpha bewertet werden,
     * gehen wir davon aus, dass der beste Zug in dieser
     * Position der einzige gute Zug ist und erhöhen seine Suchtiefe.
     * Rekursive Erweiterungen werden beschränkt, um Suchexplosionen zu vermeiden.
     */
    int singularExtension = 0;
    int singularDepth = std::min(depth / 2, depth - 4);

    if(ply > 0 && nodeType != PV_NODE && singularExtCooldown <= 0 && singularDepth >= 2 && !isMateScore(alpha) &&
       entryExists && entry.depth >= singularDepth && entry.score > alpha &&
       entry.type != TranspositionTableEntry::UPPER_BOUND) {

        int singleDepthThreshold = std::abs(alpha) / 4 + 160;
        int depthThresholdStep = singleDepthThreshold / 16;
        int reducedBeta = alpha - singleDepthThreshold + std::max(singularDepth, 10) * depthThresholdStep;

        int score = pvs(singularDepth, ply, reducedBeta - 1, reducedBeta, CUT_NODE, std::max(nullMoveCooldown, 1), SINGULAR_EXT_COOLDOWN, true);

        // Überprüfe, ob wir unter unserem reduzierten Beta-Wert liegen.
        if(score < reducedBeta)
            singularExtension = 1;
    }

    // Lade den Eintrag aus der Transpositionstabelle neu, da
    // bei der Suche mit mehreren Threads ein neuer Eintrag
    // vorliegen könnte.
    transpositionTable.probe(board.getHashValue(), entry);
    if(entryExists && nodeType != PV_NODE && !skipHashMove) {
        if(entry.depth >= depth) {
            if(entry.type == TranspositionTableEntry::EXACT) {
                return entry.score;
            } else if(entry.type == TranspositionTableEntry::LOWER_BOUND) {
                if(entry.score >= beta)
                    return entry.score;
                else if(entry.score > alpha)
                    alpha = entry.score;
            } else if(entry.type == TranspositionTableEntry::UPPER_BOUND) {
                if(entry.score <= alpha)
                    return entry.score;
                else if(entry.score < beta)
                    beta = entry.score;
            }
        }
    }

    /**
     * Internal Iterative Deepening (1/2):
     * 
     * Wenn der Hashzug aus einer viel geringeren Suchtiefe stammt,
     * führe eine reduzierte Suche durch, um Suchexplosionen zu vermeiden.
     * Führe außerdem eine reduzierte Suche durch, wenn wir uns in einem
     * PV-Knoten oder einem Cut-Knoten befinden und der Hashzug aus einem
     * All-Knoten mit gleicher oder geringerer Suchtiefe stammt.
     */
    int depthReduction = 0;
    if(entryExists) {
        int depthDiff = depth - entry.depth;
        depthReduction = (depthDiff / 4);
    } else
        depthReduction = (depth / 4);

    if(nodeType != PV_NODE && depthReduction > 0)
        depth -= depthReduction;

    /**
     * Internal Iterative Deepening (2/2):
     * 
     * Führe eine reduzierte Suche durch, um einen besseren
     * Hashzug für die aktuelle Position zu bestimmen.
     * IID wird nur in PV-Knoten und Knoten mit einer
     * Suchtiefe >= 8 durchgeführt.
     */
    bool useIID = false;
    if(nodeType != ALL_NODE)
        useIID = depthReduction > 0;

    addMovesToSearchStack(ply, useIID, depth);

    // Prüfe, ob die Suche abgebrochen werden soll.
    if(stopFlag.load() && currentSearchDepth > 1)
        return 0;

    // Ermittele, ob wir Heuristiken anwenden dürfen, um die Suchtiefe zu erweitern.
    bool allowHeuristicExtensions = extensionsOnPath < currentSearchDepth / 2;

    Move move;
    int moveCount = 0, moveScore;
    bool isCheckEvasion = board.isCheck();
    uint8_t ttEntryType = TranspositionTableEntry::UPPER_BOUND;
    int bestScore = MIN_SCORE, lmpCount = determineLMPCount(depth, isCheckEvasion, isImproving);
    Move bestMove;

    // Schleife über alle legalen Züge.
    // Die Züge werden absteigend nach ihrer vorläufigen Bewertung betrachtet,
    // d.h. die besten Züge werden zuerst untersucht.
    for(MoveScorePair& pair : searchStack[ply].moveScorePairs) {
        move = pair.move;
        moveScore = pair.score;

        if(skipHashMove && move == searchStack[ply].hashMove) {
            moveCount++;
            continue;
        }

        /**
         * Futility Pruning:
         * Wenn wir uns in einer geringen Suchtiefe befinden, unsere vorläufige
         * Bewertung weit unter alpha liegt und der Zug nicht offensichtlich
         * gut ist (hohe Bewertung in Vorsortierung), dann können wir den Zug überspringen.
         * 
         * Aus taktischen Gründen wird Futility Pruning nicht angewandt, wenn wir
         * uns im Schach oder einem PV-Knoten befinden, wenn der Zug ein Schlag-/Aufwertungszug ist
         * oder wenn der Zug den Gegner in Schach setzt. Außerdem wird Futility Pruning
         * abgeschaltet, wenn alpha eine Mattbewertung gegen uns ist.
         */
        if(depth <= 4 && moveScore <= KILLER_MOVE_SCORE && !isCheckEvasion &&
           nodeType != PV_NODE && !(isMateScore(alpha) && alpha < NEUTRAL_SCORE) &&
           !(move.isCapture() || move.isPromotion())) {
            
            int preliminaryEval = searchStack[ply].preliminaryScore;

            // Führe den Zug aus und informiere den Evaluator.
            evaluator.updateBeforeMove(move);
            board.makeMove(move);
            evaluator.updateAfterMove();

            bool isCheck = board.isCheck();

            int futilityMargin = calculateFutilityMargin(depth, isImproving);
            if(!isCheck && preliminaryEval + futilityMargin < alpha) {
                // Mache den Zug rückgängig und informiere den Evaluator.
                evaluator.updateBeforeUndo();
                board.undoMove();
                evaluator.updateAfterUndo(move);

                if(moveCount == 0) {
                    bestScore = preliminaryEval;
                    bestMove = move;
                }

                moveCount++;
                continue;
            }
        } else {
            // Führe den Zug aus und informiere den Evaluator.
            evaluator.updateBeforeMove(move);
            board.makeMove(move);
            evaluator.updateAfterMove();
        }

        int score;

        /**
         * Late Move Pruning:
         * Späte Züge in All-/Cut-Knoten mit geringer Tiefe, die den Gegner nicht in Schach setzen,
         * werden sehr wahrscheinlich nicht zu einem Beta-Schnitt führen. Deshalb überspringen wir diese Züge.
         * 
         * Als Failsafe wenden wir LMP nicht an, wenn wir im Schach stehen oder alpha eine Mattbewertung gegen uns ist.
         * Dadurch vermeiden wir, versehentlich eine Mattverteidigung zu übersehen.
         */
        if(nodeType != PV_NODE && moveCount >= lmpCount && moveScore <= NEUTRAL_SCORE &&
           !(isMateScore(alpha) && alpha < NEUTRAL_SCORE) && !isCheckEvasion && !board.isCheck()) {

            evaluator.updateBeforeUndo();
            board.undoMove();
            evaluator.updateAfterUndo(move);

            moveCount++;
            continue;
        }

        // Sage den Knotentyp des Kindknotens voraus.
        uint8_t childType = CUT_NODE;
        if(nodeType == PV_NODE && moveCount == 0)
            childType = PV_NODE;
        else if(nodeType == CUT_NODE)
            childType = ALL_NODE;
        
        int extension = singularExtension * (moveCount == 0);
        bool isCheck = board.isCheck();

        /**
         * Erweiterungen:
         * Ähnlich wie bei der Singular Reply Extension, erweitern wir die Suchtiefe
         * unter bestimmten Bedingungen.
         */

        /**
         * Schacherweiterung:
         * Wenn der Zug den Gegner in Schach setzt, erweitern wir die Suchtiefe.
         */
        if(isCheck) {
            // Erweitere die Suchtiefe, wenn der Zug den Gegner in Schach setzt.
            if(extension == 0 && allowHeuristicExtensions)
                extension += 1;
        }

        extensionsOnPath += extension;

        if(moveCount == 0) {
            // Führe eine vollständige Suche durch.
            score = -pvs(depth - 1 + extension, ply + 1, -beta, -alpha, childType, nullMoveCooldown - 1, singularExtCooldown - 1);
        } else {
            // Durchsuche die restlichen Kinder mit einem Nullfenster (nur in PV-Knoten) und bei,
            // auf ersten Blick, uninteressanten Zügen mit einer reduzierten Suchtiefe
            // (=> Late Move Reductions).
            int reduction = 0;
            if(extension == 0) {
                reduction = determineLMR(moveCount + 1, moveScore, depth, isImproving);

                if(isCheck || isCheckEvasion)
                    reduction -= 1;

                // Reduziere die Reduktion, wenn der Zug einen Freibauern bewegt.
                int movedPieceType = TYPEOF(board.pieceAt(move.getDestination()));
                if(movedPieceType == PAWN) {
                    int side = board.getSideToMove() ^ COLOR_MASK;
                    int otherSide = side ^ COLOR_MASK;
                    if(!(sentryMasks[side / COLOR_MASK][move.getDestination()]
                        & board.getPieceBitboard(otherSide | PAWN))) {
                        
                        reduction -= 1;
                    }
                }

                reduction = std::max(reduction, 0);
            }

            score = -pvs(depth - 1 + extension - reduction, ply + 1, -alpha - 1, -alpha, childType, nullMoveCooldown - 1, singularExtCooldown - 1);

            // Wenn die Bewertung über dem Nullfenster liegt,
            // führe eine vollständige Suche durch, wenn:
            // - die Suchtiefe dieses Kindes reduziert wurde (reduction > 0)
            // - oder die Bewertung liegt dieses Knotens innerhalb des Suchfensters
            //   liegt (< beta)
            if(score > alpha && (reduction > 0 || score < beta)) {
                if(nodeType == PV_NODE)
                    childType = PV_NODE;
                else
                    childType = ALL_NODE;

                score = -pvs(depth - 1 + extension, ply + 1, -beta, -alpha, childType, std::max(nullMoveCooldown - 1, 1), singularExtCooldown - 1);
            }
        }

        extensionsOnPath -= extension;

        // Mache den Zug rückgängig und informiere den Evaluator.
        evaluator.updateBeforeUndo();
        board.undoMove();
        evaluator.updateAfterUndo(move);

        // Prüfe, ob die Suche abgebrochen werden soll.
        if(stopFlag.load() && currentSearchDepth > 1)
            return 0;

        if(score >= beta) {
            // Dieser Zug ist so gut, dass der Gegner ihn bei optimalem
            // Spiel nicht zulassen wird (weil er bereits eine bessere
            // Alternative hat). Daher können wir die Suche abbrechen.
            // => Beta-Schnitt oder Fail-High

            // Speichere Informationen zu diesem Knoten
            // in der Transpositionstabelle. Weil wir verfrüht abbrechen,
            // haben wir nur eine untere Schranke für die Bewertung
            // (wir könnten ja noch einen besseren Zug finden).
            TranspositionTableEntry entry(
                move,
                (int16_t)score,
                (uint16_t)rootAge,
                (uint8_t)depth,
                TranspositionTableEntry::LOWER_BOUND
            );

            transpositionTable.put(board.getHashValue(), entry);

            // Speichere den Zug in der Killerzug-Tabelle,
            // wenn es sich um einen leisen Zug handelt.
            if(!move.isCapture() && !move.isPromotion()) {
                addKillerMove(ply, move);

                // Setze einen Eintrag in der Konterzug-Tabelle.
                Move previousMove = board.getLastMove();
                if(previousMove.exists()) {
                    int previousMoveDestination = previousMove.getDestination();
                    int prevoiuslyMovedPieceType = TYPEOF(board.pieceAt(previousMoveDestination));

                    setCounterMove(move, board.getSideToMove(), prevoiuslyMovedPieceType, previousMoveDestination);
                }
            }

            // Verbessere die relative Bewertung des Zuges
            incrementHistoryScore(move, depth);

            return score;
        }

        // Verschlechtere die relative Bewertung des Zuges,
        // er führte nicht zu einem Beta-Schnitt.
        decrementHistoryScore(move, depth);

        if(score > bestScore) {
            // Der Zug ist der Beste, den wir bisher gefunden haben.

            bestScore = score;
            bestMove = move;

            if(score > alpha) {
                // Diese Variation führt zu einer besseren Bewertung,
                // als die momentane Hauptvariante.
                // => Wir befinden uns in einem PV-Knoten.
                ttEntryType = TranspositionTableEntry::EXACT;
                alpha = score;

                addPVMove(ply, move);

                if(ply == 0)
                    pvScore = score;
            }
        }

        moveCount++;
    }

    // Es gab keine legalen Züge, das Spiel ist vorbei.
    if(moveCount == 0) {
        clearPVTable(ply);

        if(board.isCheck())
            return -MATE_SCORE + ply; // Matt
        else
            return DRAW_SCORE; // Patt
    }

    // Speichere Informationen zu diesem Knoten
    // in der Transpositionstabelle.
    // Wenn wir uns in einem PV-Knoten befinden,
    // haben wir eine exakte Bewertung, ansonsten
    // nur eine obere Schranke (die Bewertung kam in diesem Fall
    // wahrscheinlich aus einem Cut-Knoten, d.h. Beta-Schnitt,
    // zustande und war nur eine obere Schranke im Kindknoten).
    entry = {
        bestMove,
        (int16_t)bestScore,
        (uint16_t)rootAge,
        (uint8_t)depth,
        ttEntryType
    };

    transpositionTable.put(board.getHashValue(), entry);

    if(ttEntryType == TranspositionTableEntry::EXACT) {
        // Wenn dieser Zug der beste Zug in einem PV-Knoten ist,
        // erhöhe seine relative Vergangenheitsbewertung.
        incrementHistoryScore(bestMove, depth);

        if(!bestMove.isCapture() && !bestMove.isPromotion()) {
            // Setze einen Eintrag in der Konterzug-Tabelle.
            Move previousMove = board.getLastMove();
            if(previousMove.exists()) {
                int previousMoveDestination = previousMove.getDestination();
                int prevoiuslyMovedPieceType = TYPEOF(board.pieceAt(previousMoveDestination));

                setCounterMove(bestMove, board.getSideToMove(), prevoiuslyMovedPieceType, previousMoveDestination);
            }
        }
    }

    return bestScore;
}

int PVSSearchInstance::quiescence(int ply, int alpha, int beta) {
    // Führe die Checkup-Funktion regelmäßig aus.
    if(localNodeCounter >= NODES_PER_CHECKUP && checkupFunction) {
        localNodeCounter = 0;
        checkupFunction();
    }

    // Wir betrachten diesen Knoten.
    nodesSearched.fetch_add(1);
    localNodeCounter++;
    selectiveDepth = std::max(selectiveDepth, (int)ply);

    // Überprüfe, ob wir uns in einer Remisstellung befinden.
    if(evaluator.isDraw())
        return DRAW_SCORE;

    // Wenn die maximale Suchdistanz erreicht wurde,
    // gib die statische Bewertung zurück.
    if(ply >= MAX_PLY)
        return evaluator.evaluate();

    /**
     * Mate Distance Pruning (wie in der normalen Suche):
     * 
     * Wenn wir bereits ein schnelleres Matt in einem
     * anderen Pfad gefunden haben, können wir die Suche
     * abbrechen.
     */
    if(alpha >= MATE_SCORE - ply)
        return alpha; // fail-hard

    if(beta <= -MATE_SCORE + ply)
        return beta; // fail-hard

    // Leere den Eintrag im Suchstack für den aktuellen Abstand zum Wurzelknoten.
    clearSearchStack(ply);

    // Ermittele die vorläufige Bewertung der Position.
    // In der Quieszenzsuche ist die vorläufige Bewertung die
    // statische Bewertung der Position.
    searchStack[ply].preliminaryScore = evaluator.evaluate();

    // Wir betrachten in der Quieszenzsuche (außer wenn wir im Schach stehen)
    // nicht alle Züge. Wenn wir die Bewertung mit den Zügen, die wir betrachten,
    // nur verschlechtern können, geben wir die vorläufige Bewertung unter der
    // Annahme zurück, dass einer der nicht betrachteten Züge zu einer
    // besseren Bewertung führen würde.
    int standPat = searchStack[ply].preliminaryScore;

    // Delta Pruning:
    // Ein sehr großer Materialgewinn in einem Zug reicht nicht aus,
    // um alpha zu erreichen. Wir können die Suche abbrechen.
    if(standPat < alpha - DELTA_MARGIN)
        return standPat;

    // Die beste Bewertung, die wir bisher gefunden haben.
    int bestScore = MIN_SCORE;

    if(!board.isCheck()) {
        // Wenn die statische Bewertung schon über dem Suchfenster
        // liegt, können wir die Suche abbrechen.
        if(standPat >= beta)
            return standPat;

        // Stand Pat darf nur verwendet werden, wenn wir nicht im Schach stehen.
        // Ansonsten betrachten wir alle Züge.
        bestScore = standPat;

        if(standPat > alpha)
            alpha = standPat;
    }

    // Generiere alle Züge, die wir betrachten wollen.
    addMovesToSearchStackInQuiescence(ply, false);

    int moveCount = 0;
    Move move;

    // Schleife über diese Züge.
    for(MoveScorePair& pair : searchStack[ply].moveScorePairs) {
        move = pair.move;

        // Führe den Zug aus und informiere den Evaluator.
        evaluator.updateBeforeMove(move);
        board.makeMove(move);
        evaluator.updateAfterMove();

        // Betrachte den Kindknoten.
        int score = -quiescence(ply + 1, -beta, -alpha);

        // Mache den Zug rückgängig und informiere den Evaluator.
        evaluator.updateBeforeUndo();
        board.undoMove();
        evaluator.updateAfterUndo(move);

        // Überprüfe, ob wir einen Beta-Schnitt machen können.
        if(score >= beta)
            return beta;

        if(score > bestScore) {
            // Wir haben einen neuen besten Zug gefunden.
            bestScore = score;

            if(score > alpha)
                alpha = score;
        }

        moveCount++;
    }

    // Der König steht im Schach und es gibt keine legalen Züge,
    // das Spiel ist vorbei.
    if(moveCount == 0 && board.isCheck())
        return -MATE_SCORE + ply; // Matt

    return bestScore;
}

int PVSSearchInstance::determineLMR(int moveCount, int moveScore, int depth, bool isImproving) {
    // LMR reduziert nie den ersten Zug
    if(moveCount <= 1)
        return 0;

    Move lastMove = board.getLastMove();
    // Wir reduzieren keine Schlagzüge, die unser Zugvorsortierer als neutral/gut bewertet hat
    // oder die einen Bauern aufwerten.
    if(moveScore >= GOOD_CAPTURE_MOVES_MIN || lastMove.isPromotion())
        return 0;

    // Reduziere standardmäßig anhand einer Funktion, die von der Suchtiefe abhängt.
    double reduction = std::log(depth) / std::log(6) + 0.75 + 0.25 * !isImproving;

    // Passe die Reduktion an die relative Vergangenheitsbewertung des Zuges an.
    // -> Bisher bessere Züge werden weniger reduziert und schlechtere Züge stärker.
    int32_t historyScore = getHistoryScore(lastMove, board.getSideToMove() ^ COLOR_MASK);

    // Erhöhe die Reduktion anhand einer logarithmischen Funktion,
    // wenn wir auf mehreren Threads suchen.
    if(numThreads > 1 && historyScore < 0)
        historyScore = historyScore * (1.0 + std::log(numThreads) / std::log(64));

    reduction -= historyScore / 16384.0;

    // Runde die Reduktion ab.
    return (int)reduction;
}

int PVSSearchInstance::determineLMPCount(int depth, bool isCheckEvasion, bool isImproving) {
    // LMP wird nur in geringen Tiefen,
    // nicht in Schachabwehrzügen angewandt.
    if(depth >= 10 || isCheckEvasion)
        return std::numeric_limits<int>::max();

    // Standardmäßig müssen mindestens 5 Züge betrachtet werden.
    int result = 5 + 2 * isImproving;

    // Erhöhe die Anzahl der zu betrachtenden Züge mit der verbleibenden Suchtiefe.
    result += (depth - 1) * (depth - 1) * (4 + 2 * isImproving);

    return result;
}

bool PVSSearchInstance::deactivateNullMove() {
    // Deaktiviere den Null-Zug, wenn der Spieler nur noch
    // einen König und Bauern hat.
    int side = board.getSideToMove();
    return board.getPieceBitboard(side) == board.getPieceBitboard(side | PAWN);
}

bool PVSSearchInstance::verifyNullMove() {
    // Ein Nullzug muss verifiziert werden, wenn der Spieler
    // 2 Leicht-/Schwerfiguren oder weniger hat.
    int side = board.getSideToMove();
    Bitboard ownMajorOrMinorPieces = board.getPieceBitboard(side | KNIGHT) |
                                     board.getPieceBitboard(side | BISHOP) |
                                     board.getPieceBitboard(side | ROOK) |
                                     board.getPieceBitboard(side | QUEEN);

    return ownMajorOrMinorPieces.popcount() <= 2;
}

void PVSSearchInstance::addMovesToSearchStack(int ply, bool useIID, int depth) {
    // Setze den aktuellen Eintrag zurück.
    clearMovesInSearchStack(ply);

    // Suche in der Transpositionstabelle nach einem Hashzug
    // für die aktuelle Position.
    Move hashMove = Move::nullMove();

    if(ply == 0) {
        if(bestRootMoveHint.exists()) {
            // Verwende den vorgegebenen Wurzelzug.
            hashMove = bestRootMoveHint;
            // Setze den Wurzelzug zurück.
            bestRootMoveHint = Move::nullMove();
        } else if(pvTable[0].size() > 0) {
            // Probiere im Wurzelknoten den besten Zug aus der letzten Suche.
            hashMove = pvTable[0].front();
        }
    } else if(useIID) {
        // Wir haben keinen Hashzug für die aktuelle Position gefunden,
        // aber uns ist eine akkurate Zugvorsortierung wichtig.
        // Führe daher eine interne iterative Tiefensuche durch, in
        // der mit einer reduzierten, aber sonst vollständigen Suche
        // ein vorläufig bester Zug bestimmt wird.

        // Bestimme die reduzierte Suchtiefe.
        int reducedDepth = std::min(depth * 2 / 3, depth - 2);

        TranspositionTableEntry entry;
        bool entryExists = transpositionTable.probe(board.getHashValue(), entry);

        if(reducedDepth > 0 && (!entryExists || entry.depth < reducedDepth)) {
            // Führe die interne iterative Tiefensuche durch.
            int iidScore = searchStack[ply].preliminaryScore;

            // Speichere aktuelle Wurzelinformationen.
            unsigned int rootAgeBackup = rootAge;
            int currentSearchDepthBackup = currentSearchDepth;

            // Probiere eine Suche mit Aspirationsfenster.
            int alpha = iidScore - IID_ASPIRATION_WINDOW_SIZE;
            int beta = iidScore + IID_ASPIRATION_WINDOW_SIZE;
            bool alphaAlreadyWidened = false, betaAlreadyWidened = false;

            iidScore = pvs(reducedDepth, ply, alpha, beta, PV_NODE);

            while(iidScore <= alpha || iidScore >= beta) {
                // Wenn die Suche außerhalb des Aspirationsfensters liegt,
                // muss das Fenster erweitert werden.
                if(iidScore <= alpha) {
                    if(alphaAlreadyWidened)
                        alpha = MIN_SCORE;
                    else {
                        alphaAlreadyWidened = true;
                        alpha -= IID_WIDENED_ASPIRATION_WINDOW_SIZE - IID_ASPIRATION_WINDOW_SIZE;
                    }
                } else {
                    if(betaAlreadyWidened)
                        beta = MAX_SCORE;
                    else {
                        betaAlreadyWidened = true;
                        beta += IID_WIDENED_ASPIRATION_WINDOW_SIZE - IID_ASPIRATION_WINDOW_SIZE;
                    }
                }

                iidScore = pvs(reducedDepth, ply, alpha, beta, PV_NODE);
            }

            hashMove = pvTable[ply].front();

            // Stelle die Wurzelinformationen wieder her.
            rootAge = rootAgeBackup;
            currentSearchDepth = currentSearchDepthBackup;

            clearMovesInSearchStack(ply);
        } else {
            // Verwende den Eintrag der Transpositionstabelle, wenn die Suchtiefe zu gering ist.
            if(entryExists)
                hashMove = entry.hashMove;
        }
    } else {
        // Ansonsten suche in der Transpositionstabelle nach einem Hashzug.
        TranspositionTableEntry entry;
        if(transpositionTable.probe(board.getHashValue(), entry))
            hashMove = entry.hashMove;
    }

    if(hashMove.exists() && (ply != 0 || searchMoves.size() == 0 || searchMoves.contains(hashMove))) {
        // Es existiert ein Hashzug für die aktuelle Position.

        // Generiere alle legalen Züge dieser Position.
        Array<Move, 256> moves;
        if(ply > 0 || searchMoves.size() == 0)
            board.generateLegalMoves(moves);
        else
            moves = searchMoves;

        // Füge den Hashzug als ersten Zug mit maximaler Bewertung in die Zugliste ein
        // und bestimme die Reihenfolge der übrigen Züge über unsere Zugvorsortierung.
        if(moves.remove_first(hashMove)) {
            searchStack[ply].hashMove = hashMove;
            searchStack[ply].moveScorePairs.push_back(MoveScorePair(hashMove, HASH_MOVE_SCORE));
        }

        scoreMoves(moves, ply);
    } else {
        // Wir haben keinen Hashzug für die aktuelle Position gefunden
        // und wir sollen kein IID durchführen.
        // => reguläre Vorsortierung der Züge
        Array<Move, 256> moves;
        if(ply > 0 || searchMoves.size() == 0)
            board.generateLegalMoves(moves);
        else
            moves = searchMoves;

        scoreMoves(moves, ply);
    }
}

void PVSSearchInstance::addMovesToSearchStackInQuiescence(int ply, bool includeHashMove) {
    // Setze den aktuellen Eintrag zurück.
    clearMovesInSearchStack(ply);

    int minSEEScore = MIN_SCORE;
    bool isCheck = board.isCheck();
    if(!isCheck)
        minSEEScore = NEUTRAL_SCORE;

    if(includeHashMove) {
        // Wir sollen den Hashzug in die Zugliste einfügen.
        Move hashMove = Move::nullMove();
        TranspositionTableEntry entry;
        if(transpositionTable.probe(board.getHashValue(), entry))
            hashMove = entry.hashMove;

        // Generiere alle restlichen Züge,
        // die wir betrachten wollen und
        // bewerte sie.
        Array<Move, 256> moves;
        if(isCheck)
            board.generateLegalMoves(moves);
        else
            board.generateLegalCaptures(moves);

        if(hashMove.exists()) {
            if(!isCheck && board.isMoveLegal(hashMove)) {
                searchStack[ply].hashMove = hashMove;
                searchStack[ply].moveScorePairs.push_back(MoveScorePair(hashMove, HASH_MOVE_SCORE));
                moves.remove_first(hashMove);
            } else if(moves.remove_first(hashMove)) {
                searchStack[ply].hashMove = hashMove;
                searchStack[ply].moveScorePairs.push_back(MoveScorePair(hashMove, HASH_MOVE_SCORE));
            }
        }

        scoreMovesForQuiescence(moves, ply, minSEEScore);
    } else {
        // Der Hashzug soll nicht explizit betrachtet werden.
        Array<Move, 256> moves;
        if(isCheck)
            board.generateLegalMoves(moves);
        else
            board.generateLegalCaptures(moves);

        scoreMovesForQuiescence(moves, ply, minSEEScore);
    }
}

void PVSSearchInstance::scoreMoves(const Array<Move, 256>& moves, int ply) {
    // Die Zugvorsortierung unterteilt die Züge in drei Kategorien:
    // - gute Schlagzüge (mit SEE >= 0), enthält auch Bauernumwandlungen
    // - Killerzüge (leise Züge, die einen Beta-Schnitt verursacht haben)
    // - verbleibende Züge (leise Züge und schlechte Schlagzüge)

    Array<MoveScorePair, 64> goodCaptures;
    Array<MoveScorePair, 4> killers;
    Array<MoveScorePair, 256> remainingMoves;

    // Bestimme den eingetragenen Konterzug für den
    // letzten gespielten Zug des Gegners.
    Move previousMove = board.getLastMove();
    int previousMoveDestination = previousMove.getDestination();
    int prevoiuslyMovedPieceType = TYPEOF(board.pieceAt(previousMoveDestination));
    Move counterMove = Move::nullMove();
    if(previousMove.exists())
        counterMove = getCounterMove(board.getSideToMove(), prevoiuslyMovedPieceType, previousMoveDestination);

    // Betrachte alle, zu bewertenden Züge.
    for(Move move : moves) {
        int score;

        if(move.isCapture() || move.isPromotion()) {
            // Schlagzüge und Bauernumwandlungen werden mit
            // der Static Exchange Evaluation (SEE) bewertet.
            int seeEvaluation = evaluator.evaluateMoveSEE(move);

            if(seeEvaluation >= 0) {
                // Gute Schlagzüge
                score = std::clamp(GOOD_CAPTURE_MOVES_NEUTRAL + seeEvaluation,
                                   GOOD_CAPTURE_MOVES_MIN,
                                   GOOD_CAPTURE_MOVES_MAX);

                goodCaptures.insert_sorted(MoveScorePair(move, score), std::greater<MoveScorePair>());
            } else {
                // Schlechte Schlagzüge
                score = std::clamp(QUIET_MOVES_NEUTRAL + getHistoryScore(move) / currentSearchDepth + seeEvaluation,
                                   QUIET_MOVES_MIN,
                                   QUIET_MOVES_MAX);

                remainingMoves.insert_sorted(MoveScorePair(move, score), std::greater<MoveScorePair>());
            }
        } else {
            if(isKillerMove(ply, move)) {
                // Killerzüge
                score = KILLER_MOVE_SCORE;
                killers.push_back(MoveScorePair(move, score));
            } else if(ply > 1 && isKillerMove(ply - 2, move)) {
                // Killerzüge aus der vorletzten Tiefe
                score = KILLER_MOVE_SCORE;
                killers.push_back(MoveScorePair(move, score));
            } else {
                // Ruhige Züge. Gebe einen Bonus für den Konterzug.
                score = std::clamp(QUIET_MOVES_NEUTRAL + (getHistoryScore(move) + (move == counterMove) * 300) / currentSearchDepth,
                                   QUIET_MOVES_MIN,
                                   QUIET_MOVES_MAX); // Bewerte anhand der relativen Vergangenheitsbewertung

                remainingMoves.insert_sorted(MoveScorePair(move, score), std::greater<MoveScorePair>());
            }
        }
    }

    // Die Züge wurden bereits sortiert in die Kategorie-Arrays eingefügt,
    // wir müssen sie nur noch auf den Suchstack übertragen.
    searchStack[ply].moveScorePairs.push_back(goodCaptures);
    searchStack[ply].moveScorePairs.push_back(killers);
    searchStack[ply].moveScorePairs.push_back(remainingMoves);
}

void PVSSearchInstance::scoreMovesForQuiescence(const Array<Move, 256>& moves, int ply, int minSEEScore) {
    for(Move move : moves) {
        int score;

        if(move.isCapture() || move.isPromotion()) {
            // Schlagzüge und Bauernumwandlungen werden mit
            // der Static Exchange Evaluation (SEE) bewertet.
            int seeEvaluation = evaluator.evaluateMoveSEE(move);

            if(seeEvaluation >= NEUTRAL_SCORE) {
                // Gute Schlagzüge
                score = std::clamp(GOOD_CAPTURE_MOVES_NEUTRAL + seeEvaluation,
                                    GOOD_CAPTURE_MOVES_MIN,
                                    GOOD_CAPTURE_MOVES_MAX);
            } else {
                // Schlechte Schlagzüge
                score = std::clamp(QUIET_MOVES_NEUTRAL + getHistoryScore(move) / currentSearchDepth + seeEvaluation,
                                   QUIET_MOVES_MIN,
                                   QUIET_MOVES_MAX);
            }

            if(score < minSEEScore)
                continue;
        } else {
            // Ruhige Züge werden anhand ihrer relativen Vergangenheitsbewertung
            // bewertet.
            score = std::clamp(QUIET_MOVES_NEUTRAL + getHistoryScore(move) / currentSearchDepth,
                               QUIET_MOVES_MIN,
                               QUIET_MOVES_MAX);
        }

        searchStack[ply].moveScorePairs.insert_sorted(MoveScorePair(move, score), std::greater<MoveScorePair>());
    }
}