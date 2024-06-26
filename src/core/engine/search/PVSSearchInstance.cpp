#include "core/engine/search/PVSSearchInstance.h"

int16_t PVSSearchInstance::pvs(int16_t depth, uint16_t ply, int16_t alpha, int16_t beta, uint8_t nodeType, int8_t nullMoveCooldown) {
    // Setze die momentane Suchtiefe, wenn wir uns im Wurzelknoten befinden.
    if(ply == 0) {
        currentSearchDepth = std::max(depth / ONE_PLY, 1);
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
    selectiveDepth = std::max(selectiveDepth, ply);

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
    if(entryExists && nodeType != PV_NODE) {
        // Ein Eintrag kann nur verwendet werden, wenn die eingetragene
        // Suchtiefe mindestens so groß ist wie unsere Suchtiefe.
        if(entry.depth * ONE_PLY >= depth) {
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
     * Überprüfe, ob der aktuelle Pfad zu einer plausiblen
     * Position führt, d.h. die vorläufige Bewertung liegt
     * innerhalb oder in der Nähe des Suchfensters.
     */
    bool isPlausibleLine = ply == 0 ||
                           (searchStack[ply - 1].isPlausibleLine &&
                           !board.getLastMove().isNullMove() &&
                           searchStack[ply].preliminaryScore > alpha - 100 &&
                           searchStack[ply].preliminaryScore < beta + 100);

    searchStack[ply].isPlausibleLine = isPlausibleLine;

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
       depth > ONE_PLY && !deactivateNullMove()) {

        int16_t depthReduction = calculateNullMoveReduction(depth, searchStack[ply].preliminaryScore, beta);
        if(depthReduction > ONE_PLY) {
            Move nullMove = Move::nullMove();
            board.makeMove(nullMove);

            int16_t score = -pvs(depth - depthReduction, ply + 1, -beta, -beta + 1, CUT_NODE, NULL_MOVE_COOLDOWN);

            board.undoMove();

            if(score >= beta) {
                if(!verifyNullMove() || depthReduction >= depth + ONE_PLY)
                    return score;

                score = pvs(depth - depthReduction + ONE_PLY, ply, beta - 1, beta, CUT_NODE, std::numeric_limits<int8_t>::max());
                if(score >= beta)
                    return score;
            }
        }
    }

    clearPVTable(ply + 1);

    // Lade den Eintrag aus der Transpositionstabelle neu, da
    // bei der Suche mit mehreren Threads ein neuer Eintrag
    // vorliegen könnte.
    entryExists = transpositionTable.probe(board.getHashValue(), entry);
    if(entryExists && nodeType != PV_NODE) {
        if(entry.depth * ONE_PLY >= depth) {
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
     * Internal Iterative Deepening (wenn ein Hashzug existiert):
     * 
     * Wenn der Hashzug aus einer viel geringeren Suchtiefe stammt,
     * führe eine reduzierte Suche durch, um Suchexplosionen zu vermeiden.
     * Führe außerdem eine reduzierte Suche durch, wenn wir uns in einem
     * PV-Knoten oder einem Cut-Knoten befinden und der Hashzug aus einem
     * All-Knoten mit gleicher oder geringerer Suchtiefe stammt.
     */
    if(entryExists) {
        if((entry.depth + 4) * ONE_PLY <= depth)
            depth -= ONE_PLY;
        else if(nodeType == CUT_NODE && entry.type == TranspositionTableEntry::UPPER_BOUND && entry.depth * ONE_PLY <= depth)
            depth -= ONE_PLY;

        depth = std::max(depth, ONE_PLY);
    }

    /**
     * Internal Iterative Deepening (wenn kein Hashzug existiert
     * oder der Eintrag unzureichend ist):
     * 
     * Führe eine reduzierte Suche durch, um einen besseren
     * Hashzug für die aktuelle Position zu bestimmen.
     * IID wird nur in PV-Knoten und Cut-Knoten mit einer
     * Suchtiefe >= 6 durchgeführt.
     */
    bool fallbackToIID = nodeType == PV_NODE || (nodeType == CUT_NODE && depth >= 6 * ONE_PLY);
    addMovesToSearchStack(ply, fallbackToIID, depth);

    // Prüfe, ob die Suche abgebrochen werden soll.
    if(stopFlag.load() && currentSearchDepth > 1)
        return 0;

    Move move;
    int16_t moveCount = 0, moveScore;
    bool isCheckEvasion = board.isCheck();

    /**
     * Singular Reply Extension (nur in Cut-Knoten):
     * 
     * Wir haben diese Position bereits einmal betrachtet und
     * mit einer Bewertung > alpha bewertet. Wir wollen jetzt
     * herausfinden, ob das der einzig gute Zug ist. Dazu werden
     * alle übrigen Züge mit reduzierter Suchtiefe betrachtet.
     * Wenn alle anderen Züge weit unter alpha bewertet werden,
     * gehen wir davon aus, dass der beste Zug in dieser Position
     * der einzige gute Zug ist und erhöhen seine Suchtiefe.
     */
    int16_t singularExtension = 0;
    int16_t singularDepth = std::min(depth / (2 * ONE_PLY) * ONE_PLY, depth - 4 * ONE_PLY);

    if(nodeType == CUT_NODE && singularDepth > 0 && !isMateScore(alpha) &&
       entryExists && entry.depth * ONE_PLY >= singularDepth && entry.score > alpha &&
       entry.type != TranspositionTableEntry::UPPER_BOUND) {

        int16_t reducedAlpha = alpha - 100;
        singularExtension = ONE_PLY;

        for(MoveScorePair& pair : searchStack[ply].moveScorePairs) {
            move = pair.move;
            moveScore = pair.score;

            if(move == entry.hashMove) // Der Hashzug wurde bereits betrachtet.
                continue;

            // Führe den Zug aus und informiere den Evaluator.
            evaluator.updateBeforeMove(move);
            board.makeMove(move);
            evaluator.updateAfterMove();

            // Durchsuche den Kindknoten mit reduzierter Suchtiefe.
            int16_t score = -pvs(singularDepth, ply + 1, -reducedAlpha - 1, -reducedAlpha, ALL_NODE, nullMoveCooldown);

            // Mache den Zug rückgängig und informiere den Evaluator.
            evaluator.updateBeforeUndo();
            board.undoMove();
            evaluator.updateAfterUndo(move);

            // Überprüfe, ob wir über unserem reduzierten Alpha-Wert liegen.
            if(score > reducedAlpha) {
                if(score >= beta && entry.score >= beta)
                    return beta; // fail-hard

                singularExtension = 0;
                break;
            }
        }
    }

    // Ermittele, ob wir Heuristiken anwenden dürfen, um die Suchtiefe zu erweitern.
    bool allowHeuristicExtensions = extensionsOnPath < currentSearchDepth / (3 - isPlausibleLine) * ONE_PLY;

    uint8_t ttEntryType = TranspositionTableEntry::UPPER_BOUND;
    int16_t bestScore = MIN_SCORE, lmpCount = determineLMPCount(depth, isCheckEvasion, isPlausibleLine);
    Move bestMove;

    // Schleife über alle legalen Züge.
    // Die Züge werden absteigend nach ihrer vorläufigen Bewertung betrachtet,
    // d.h. die besten Züge werden zuerst untersucht.
    for(MoveScorePair& pair : searchStack[ply].moveScorePairs) {
        move = pair.move;
        moveScore = pair.score;

        /**
         * Futility Pruning:
         * Wenn wir uns in einer geringen Suchtiefe befinden, unsere vorläufige
         * Bewertung weit unter alpha liegt und der Zug nicht offensichtlich
         * gut ist (hohe Bewertung in Vorsortierung), dann können wir den Zug überspringen.
         * 
         * Aus taktischen Gründen wird Futility Pruning nicht angewandt, wenn wir
         * uns im Schach oder einem PV-Knoten befinden, wenn der Zug ein Schlagzug ist
         * oder wenn der Zug den Gegner in Schach setzt. Außerdem wird Futility Pruning
         * abgeschaltet, wenn alpha oder beta eine Mattbewertung ist.
         */
        if(depth <= 2 * ONE_PLY && moveScore < KILLER_MOVE_SCORE && !isCheckEvasion &&
           nodeType != PV_NODE && !(isMateScore(alpha) || isMateScore(beta)) && !move.isCapture()) {
            
            int16_t preliminaryEval = searchStack[ply].preliminaryScore;

            // Führe den Zug aus und informiere den Evaluator.
            evaluator.updateBeforeMove(move);
            board.makeMove(move);
            evaluator.updateAfterMove();

            bool isCheck = board.isCheck();

            int16_t futilityMargin = calculateFutilityMargin(depth);
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

        int16_t score;

        /**
         * Late Move Pruning:
         * Späte Züge in All-/Cut-Knoten mit geringer Tiefe, die weder eine Figur schlagen, einen Bauern
         * aufwerten oder den Gegner in Schach setzen, werden sehr wahrscheinlich nicht zu einem
         * Beta-Schnitt führen. Deshalb überspringen wir diese Züge.
         * 
         * Als Failsafe wenden wir LMP nicht an, wenn wir im Schach stehen oder alpha eine Mattbewertung ist.
         * Dadurch vermeiden wir, versehentlich eine Mattverteidigung zu übersehen.
         * Außerdem wird LMP nur auf Züge mit einer neutralen oder negativen Vergangenheitsbewertung angewandt.
         */
        if(nodeType != PV_NODE && moveCount >= lmpCount && !(move.isCapture() || move.isPromotion()) &&
           moveScore <= NEUTRAL_SCORE && !isMateScore(alpha) && !board.isCheck()) {

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
        
        int16_t extension = singularExtension * (moveCount == 0);
        bool isCheck = false, isPassedPawnPush = false;

        /**
         * Erweiterungen:
         * Ähnlich wie bei der Singular Reply Extension, erweitern wir die Suchtiefe
         * unter bestimmten Bedingungen.
         */

        /**
         * Schacherweiterung:
         * Wenn der Zug den Gegner in Schach setzt, erweitern wir die Suchtiefe.
         */
        if(board.isCheck()) {
            // Erweitere die Suchtiefe, wenn der Zug den Gegner in Schach setzt.
            if(extension == 0 && allowHeuristicExtensions)
                extension += ONE_PLY;
            
            isCheck = true;
        } else {
            /**
             * Freibauer-Erweiterung:
             * Wenn der Zug einen Freibauern erzeugt oder bewegt, erweitern wir die Suchtiefe.
             */
            int32_t movedPieceType = TYPEOF(board.pieceAt(move.getDestination()));
            if(movedPieceType == PAWN) {
                int32_t side = board.getSideToMove() ^ COLOR_MASK;
                int32_t otherSide = side ^ COLOR_MASK;
                if(!(sentryMasks[side / COLOR_MASK][move.getDestination()]
                    & board.getPieceBitboard(otherSide | PAWN))) {
                    
                    if(extension == 0 && allowHeuristicExtensions)
                        extension += ONE_PLY;

                    isPassedPawnPush = true;
                }
            }
        }

        extensionsOnPath += extension;

        if(moveCount == 0) {
            // Führe eine vollständige Suche durch.
            score = -pvs(depth - ONE_PLY + extension, ply + 1, -beta, -alpha, childType, nullMoveCooldown - 1);
        } else {
            // Durchsuche die restlichen Kinder mit einem Nullfenster (nur in PV-Knoten) und bei,
            // auf ersten Blick, uninteressanten Zügen mit einer reduzierten Suchtiefe
            // (=> Late Move Reductions).
            int16_t reduction = 0;
            if(extension == 0 && !isCheckEvasion) {
                reduction = determineLMR(moveCount + 1, moveScore, depth);

                if(isCheck || isPassedPawnPush)
                    reduction -= ONE_PLY;

                reduction = std::max(reduction, (int16_t)0);
            }

            score = -pvs(depth - ONE_PLY + extension - reduction, ply + 1, -alpha - 1, -alpha, childType, nullMoveCooldown - 1);

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

                score = -pvs(depth - ONE_PLY + extension, ply + 1, -beta, -alpha, childType, std::max(nullMoveCooldown - 1, 1));
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
                score,
                rootAge,
                (uint8_t)(depth / ONE_PLY),
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
                    int32_t previousMoveDestination = previousMove.getDestination();
                    int32_t prevoiuslyMovedPieceType = TYPEOF(board.pieceAt(previousMoveDestination));

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
        bestScore,
        rootAge,
        (uint8_t)(depth / ONE_PLY),
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
                int32_t previousMoveDestination = previousMove.getDestination();
                int32_t prevoiuslyMovedPieceType = TYPEOF(board.pieceAt(previousMoveDestination));

                setCounterMove(bestMove, board.getSideToMove(), prevoiuslyMovedPieceType, previousMoveDestination);
            }
        }
    }

    return bestScore;
}

int16_t PVSSearchInstance::quiescence(uint16_t ply, int16_t alpha, int16_t beta) {
    // Führe die Checkup-Funktion regelmäßig aus.
    if(localNodeCounter >= NODES_PER_CHECKUP && checkupFunction) {
        localNodeCounter = 0;
        checkupFunction();
    }

    // Wir betrachten diesen Knoten.
    nodesSearched.fetch_add(1);
    localNodeCounter++;
    selectiveDepth = std::max(selectiveDepth, ply);

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
    int16_t standPat = searchStack[ply].preliminaryScore;

    // Delta Pruning:
    // Ein sehr großer Materialgewinn in einem Zug reicht nicht aus,
    // um alpha zu erreichen. Wir können die Suche abbrechen.
    if(standPat < alpha - DELTA_MARGIN)
        return standPat;

    // Die beste Bewertung, die wir bisher gefunden haben.
    int16_t bestScore = MIN_SCORE;

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

    int16_t moveCount = 0;
    Move move;

    // Schleife über diese Züge.
    for(MoveScorePair& pair : searchStack[ply].moveScorePairs) {
        move = pair.move;

        // Führe den Zug aus und informiere den Evaluator.
        evaluator.updateBeforeMove(move);
        board.makeMove(move);
        evaluator.updateAfterMove();

        // Betrachte den Kindknoten.
        int16_t score = -quiescence(ply + 1, -beta, -alpha);

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

int16_t PVSSearchInstance::determineLMR(int16_t moveCount, int16_t moveScore, int16_t depth) {
    // LMR reduziert nie den ersten Zug
    if(moveCount <= 1)
        return 0;

    Move lastMove = board.getLastMove();
    // Wir reduzieren keine Schlagzüge, die unser Zugvorsortierer als neutral/gut bewertet hat
    // oder die einen Bauern aufwerten.
    if(moveScore >= GOOD_CAPTURE_MOVES_MIN || lastMove.isPromotion())
        return 0;

    // Reduziere standardmäßig anhand einer logarithmischen Funktion, die von der Suchtiefe abhängt.
    double reduction = ONE_PLY * (std::log(depth / ONE_PLY) / std::log(6) + 1.0);

    // Passe die Reduktion an die relative Vergangenheitsbewertung des Zuges an.
    // -> Bisher bessere Züge werden weniger reduziert und schlechtere Züge stärker.
    int32_t historyScore = getHistoryScore(lastMove, board.getSideToMove() ^ COLOR_MASK);

    // Dividiere negative Vergangenheitsbewertungen, die für eine Erhöhung der Reduktion sorgen,
    // durch die Anzahl der PVs, um die Reduktion zu verringern. Im MultiPV-Modus wird der Spielbaum
    // für jede PV durchsucht und die Vergangenheitsbewertung wird mehrmals aktualisiert.
    // Durch die Division soll dieser Effekt annähernd ausgeglichen werden.
    if(historyScore < 0)
        historyScore /= numPVs;

    double historyReduction = -historyScore * ONE_PLY / 12000.0;
    historyReduction *= std::log(numThreads) / std::log(16) + 1.0;

    reduction += historyReduction;

    // Runde die Reduktion auf ein Vielfaches von ONE_PLY ab.
    return (int32_t)reduction / ONE_PLY * ONE_PLY;
}

int16_t PVSSearchInstance::determineLMPCount(int16_t depth, bool isCheckEvasion, bool isPlausibleLine) {
    // LMP wird nur in geringen Tiefen,
    // nicht in Schachabwehrzügen angewandt.
    if(depth >= 12 * ONE_PLY || isCheckEvasion)
        return std::numeric_limits<int16_t>::max();

    // Standardmäßig müssen mindestens 5 Züge betrachtet werden.
    int16_t result = 5 + 3 * isPlausibleLine;

    // Erhöhe die Anzahl der zu betrachtenden Züge mit der verbleibenden Suchtiefe.
    result += (depth / ONE_PLY - 1) * (3 + 2 * isPlausibleLine);

    return result;
}

bool PVSSearchInstance::deactivateNullMove() {
    // Deaktiviere den Null-Zug, wenn der Spieler nur noch
    // einen König und Bauern hat.
    int32_t side = board.getSideToMove();
    return board.getPieceBitboard(side) == board.getPieceBitboard(side | PAWN);
}

bool PVSSearchInstance::verifyNullMove() {
    // Ein Nullzug muss verifiziert werden, wenn der Spieler
    // 2 Leicht-/Schwerfiguren oder weniger hat.
    int32_t side = board.getSideToMove();
    Bitboard ownMajorOrMinorPieces = board.getPieceBitboard(side | KNIGHT) |
                                     board.getPieceBitboard(side | BISHOP) |
                                     board.getPieceBitboard(side | ROOK) |
                                     board.getPieceBitboard(side | QUEEN);

    return ownMajorOrMinorPieces.popcount() <= 2;
}

void PVSSearchInstance::addMovesToSearchStack(uint16_t ply, bool useIID, int16_t depth) {
    // Setze den aktuellen Eintrag zurück.
    clearMovesInSearchStack(ply);

    // Suche in der Transpositionstabelle nach einem Hashzug
    // für die aktuelle Position.
    Move hashMove = Move::nullMove();

    if(ply == 0 && pvTable[0].size() > 0) {
        // Probiere im Wurzelknoten den besten Zug aus der letzten Suche.
        hashMove = pvTable[0].front();
    } else {
        // Ansonsten suche in der Transpositionstabelle nach einem Hashzug.
        TranspositionTableEntry entry;
        if(transpositionTable.probe(board.getHashValue(), entry))
            hashMove = entry.hashMove;

        // Der Eintrag ist aufgrund der Suchtiefe unzureichend.
        // Bestimme einen genaueren Hashzug über IID (falls useIID == true).
        if(useIID && hashMove.exists() && entry.depth * ONE_PLY <= depth - 6 * ONE_PLY)
            hashMove = Move::nullMove();
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
        } else if(useIID) {
            // Im Fall einer super seltenen Hashkollision, ist der Hashzug nicht legal
            // in unserer aktuellen Position. Wenn IID in diesem Fall durchgeführt werden
            // soll, dann springe dahin.
            goto iid;
        }

        scoreMoves(moves, ply);
    } else if(useIID) {
        iid:
        
        // Wir haben keinen Hashzug für die aktuelle Position gefunden,
        // aber uns ist eine akkurate Zugvorsortierung wichtig.
        // Führe daher eine interne iterative Tiefensuche durch, in
        // der mit einer reduzierten, aber sonst vollständigen Suche
        // ein vorläufig bester Zug bestimmt wird.

        // Bestimme die reduzierte Suchtiefe.
        int16_t reducedDepth = depth - 3 * ONE_PLY;

        Move iidMove = Move::nullMove();

        if(reducedDepth > 0) {
            // Führe die interne iterative Tiefensuche durch.
            int16_t iidScore = searchStack[ply].preliminaryScore;

            // Speichere aktuelle Wurzelinformationen.
            uint16_t rootAgeBackup = rootAge;
            int16_t currentSearchDepthBackup = currentSearchDepth;

            // Probiere eine Suche mit Aspirationsfenster.
            int16_t alpha = iidScore - IID_ASPIRATION_WINDOW_SIZE;
            int16_t beta = iidScore + IID_ASPIRATION_WINDOW_SIZE;
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

            iidMove = pvTable[ply].front();

            // Stelle die Wurzelinformationen wieder her.
            rootAge = rootAgeBackup;
            currentSearchDepth = currentSearchDepthBackup;
        }

        // Füge den besten Zug aus der internen Tiefensuche
        // als Hashzug in den Suchstack ein.
        searchStack[ply].hashMove = iidMove;
        searchStack[ply].moveScorePairs.clear();

        // Generiere und bewerte die restlichen Züge.
        Array<Move, 256> moves;
        if(ply > 0 || searchMoves.size() == 0)
            board.generateLegalMoves(moves);
        else
            moves = searchMoves;

        // Entferne den Hashzug aus der Zugliste, falls er existiert.
        // Im Fall eines Schachmatts existiert kein IID-Zug,
        // dieser Sonderfall wird hier überprüft.
        if(iidMove.exists() && moves.remove_first(iidMove))
            searchStack[ply].moveScorePairs.push_back(MoveScorePair(iidMove, HASH_MOVE_SCORE));
        
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

void PVSSearchInstance::addMovesToSearchStackInQuiescence(uint16_t ply, bool includeHashMove) {
    // Setze den aktuellen Eintrag zurück.
    clearMovesInSearchStack(ply);

    int16_t minSEEScore = MIN_SCORE;
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

void PVSSearchInstance::scoreMoves(const Array<Move, 256>& moves, uint16_t ply) {
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
    int32_t previousMoveDestination = previousMove.getDestination();
    int32_t prevoiuslyMovedPieceType = TYPEOF(board.pieceAt(previousMoveDestination));
    Move counterMove = Move::nullMove();
    if(previousMove.exists())
        counterMove = getCounterMove(board.getSideToMove(), prevoiuslyMovedPieceType, previousMoveDestination);

    // Betrachte alle, zu bewertenden Züge.
    for(Move move : moves) {
        int16_t score;

        if(move.isCapture() || move.isPromotion()) {
            // Schlagzüge und Bauernumwandlungen werden mit
            // der Static Exchange Evaluation (SEE) bewertet.
            int16_t seeEvaluation = evaluator.evaluateMoveSEE(move);

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

void PVSSearchInstance::scoreMovesForQuiescence(const Array<Move, 256>& moves, uint16_t ply, int16_t minSEEScore) {
    for(Move move : moves) {
        int16_t score;

        if(move.isCapture() || move.isPromotion()) {
            // Schlagzüge und Bauernumwandlungen werden mit
            // der Static Exchange Evaluation (SEE) bewertet.
            int32_t seeEvaluation = evaluator.evaluateMoveSEE(move);

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