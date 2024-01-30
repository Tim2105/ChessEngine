#include "core/engine/search/PVSSearchInstance.h"

int16_t PVSSearchInstance::pvs(int16_t depth, uint16_t ply, int16_t alpha, int16_t beta, bool allowNullMove, uint8_t nodeType) {
    // Setze die momentane Suchtiefe, wenn wir uns im Wurzelknoten befinden.
    if(ply == 0)
        currentSearchDepth = depth / ONE_PLY;

    // Führe die Checkup-Funktion regelmäßig aus.
    if(localNodeCounter >= NODES_PER_CHECKUP && checkupFunction) {
        localNodeCounter = 0;
        checkupFunction();
    }

    // Prüfe, ob die Suche abgebrochen werden soll.
    if(stopFlag.load())
        return 0;

    // Wenn die Suchtiefe 0 oder die maximale Suchdistanz erreicht wurde, bestimme die Bewertung
    // des aktuellen Knotens mit einer Quieszenzsuche.
    if(depth <= 0 || ply >= MAX_PLY)
        return quiescence(ply, alpha, beta);

    // Wir betrachten diesen Knoten.
    nodesSearched.fetch_add(1);
    localNodeCounter++;

    // Überprüfe, ob dieser Knoten zu einem Unentschieden durch
    // dreifache Stellungswiederholung oder die 50-Züge-Regel führt.
    uint16_t repetitionCount = board.repetitionCount();
    if(repetitionCount >= 3 || board.getFiftyMoveCounter() >= 100)
        return DRAW_SCORE;

    /**
     * Mate Distance Pruning:
     * 
     * Wenn unser alpha größer oder gleich als MATE_SCORE - ply ist,
     * können wir den Knoten abbrechen, weil wir in diesem
     * Fall auf keinen Fall alpha anheben können (wir haben
     * bereits ein schnelleres Matt gefunden).
     */
    if(alpha >= MATE_SCORE - ply)
        return alpha; // fail-hard

    // Suche in der Transpositionstabelle nach einem Eintrag für
    // die aktuelle Position.
    TranspositionTableEntry entry;
    if((nodeType != PV_NODE || !isMainThread) && transpositionTable.probe(board.getHashValue(), entry)) {
        // Ein Eintrag kann nur verwendet werden, wenn die eingetragene
        // Suchtiefe mindestens so groß ist wie unsere Suchtiefe.
        if(entry.depth * ONE_PLY >= depth) {
            if(entry.type == TranspositionTableEntry::EXACT) {
                // Der Eintrag speichert eine exakte Bewertung,
                // d.h. er stammt aus einem PV-Knoten.
                addPVMove(ply, entry.hashMove);
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
     * Null-Move-Pruning:
     * Wir gehen davon aus, dass ein Spieler mit einem Zug
     * immer seine Position verbessern kann (=> Nullzugobservation).
     * Wenn wir also unseren Zug überspringen (=> Nullzug) und in einer
     * reduzierten Suche immer noch >= beta sind,
     * gucken wir uns diesen Knoten nicht vollständig an.
     * 
     * In Positionen mit Zugzwang hält die Nullzugobservation
     * nicht, daher deaktivieren wir die Heuristik wenn wir
     * nur noch einen König und Bauern haben.
     */
    if(allowNullMove && !board.isCheck() &&
       depth > ONE_PLY && !deactivateNullMove()) {

        int16_t staticEvaluation = getStaticEvalInSearchStack(ply);
        if(staticEvaluation > beta) {
            Move nullMove = Move::nullMove();
            board.makeMove(nullMove);

            int16_t score = -pvs(depth - calculateNullMoveReduction(depth, staticEvaluation, beta),
                                 ply + 1, -beta, -beta + 1, false, CUT_NODE);

            board.undoMove();

            if(score >= beta)
                return score;
        }
    }

    clearPVTable(ply + 1);

    // Generiere alle legalen Züge dieser Position.
    // In PV-Knoten und Cut-Knoten mit höherer Tiefe soll über
    // interne Iterative Tiefensuche (IID) der beste Zug genauer vorhergesagt werden,
    // wenn kein Hashzug existiert.
    bool fallbackToIID = nodeType == PV_NODE || (nodeType == CUT_NODE && depth >= 12 * ONE_PLY);
    addMovesToSearchStack(ply, fallbackToIID, depth);

    // Prüfe, ob die Suche abgebrochen werden soll.
    if(stopFlag.load())
        return 0;

    Move move;
    int16_t moveCount = 0, moveScore;

    uint8_t ttEntryType = TranspositionTableEntry::UPPER_BOUND;
    int16_t bestScore = MIN_SCORE;
    moveCount = 0;
    Move bestMove;
    bool isCheckEvasion = board.isCheck();

    // Schleife über alle legalen Züge.
    // Die Züge werden absteigend nach ihrer vorläufigen Bewertung betrachtet,
    // d.h. die besten Züge werden zuerst untersucht.
    for(MoveScorePair& pair : searchStack[ply].moveScorePairs) {
        move = pair.move;
        moveScore = pair.score;

        // Führe den Zug aus und informiere den Evaluator.
        evaluator.updateBeforeMove(move);
        board.makeMove(move);
        evaluator.updateAfterMove();

        int16_t score;

        // Sage den Knotentyp des Kindknotens voraus.
        uint8_t childType = CUT_NODE;
        if(nodeType == PV_NODE && moveCount == 0)
            childType = PV_NODE;
        else if(nodeType == CUT_NODE && moveScore >= KILLER_MOVE_SCORE)
            childType = ALL_NODE;
        
        // Erweitere die Suchtiefe, wenn der Zug den Gegner in Schach setzt.
        int16_t extension = 0;
        if(board.isCheck())
            extension += ONE_PLY;

        if(moveCount == 0) {
            // Durchsuche das erste Kind vollständig.
            score = -pvs(depth - ONE_PLY + extension, ply + 1, -beta, -alpha, true, childType);
        } else {
            // Durchsuche die restlichen Kinder mit einem Nullfenster (nur in PV-Knoten) und bei,
            // auf ersten Blick, uninteressanten Zügen mit einer reduzierten Suchtiefe
            // (=> Late Move Reductions).
            int16_t reduction = 0;
            if(depth >= 3 * ONE_PLY && extension == 0 && !isCheckEvasion)
                reduction = determineLMR(moveCount + 1, moveScore, nodeType);

            score = -pvs(depth - ONE_PLY + extension - reduction, ply + 1, -alpha - 1, -alpha, true, childType);

            // Wenn die Bewertung entweder über dem Nullfenster liegt,
            // führe eine vollständige Suche durch, wenn:
            // - die Suchtiefe dieses Kindes reduziert wurde (reduction > 0)
            // - oder die Bewertung liegt dieses Knotens innerhalb des Suchfensters
            //   liegt (< beta)
            if(score > alpha && (reduction > 0 || score < beta)) {
                if(nodeType == PV_NODE)
                    childType = PV_NODE;
                else
                    childType = ALL_NODE;

                score = -pvs(depth - ONE_PLY + extension, ply + 1, -beta, -alpha, false, childType);
            }
        }

        // Skaliere die Bewertung in Richtung 0, wenn der Zug einmal wiederholt wurde.
        // Dadurch kann die Suche Dauerschach (als Strategie) schneller erkennen.
        if(repetitionCount >= 2)
            score /= 2;

        // Skaliere die Bewertung in Richtung 0, wenn wir uns der 50-Züge-Regel annähern.
        // (Starte erst nach 10 Zügen, damit die Bewertung nicht zu früh verzerrt wird.)
        int32_t fiftyMoveCounter = board.getFiftyMoveCounter();
        if(fiftyMoveCounter > 20 && !isMateScore(score))
            score = (int32_t)score * (100 - fiftyMoveCounter) / 80;

        // Mache den Zug rückgängig und informiere den Evaluator.
        evaluator.updateBeforeUndo();
        board.undoMove();
        evaluator.updateAfterUndo(move);

        // Prüfe, ob die Suche abgebrochen werden soll.
        if(stopFlag.load())
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
                board.getPly(),
                (uint8_t)(depth / ONE_PLY),
                TranspositionTableEntry::LOWER_BOUND
            );

            transpositionTable.put(board.getHashValue(), entry);

            // Speichere den Zug in der Killerzug-Tabelle,
            // wenn es sich um einen "leisen" Zug handelt.
            if(!move.isCapture() && !move.isPromotion())
                addKillerMove(ply, move);

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
        board.getPly(),
        (uint8_t)(depth / ONE_PLY),
        ttEntryType
    };

    transpositionTable.put(board.getHashValue(), entry);

    // Wenn dieser Zug der beste Zug in einem PV-Knoten ist,
    // erhöhe seine relative Vergangenheitsbewertung.
    if(ttEntryType == TranspositionTableEntry::EXACT)
        incrementHistoryScore(bestMove, depth);

    return bestScore;
}

int16_t PVSSearchInstance::quiescence(int16_t ply, int16_t alpha, int16_t beta) {
    // Führe die Checkup-Funktion regelmäßig aus.
    if(localNodeCounter >= NODES_PER_CHECKUP && checkupFunction) {
        localNodeCounter = 0;
        checkupFunction();
    }

    // Wir betrachten diesen Knoten.
    nodesSearched.fetch_add(1);
    localNodeCounter++;

    // Überprüfe, ob dieser Knoten zu einem Unentschieden durch
    // dreifache Stellungswiederholung oder die 50-Züge-Regel führt.
    if(board.repetitionCount() >= 3 || board.getFiftyMoveCounter() >= 100)
        return DRAW_SCORE;

    /**
     * Mate Distance Pruning (wie in der normalen Suche):
     * 
     * Wenn unser alpha größer oder gleich als MATE_SCORE - ply ist,
     * können wir den Knoten abbrechen, weil wir in diesem
     * Fall auf keinen Fall alpha anheben können (wir haben
     * bereits ein schnelleres Matt gefunden).
     */
    if(alpha >= MATE_SCORE - ply)
        return alpha; // fail-hard

    // Leere den Eintrag im Suchstack für den aktuellen Abstand zum Wurzelknoten.
    clearSearchStack(ply);

    // Die aktuelle statische Bewertung der Position (=> Stand Pat Score).
    // Wir betrachten in der Quieszenzsuche (außer wenn wir im Schach stehen)
    // nicht alle Züge. Wenn wir die Bewertung mit den Zügen, die wir betrachten,
    // nur verschlechtern können, geben wir die statische Bewertung unter der
    // Annahme zurück, dass einer der nicht betrachteten Züge zu einer
    // besseren Bewertung führen würde.
    int16_t standPat = getStaticEvalInSearchStack(ply);

    // Wenn die msximale Suchdiaztanz erreicht wurde,
    // gib die statische Bewertung zurück.
    if(ply >= MAX_PLY)
        return standPat;

    // Wenn die statische Bewertung schon über dem Suchfenster
    // liegt, können wir die Suche abbrechen.
    if(standPat >= beta)
        return beta;

    // Delta Pruning:
    // Der größtmögliche Materialgewinn in einem Zug reicht nicht aus,
    // um alpha zu erreichen. Wir können die Suche abbrechen.
    if(standPat < alpha - DELTA_MARGIN)
        return standPat;

    // Die beste Bewertung, die wir bisher gefunden haben.
    int16_t bestScore = MIN_SCORE;
    int16_t minMoveScore = MIN_SCORE;

    if(!board.isCheck()) {
        // Stand Pat darf nur verwendet werden, wenn wir nicht im Schach stehen.
        // Ansonsten betrachten wir alle Züge.
        bestScore = standPat;

        // Wenn wir uns nicht im Schach befinden, betrachten wur nur Schlagzüge,
        // die unser Zugvorsortierer als neutral oder besser bewertet hat.
        minMoveScore = NEUTRAL_SCORE;

        if(standPat > alpha)
            alpha = standPat;
    }

    // Generiere alle Züge, die wir betrachten wollen.
    addMovesToSearchStackInQuiescence(ply, minMoveScore, false);

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

int16_t PVSSearchInstance::determineLMR(int16_t moveCount, int16_t moveScore, uint8_t nodeType) {
    // LMR reduziert nie den ersten Zug
    if(moveCount == 1)
        return 0;

    Move lastMove = board.getLastMove();
    // Wir reduzieren keine Schlagzüge, die unser Zugvorsortierer als neutral/gut bewertet hat
    // oder die einen Bauern aufwerten.
    if(moveScore >= GOOD_CAPTURE_MOVES_MIN || lastMove.isPromotion())
        return 0;
    else {
        // Überprüfe, ob der letzte Zug einen Freibauern bewegt hat.
        // Freibauerzüge werden nie reduziert, weil sie im Endspiel
        // in der Regel das Spiel entscheiden.
        int32_t movedPieceType = TYPEOF(board.pieceAt(lastMove.getDestination()));
        if(movedPieceType == PAWN) {
            int32_t side = board.getSideToMove() ^ COLOR_MASK;
            int32_t otherSide = side ^ COLOR_MASK;
            if(!(sentryMasks[side / COLOR_MASK][lastMove.getDestination()]
                & board.getPieceBitboard(otherSide | PAWN)))
                return 0;
        }
    }

    // Reduziere standardmäßig um eine zusätzliche Tiefe.
    int32_t reduction = ONE_PLY;

    // Passe die Reduktion an die relative Vergangenheitsbewertung des Zuges an.
    // -> Bessere Züge werden weniger reduziert und schlechtere Züge mehr.
    int32_t historyScore = getHistoryScore(lastMove, board.getSideToMove() ^ COLOR_MASK);
    int32_t historyReduction = -historyScore / 16384 * ONE_PLY;
    historyReduction = std::clamp(historyReduction, -ONE_PLY, 2 * ONE_PLY);
    reduction += historyReduction;

    // Erhöhe die Reduktion in erwarteten Cut-Knoten.
    if(nodeType == CUT_NODE) {
        reduction += ONE_PLY;

        if(moveScore < NEUTRAL_SCORE)
            reduction += ONE_PLY;
    }

    return reduction;
}

bool PVSSearchInstance::deactivateNullMove() {
    // Deaktiviere den Null-Zug, wenn der Spieler nur noch
    // einen König und Bauern hat.
    int32_t side = board.getSideToMove();
    Bitboard ownPieces = side == WHITE ? board.getWhiteOccupiedBitboard() : board.getBlackOccupiedBitboard();
    return ownPieces == board.getPieceBitboard(side | PAWN);
}

void PVSSearchInstance::addMovesToSearchStack(uint16_t ply, bool useIID, int16_t depth) {
    // Setze den aktuellen Eintrag zurück.
    clearMovesInSearchStack(ply);

    // Suche in der Transpositionstabelle nach einem Hashzug
    // für die aktuelle Position.
    Move hashMove = Move::nullMove();

    if(ply == 0 && pvTable[0].size() > 0 && pvTable[0].front().exists()) {
        // Probiere im Wurzelknoten den besten Zug aus der letzten Suche.
        hashMove = pvTable[0].front();
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
        int16_t reducedDepth = std::min(depth / 2, depth - 3 * ONE_PLY);

        Move iidMove = Move::nullMove();

        if(reducedDepth > 0) {
            // Führe die interne iterative Tiefensuche durch.
            int16_t iidScore = getStaticEvalInSearchStack(ply);

            Array<Move, 256> searchMoves;

            // Erstelle eine neue Suchinstanz für die interne iterative Tiefensuche.
            PVSSearchInstance iidInstance(board, transpositionTable, stopFlag, startTime, stopTime,
                                          nodesSearched, searchMoves, checkupFunction);

            // Die IID-Instanz soll nicht als Hauptinstanz betrachtet werden, damit
            // sie in PV-Knoten Transpositionseinträge verwenden darf
            // (wir müssen ja keine vollständige PV konstruieren).
            iidInstance.setMainThread(false);

            for(int16_t d = ONE_PLY; d <= reducedDepth; d += ONE_PLY) {
                // Prüfe, ob die Suche abgebrochen werden soll.
                if(stopFlag.load())
                    return;

                // Probiere eine Suche mit Aspirationsfenster.
                int16_t alpha = iidScore - IID_ASPIRATION_WINDOW_SIZE;
                int16_t beta = iidScore + IID_ASPIRATION_WINDOW_SIZE;
                bool alphaAlreadyWidened = false, betaAlreadyWidened = false;

                iidScore = iidInstance.pvs(d, 0, alpha, beta, true, PV_NODE);

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

                    iidScore = iidInstance.pvs(d, 0, alpha, beta, true, PV_NODE);
                }
            }

            iidMove = iidInstance.getPV().front();
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
        // Im Fall eines Schachmatts kann der IID-Zug ein alter,
        // Eintrag in der PV-Tabelle sein, der nicht legal ist.
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

void PVSSearchInstance::addMovesToSearchStackInQuiescence(uint16_t ply, int16_t minMoveScore, bool includeHashMove) {
    // Setze den aktuellen Eintrag zurück.
    clearMovesInSearchStack(ply);

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
        if(board.isCheck())
            board.generateLegalMoves(moves);
        else
            board.generateLegalCaptures(moves);

        if(hashMove.exists() && moves.remove_first(hashMove)) {
            searchStack[ply].hashMove = hashMove;
            searchStack[ply].moveScorePairs.push_back(MoveScorePair(hashMove, HASH_MOVE_SCORE));
        }

        scoreMovesForQuiescence(moves, ply, minMoveScore);
    } else {
        // Der Hashzug soll nicht explizit betrachtet werden.
        // (Er könnte aber trotzdem in der Zugliste auftauchen.)
        Array<Move, 256> moves;
        if(board.isCheck())
            board.generateLegalMoves(moves);
        else
            board.generateLegalCaptures(moves);

        scoreMovesForQuiescence(moves, ply, minMoveScore);
    }
}

void PVSSearchInstance::scoreMoves(const Array<Move, 256>& moves, uint16_t ply) {
    // Die Zugvorsortierung unterteilt die Züge in vier Kategorien:
    // - gute Schlagzüge (mit SEE >= 0)
    // - schlechte Schlagzüge (mit SEE < 0)
    // - Killerzüge
    // - ruhige Züge (alle verbleibenden Züge)

    Array<MoveScorePair, 64> goodCaptures;
    Array<MoveScorePair, 64> badCaptures;
    Array<MoveScorePair, 256> quietMoves;
    Array<MoveScorePair, 2> killers;

    // Betrachte alle, zu bewertenden Züge.
    for(Move move : moves) {
        int16_t score;

        // Hilfsthread im Lazy SMP-Algortihmus fügen eine leichte
        // Bewertungsverzerrung hinzu, um von anderen Threads zu divergieren.
        int16_t scoreDistortion = 0;
        if(!isMainThread)
            scoreDistortion = mersenneTwister() % (2 * MAX_MOVE_SCORE_DISTORTION + 1) - MAX_MOVE_SCORE_DISTORTION;

        if(move.isCapture() || move.isPromotion()) {
            // Schlagzüge und Bauernumwandlungen werden mit
            // der Static Exchange Evaluation (SEE) bewertet.

            uint64_t nodesSearchedBySEE = 0;
            int16_t seeEvaluation = evaluator.evaluateMoveSEE(move, nodesSearchedBySEE);
            localNodeCounter += nodesSearchedBySEE;
            nodesSearched.fetch_add(nodesSearchedBySEE);

            if(seeEvaluation >= 0) {
                // Gute Schlagzüge
                score = std::clamp(GOOD_CAPTURE_MOVES_NEUTRAL + seeEvaluation + scoreDistortion,
                                   GOOD_CAPTURE_MOVES_MIN,
                                   GOOD_CAPTURE_MOVES_MAX);

                goodCaptures.insert_sorted(MoveScorePair(move, score), std::greater<MoveScorePair>());
            } else {
                // Schlechte Schlagzüge
                score = std::clamp(BAD_CAPTURE_MOVES_NEUTRAL + seeEvaluation + scoreDistortion,
                                   BAD_CAPTURE_MOVES_MIN,
                                   BAD_CAPTURE_MOVES_MAX);

                badCaptures.insert_sorted(MoveScorePair(move, score), std::greater<MoveScorePair>());
            }
        } else {
            if(isKillerMove(ply, move)) {
                // Killerzüge
                score = KILLER_MOVE_SCORE;
                killers.push_back(MoveScorePair(move, score));
            } else {
                // Ruhige Züge
                score = std::clamp(QUIET_MOVES_NEUTRAL + getHistoryScore(move) / currentSearchDepth + scoreDistortion,
                                   QUIET_MOVES_MIN,
                                   QUIET_MOVES_MAX);

                quietMoves.insert_sorted(MoveScorePair(move, score), std::greater<MoveScorePair>());
            }
        }
    }

    // Die Züge wurden bereits sortiert in die Kategorie-Arrays eingefügt,
    // wir müssen sie nur noch auf den Suchstack übertragen.
    searchStack[ply].moveScorePairs.push_back(goodCaptures);
    searchStack[ply].moveScorePairs.push_back(killers);
    searchStack[ply].moveScorePairs.push_back(badCaptures);
    searchStack[ply].moveScorePairs.push_back(quietMoves);
}

void PVSSearchInstance::scoreMovesForQuiescence(const Array<Move, 256>& moves, uint16_t ply, int16_t minMoveScore) {
    for(Move move : moves) {
        int16_t score;

        if(move.isCapture() || move.isPromotion()) {
            // Schlagzüge und Bauernumwandlungen werden mit
            // der Static Exchange Evaluation (SEE) bewertet.

            uint64_t nodesSearchedBySEE = 0;
            score = evaluator.evaluateMoveSEE(move, nodesSearchedBySEE);
            localNodeCounter += nodesSearchedBySEE;
            nodesSearched.fetch_add(nodesSearchedBySEE);
        } else {
            // Ruhige Züge werden anhand ihrer relativen Vergangenheitsbewertung
            // bewertet.

            score = std::clamp(QUIET_MOVES_NEUTRAL + getHistoryScore(move) / currentSearchDepth,
                               QUIET_MOVES_MIN,
                               QUIET_MOVES_MAX);
        }

        if(score >= minMoveScore)
            searchStack[ply].moveScorePairs.insert_sorted(MoveScorePair(move, score), std::greater<MoveScorePair>());
    }
}