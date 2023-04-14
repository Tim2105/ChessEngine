#include "core/engine/ScoutEngine.h"
#include <thread>
#include <algorithm>
#include <cmath>
#include "core/utils/MoveNotations.h"
#include "core/chess/MailboxDefinitions.h"

void ScoutEngine::searchTimer(uint32_t searchTime) {
    auto start = std::chrono::system_clock::now();

    while(searching) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        auto end = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        if(elapsed >= searchTime && searchTime > 0 && variations.size() > 0)
            searching = false;
    }
}

void ScoutEngine::clearRelativeHistory() {
    for(int i = 0; i < 2; i++) {
        for(int j = 0; j < 64; j++) {
            for(int k = 0; k < 64; k++) {
                relativeHistory[i][j][k] = 0;
            }
        }
    }
}

void ScoutEngine::clearPvTable() {
    for(int i = 0; i < 64; i++) {
        pvTable[i].clear();
    }
}

void ScoutEngine::clearKillerMoves() {
    for(int i = 0; i < 256; i++) {
        killerMoves[i][0] = Move();
        killerMoves[i][1] = Move();
    }
}

void ScoutEngine::runSearch(const std::function<void()> callback, bool timeControl, uint32_t minTime, uint32_t maxTime) {
    std::vector<Variation> principalVariationHistory;

    int16_t score = evaluator.evaluate();

    auto start = std::chrono::system_clock::now();

    for(int16_t depth = ONE_PLY; searching && depth < (MAX_PLY * ONE_PLY); depth += ONE_PLY) {
        currentMaxDepth = depth;

        score = rootSearch(depth, score);

        auto end = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        // std::cout << "info depth " << depth / ONE_PLY << " score cp " << score << " time " << elapsed << " nodes " << nodesSearched << " pv ";

        // for(std::string move : variationToFigurineAlgebraicNotation(getPrincipalVariation(), searchBoard)) {
        //     std::cout << move << " ";
        // }

        // std::cout << std::endl;

        principalVariationHistory.push_back({
                getPrincipalVariation(),
                getBestMoveScore()
        });

        if(timeControl && !extendSearchUnderTimeControl(principalVariationHistory, minTime, maxTime, elapsed))
            break;
    }

    if(searching) {
        // Wenn die Suche vorzeitig beendet wurde weil die maximale Tiefe erreicht wurde, dann wurde die letzte durchsuchte Tiefe vervollständigt
        currentMaxDepth += ONE_PLY;
    }

    searching = false;

    callback();
}

bool ScoutEngine::extendSearchUnderTimeControl(std::vector<Variation> pvHistory, uint32_t minTime, uint32_t maxTime, uint32_t timeSpent) {
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
    // wenn die Standardabweichung der Bewertungen der letzten 5 Durchläufe größer als 30 ist
    if(bestMoveChanges >= 3 && scoreStandardDeviation > 30.0 * timeFactor)
        return true;

    // Wenn sich der beste Zug in den letzten 5 Durchläufen mindestens 2 mal geändert hat, dann wird die Suche verlängert,
    // wenn die Standardabweichung der Bewertungen der letzten 5 Durchläufe größer als 45 ist
    if(bestMoveChanges >= 2 && scoreStandardDeviation > 45.0 * timeFactor)
        return true;

    // Wenn sich der beste Zug in den letzten 5 Durchläufen mindestens 1 mal geändert hat, dann wird die Suche verlängert,
    // wenn die Standardabweichung der Bewertungen der letzten 5 Durchläufe größer als 60 ist
    if(bestMoveChanges >= 1 && scoreStandardDeviation > 60.0 * timeFactor)
        return true;

    // Ansonsten wird die Suche nicht verlängert
    return false;
}

void ScoutEngine::search(uint32_t searchTime, bool treatAsTimeControl, bool dontBlock) {
    search(searchTime, []() {}, treatAsTimeControl, dontBlock);
}

void ScoutEngine::search(uint32_t searchTime, std::function<void()> callback, bool treatAsTimeControl, bool dontBlock) {
    stop();

    searching = true;
    currentMaxDepth = 0;
    currentAge = board->getPly();
    nodesSearched = 0;
    mateDistance = MAX_PLY;

    searchBoard = *board;
    evaluator.setBoard(searchBoard);

    clearRelativeHistory();
    clearKillerMoves();
    clearVariations();

    if(treatAsTimeControl) {
        int32_t numLegalMoves = searchBoard.generateLegalMoves().size();

        double oneThirtiethOfSearchTime = searchTime * 0.0333;
        double oneFourthOfSearchTime = searchTime * 0.25;

        uint32_t minTime = oneThirtiethOfSearchTime - oneThirtiethOfSearchTime * exp(-0.05  * (numLegalMoves - 1));
        uint32_t maxTime = oneFourthOfSearchTime - oneFourthOfSearchTime * exp(-0.05  * (numLegalMoves - 1));

        int32_t timeFacArrayIndex = std::min(numLegalMoves, timeFactorArraySize) - 1;

        minTime *= timeFactor[timeFacArrayIndex];
        maxTime *= timeFactor[timeFacArrayIndex];

        // Puffer von 30 ms für das Starten und Stoppen der Threads einberechnen
        maxTime = std::max(0u, maxTime - 30);

        if(minTime > maxTime)
            minTime = maxTime;

        std::cout << "Min time: " << minTime << " ms | Max time: " << maxTime << " ms" << std::endl;

        timerThread = std::thread(std::bind(&ScoutEngine::searchTimer, this, maxTime));

        searchThread = std::thread(std::bind(&ScoutEngine::runSearch, this, callback, true, minTime, maxTime));
    } else {
        if(searchTime > 0)
            timerThread = std::thread(std::bind(&ScoutEngine::searchTimer, this, searchTime));

        searchThread = std::thread(std::bind(&ScoutEngine::runSearch, this, callback, false, 0, 0));
    }

    if(!dontBlock) {
        if(timerThread.joinable())
            timerThread.join();

        if(searchThread.joinable())
            searchThread.join();

        evaluator.setBoard(*board);
    }
}

void ScoutEngine::stop() {
    bool wasSearching = searching;

    searching = false;

    if(timerThread.joinable())
        timerThread.join();

    if(searchThread.joinable())
        searchThread.join();

    if(wasSearching)
        evaluator.setBoard(*board);
}

inline int32_t ScoutEngine::scoreMove(Move& move, int16_t ply) {
    int32_t moveScore = 0;
    
    if(!move.isQuiet()) {
        int32_t seeScore = evaluator.evaluateMoveSEE(move);
        seeCache.put(move, seeScore);
        moveScore += seeScore + DEFAULT_CAPTURE_MOVE_SCORE;
    } else {
        if(killerMoves[ply][0] == move)
            moveScore += 80;
        else if(killerMoves[ply][1] == move)
            moveScore += 70;  
        else if(ply >= 2) {
            if(killerMoves[ply - 2][0] == move)
                moveScore += 60;
            else if(killerMoves[ply - 2][1] == move)
                moveScore += 50;
        }

        Move lastMove = searchBoard.getLastMove();
        if(lastMove.exists() && counterMoves[searchBoard.pieceAt(lastMove.getOrigin())]
                                            [Mailbox::mailbox[lastMove.getDestination()]] == move)
            moveScore += 40;
    }

    int32_t movedPieceType = TYPEOF(searchBoard.pieceAt(move.getOrigin()));
    int32_t side = searchBoard.getSideToMove();
    int32_t psqtOrigin64 = Mailbox::mailbox[move.getOrigin()];
    int32_t psqtDestination64 = Mailbox::mailbox[move.getDestination()];

    if(side == BLACK) {
        int32_t rank = psqtOrigin64 / 8;
        int32_t file = psqtOrigin64 % 8;

        psqtOrigin64 = (RANK_8 - rank) * 8 + file;

        rank = psqtDestination64 / 8;
        file = psqtDestination64 % 8;

        psqtDestination64 = (RANK_8 - rank) * 8 + file;
    }

    moveScore += MOVE_ORDERING_PSQT[movedPieceType][psqtDestination64] - MOVE_ORDERING_PSQT[movedPieceType][psqtOrigin64];

    moveScore += std::clamp(relativeHistory[searchBoard.getSideToMove() / COLOR_MASK]
                        [Mailbox::mailbox[move.getOrigin()]]
                        [Mailbox::mailbox[move.getDestination()]] / ((currentMaxDepth / ONE_PLY) + 1),
                        -99, 49);

    return moveScore;
}

void ScoutEngine::sortMovesAtRoot(Array<Move, 256>& moves) {
    Array<MoveScorePair, 256> msp;
    
    std::vector<Move> bestMovesFromLastDepth;

    for(Variation v : variations) {
        bestMovesFromLastDepth.push_back(v.moves[0]);
    }

    for(Move move : moves) {
        int32_t moveScore = 0;

        size_t index = std::find(bestMovesFromLastDepth.begin(), bestMovesFromLastDepth.end(), move) - bestMovesFromLastDepth.begin();
        if(index < bestMovesFromLastDepth.size())
            moveScore += 30000 - index;
        else
            moveScore += scoreMove(move, 0);

        moveScoreCache.put(move, moveScore);
        msp.push_back({move, moveScore});
    }

    std::sort(msp.begin(), msp.end(), std::greater<MoveScorePair>());

    moves.clear();

    for(MoveScorePair pair : msp)
        moves.push_back(pair.move);
}

void ScoutEngine::sortMoves(Array<Move, 256>& moves, int16_t ply) {
    sortAndCutMoves(moves, ply, MIN_SCORE);
}

void ScoutEngine::sortAndCutMovesForQuiescence(Array<Move, 256>& moves, int32_t minScore, int32_t moveEvalFunc) {
    Array<MoveScorePair, 256> msp;

    for(Move move : moves) {
        int32_t moveScore = 0;

        switch(moveEvalFunc) {
            case MVVLVA:
                moveScore += evaluator.evaluateMoveMVVLVA(move) + DEFAULT_CAPTURE_MOVE_SCORE;
                break;
            case SEE:
                int32_t seeScore = evaluator.evaluateMoveSEE(move) + DEFAULT_CAPTURE_MOVE_SCORE;
                seeCache.put(move, seeScore);
                moveScore += seeScore;
                break;
        }

        if(moveScore >= minScore) {
            moveScoreCache.put(move, moveScore);
            msp.push_back({move, moveScore});
        }
    }

    std::sort(msp.begin(), msp.end(), std::greater<MoveScorePair>());

    moves.clear();

    for(MoveScorePair msPair : msp)
        moves.push_back(msPair.move);
}

void ScoutEngine::sortAndCutMoves(Array<Move, 256>& moves, int16_t ply, int32_t minScore) {
    Array<MoveScorePair, 256> msp;

    TranspositionTableEntry ttEntry;
    transpositionTable.probe(searchBoard.getHashValue(), ttEntry);

    for(Move move : moves) {
        int32_t moveScore = 0;

        if(move == ttEntry.hashMove)
            moveScore += 30000;
        else
            moveScore += scoreMove(move, ply);

        if(moveScore >= minScore) {
            moveScoreCache.put(move, moveScore);
            msp.push_back({move, moveScore});
        }
    }

    std::sort(msp.begin(), msp.end(), std::greater<MoveScorePair>());

    moves.clear();

    for(MoveScorePair pair : msp)
        moves.push_back(pair.move);
}

int16_t ScoutEngine::rootSearch(int16_t depth, int16_t expectedScore) {
    int32_t aspAlphaReduction = ASP_WINDOW, numAlphaReduction = 1;
    int32_t aspBetaReduction = ASP_WINDOW, numBetaReduction = 1;

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
            if(numAlphaReduction >= ASP_MAX_DEPTH)
                alpha = MIN_SCORE;
            else {
                aspAlphaReduction *= ASP_STEP_FACTOR;
                alpha = lowerExpectedScore - aspAlphaReduction;
            }

            numAlphaReduction++;
        } else if(score >= beta) {
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

int16_t ScoutEngine::pvSearchRoot(int16_t depth, int16_t alpha, int16_t beta) {
    if(!searching)
        return 0;

    clearPvTable();
    moveScoreCache.clear();
    seeCache.clear();

    int32_t pvNodes = numVariations;
    int16_t score, bestScore = MIN_SCORE, worstVariationScore = MIN_SCORE;
    Move bestMove;
    int16_t oldAlpha = alpha;

    std::vector<Variation> newVariations;

    int32_t moveNumber = 1;
    bool isCheckEvasion = searchBoard.isCheck();

    Array<Move, 256> moves = searchBoard.generateLegalMoves();
    sortMovesAtRoot(moves);

    for(Move move : moves) {
        nodesSearched++;

        int16_t matePly = MAX_PLY;
        bool isMateVariation = false;

        for(Variation variation : variations) {
            if(variation.moves.size() > 0 && variation.moves[0] == move &&
                IS_MATE_SCORE(variation.score)) {
                matePly = MATE_SCORE - std::abs(variation.score);
                isMateVariation = true;
                break;
            }
        }

        if(variations.size() > 0 && !isMateVariation) {
            int16_t bestVariationScore = variations.front().score;

            if(bestVariationScore < 0 && IS_MATE_SCORE(bestVariationScore))
                matePly = MATE_SCORE - std::abs(bestVariationScore);
            else if(worstVariationScore > 0 && newVariations.size() >= numVariations && IS_MATE_SCORE(worstVariationScore))
                matePly = MATE_SCORE - std::abs(worstVariationScore);
        }

        mateDistance = matePly;

        searchBoard.makeMove(move);

        int16_t extension = determineExtension(false, false, isCheckEvasion);
        int16_t nwReduction = determineReduction(depth, 0, moveNumber, isCheckEvasion);

        int16_t nwDepthDelta = -ONE_PLY - nwReduction + extension;

        int16_t minReduction = -ONE_PLY;

        nwDepthDelta = std::min(nwDepthDelta, minReduction);

        if(pvNodes > 0) {
            score = -pvSearch(depth - ONE_PLY, 1, -beta, -alpha, 1);
        } else {
            score = -nwSearch(depth + nwDepthDelta, 1, -alpha - 1, -alpha, 1);
            if(score > worstVariationScore)
                score = -pvSearch(depth - ONE_PLY, 1, -beta, -alpha, 1);
        }

        searchBoard.undoMove();

        if(!searching)
            return 0;
        
        relativeHistory[searchBoard.getSideToMove() / COLOR_MASK]
                           [Mailbox::mailbox[move.getOrigin()]]
                           [Mailbox::mailbox[move.getDestination()]] -= depth / ONE_PLY;
        

        if(score >= beta) {
            transpositionTable.put(searchBoard.getHashValue(), {
                currentAge, depth, score, CUT_NODE, move
            });

            if(move.isQuiet()) {
                if(killerMoves[0][0] != move) {
                    killerMoves[0][1] = killerMoves[0][0];
                    killerMoves[0][0] = move;
                }

                Move lastMove = searchBoard.getLastMove();
                if(lastMove.exists())
                    counterMoves[searchBoard.pieceAt(lastMove.getDestination())]
                                [Mailbox::mailbox[lastMove.getOrigin()]] = move;
            }

            relativeHistory[searchBoard.getSideToMove() / COLOR_MASK]
                           [Mailbox::mailbox[move.getOrigin()]]
                           [Mailbox::mailbox[move.getDestination()]] += (currentMaxDepth / ONE_PLY) * (currentMaxDepth / ONE_PLY);

            return score;
        }

        if(score > bestScore) {
            bestScore = score;
            bestMove = move;
        }

        if(score > worstVariationScore) {
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
                worstVariationScore = newVariations.back().score;

                if(worstVariationScore > oldAlpha)
                    alpha = worstVariationScore;
            }
        }

        pvNodes--;
        moveNumber++;
    }

    transpositionTable.put(searchBoard.getHashValue(), {
        currentAge, depth, bestScore, EXACT_NODE, bestMove
    });

    relativeHistory[searchBoard.getSideToMove() / COLOR_MASK]
                    [Mailbox::mailbox[bestMove.getOrigin()]]
                    [Mailbox::mailbox[bestMove.getDestination()]] += (currentMaxDepth / ONE_PLY) * (currentMaxDepth / ONE_PLY);
                
    if(worstVariationScore > oldAlpha) {
        setVariations(newVariations);
    }

    return worstVariationScore;
}

int16_t ScoutEngine::pvSearch(int16_t depth, int16_t ply, int16_t alpha, int16_t beta, int32_t nullMoveCooldown) {
    if(!searching)
        return 0;

    if(evaluator.isDraw()) {
        pvTable[ply].clear();
        return 0;
    }

    if(mateDistance < ply) {
        pvTable[ply].clear();
        return MIN_SCORE + 1;
    }

    if(depth <= 0)
        return quiescence(alpha, beta);

    pvTable[ply + 1].clear();

    TranspositionTableEntry ttEntry;
    
    bool searchPv = true;
    int16_t score, bestScore = MIN_SCORE;
    Move bestMove;
    
    Array<Move, 256> moves = searchBoard.generateLegalMoves();

    if(moves.size() == 0) {
        pvTable[ply].clear();

        if(searchBoard.isCheck())
            return -MATE_SCORE + ply;
        else
            return 0;
    }

    int32_t moveNumber = 1;
    bool isCheckEvasion = searchBoard.isCheck();

    sortMoves(moves, ply);

    for(Move move : moves) {
        nodesSearched++;

        searchBoard.makeMove(move);

        int16_t extension = determineExtension(false, false, isCheckEvasion);
        int16_t nwReduction = determineReduction(depth, ply, moveNumber, isCheckEvasion);

        int16_t nwDepthDelta = -ONE_PLY - nwReduction + extension;

        int16_t minReduction = -ONE_PLY;

        nwDepthDelta = std::min(nwDepthDelta, minReduction);

        if(searchPv) {
            score = -pvSearch(depth - ONE_PLY, ply + 1, -beta, -alpha, nullMoveCooldown - 1);
        } else {
            score = -nwSearch(depth + nwDepthDelta, ply + 1, -alpha - 1, -alpha, nullMoveCooldown - 1);
            if(score > alpha)
                score = -pvSearch(depth - ONE_PLY, ply + 1, -beta, -alpha, nullMoveCooldown - 1);
        }

        searchBoard.undoMove();

        if(!searching)
            return 0;
        
        relativeHistory[searchBoard.getSideToMove() / COLOR_MASK]
                           [Mailbox::mailbox[move.getOrigin()]]
                           [Mailbox::mailbox[move.getDestination()]] -= depth / ONE_PLY;
        

        if(score >= beta) {
            bool tableHit = transpositionTable.probe(searchBoard.getHashValue(), ttEntry);

            if((!tableHit || (depth > ttEntry.depth)) &&
                             (ttEntry.type != (PV_NODE | EXACT_NODE)))
                transpositionTable.put(searchBoard.getHashValue(), {
                    currentAge, depth, score, CUT_NODE, move
                });

            if(move.isQuiet()) {
                if(killerMoves[ply][0] != move) {
                    killerMoves[ply][1] = killerMoves[ply][0];
                    killerMoves[ply][0] = move;
                }

                Move lastMove = searchBoard.getLastMove();
                if(lastMove.exists())
                    counterMoves[searchBoard.pieceAt(lastMove.getDestination())]
                                [Mailbox::mailbox[lastMove.getOrigin()]] = move;
            }

            relativeHistory[searchBoard.getSideToMove() / COLOR_MASK]
                           [Mailbox::mailbox[move.getOrigin()]]
                           [Mailbox::mailbox[move.getDestination()]] += (currentMaxDepth / ONE_PLY) * (currentMaxDepth / ONE_PLY);

            return score;
        }

        if(score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
        
        if(score > alpha) {
            alpha = score;

            if(ply < 63) {
                pvTable[ply].clear();
                pvTable[ply].push_back(move);
                pvTable[ply].push_back(pvTable[ply + 1]);
            }
        }

        searchPv = false;
        moveNumber++;
    }

    bool tableHit = transpositionTable.probe(searchBoard.getHashValue(), ttEntry);

    if(!tableHit || (depth > ttEntry.depth))
        transpositionTable.put(searchBoard.getHashValue(), {
            currentAge, depth, bestScore, EXACT_NODE, bestMove
        });

    relativeHistory[searchBoard.getSideToMove() / COLOR_MASK]
                    [Mailbox::mailbox[bestMove.getOrigin()]]
                    [Mailbox::mailbox[bestMove.getDestination()]] += (currentMaxDepth / ONE_PLY) * (currentMaxDepth / ONE_PLY);

    return bestScore;
}

int16_t ScoutEngine::nwSearch(int16_t depth, int16_t ply, int16_t alpha, int16_t beta, int32_t nullMoveCooldown) {
    if(!searching)
        return 0;

    if(evaluator.isDraw())
        return 0;

    if(mateDistance < ply)
        return MIN_SCORE + 1;

    if(depth <= 0)
        return quiescence(alpha, beta);

    TranspositionTableEntry ttEntry;
    bool tableHit = transpositionTable.probe(searchBoard.getHashValue(), ttEntry);

    if(tableHit) {
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
    bool isThreat = false, isMateThreat = false;

    // Null move pruning
    if(nullMoveCooldown <= 0 && !isCheckEvasion && !isMateLine()) {
        int32_t side = searchBoard.getSideToMove();
        Bitboard minorOrMajorPieces;

        if(side == WHITE)
            minorOrMajorPieces = searchBoard.getWhiteOccupiedBitboard() & ~searchBoard.getPieceBitboard(WHITE | PAWN);
        else
            minorOrMajorPieces = searchBoard.getBlackOccupiedBitboard() & ~searchBoard.getPieceBitboard(BLACK | PAWN);

        // Nullzug nur durchführen, wenn wir mindestens eine Leichtfigur haben
        if(minorOrMajorPieces) {
            Move nullMove = Move::nullMove();
            searchBoard.makeMove(nullMove);

            int16_t depthReduction = 3 * ONE_PLY;

            if(depth >= 8 * ONE_PLY)
                depthReduction += ONE_PLY;

            int16_t nullMoveScore = -nwSearch(depth - depthReduction, ply + 1, -beta, -alpha, NULL_MOVE_R_VALUE);

            searchBoard.undoMove();

            if(nullMoveScore >= beta) {
                return nullMoveScore;
            }

            if(nullMoveScore < (alpha - THREAT_MARGIN))
                isThreat = true;

            if(IS_MATE_SCORE(nullMoveScore))
                isMateThreat = true;
        }
    }
    
    Array<Move, 256> moves = searchBoard.generateLegalMoves();

    if(moves.size() == 0) {
        if(searchBoard.isCheck())
            return -MATE_SCORE + ply;
        else
            return 0;
    }

    sortMoves(moves, ply);

    int16_t bestScore = MIN_SCORE;
    Move bestMove;

    int32_t moveNumber = 1;

    for(Move move : moves) {
        nodesSearched++;

        searchBoard.makeMove(move);

        int16_t extension = determineExtension(isThreat, isMateThreat, isCheckEvasion);
        int16_t nwReduction = determineReduction(depth, ply, moveNumber, isCheckEvasion);

        int16_t nwDepthDelta = -ONE_PLY - nwReduction + extension;
        int16_t minReduction = -ONE_PLY;

        nwDepthDelta = std::min(nwDepthDelta, minReduction);

        int16_t score = -nwSearch(depth + nwDepthDelta, ply + 1, -beta, -alpha, nullMoveCooldown - 1);

        // Wenn eine reduzierte Suche eine Bewertung > alpha liefert, dann
        // wird eine nicht reduzierte Suche durchgeführt.
        if(nwDepthDelta < -ONE_PLY && score > alpha)
            score = -nwSearch(depth - ONE_PLY, ply + 1, -beta, -alpha, nullMoveCooldown - 1);

        searchBoard.undoMove();

        if(!searching)
            return 0;
        
        relativeHistory[searchBoard.getSideToMove() / COLOR_MASK]
                        [Mailbox::mailbox[move.getOrigin()]]
                        [Mailbox::mailbox[move.getDestination()]] -= depth / ONE_PLY;

        if(score >= beta) {
            bool tableHit = transpositionTable.probe(searchBoard.getHashValue(), ttEntry);

            if(!tableHit || (depth > ttEntry.depth &&
                               ttEntry.type == (NW_NODE | CUT_NODE)))
                transpositionTable.put(searchBoard.getHashValue(), {
                    currentAge, depth, score, NW_NODE | CUT_NODE, move
                });
            
            if(move.isQuiet()) {
                if(killerMoves[ply][0] != move) {
                    killerMoves[ply][1] = killerMoves[ply][0];
                    killerMoves[ply][0] = move;
                }

                Move lastMove = searchBoard.getLastMove();
                if(lastMove.exists())
                    counterMoves[searchBoard.pieceAt(lastMove.getDestination())]
                                [Mailbox::mailbox[lastMove.getOrigin()]] = move;
            }

            relativeHistory[searchBoard.getSideToMove() / COLOR_MASK]
                           [Mailbox::mailbox[move.getOrigin()]]
                           [Mailbox::mailbox[move.getDestination()]] += (currentMaxDepth / ONE_PLY) * (currentMaxDepth / ONE_PLY);

            return score;
        }
        
        if(score > bestScore) {
            bestScore = score;
            bestMove = move;
        }

        moveNumber++;
    }

    if(!tableHit || (depth > ttEntry.depth &&
                        !IS_REGULAR_NODE(ttEntry.type)))
        transpositionTable.put(searchBoard.getHashValue(), {
            currentAge, depth, bestScore, NW_NODE | EXACT_NODE, bestMove
        });
    
    relativeHistory[searchBoard.getSideToMove() / COLOR_MASK]
                [Mailbox::mailbox[bestMove.getOrigin()]]
                [Mailbox::mailbox[bestMove.getDestination()]] += (currentMaxDepth / ONE_PLY) * (currentMaxDepth / ONE_PLY);

    return bestScore;
}

inline int16_t ScoutEngine::determineExtension(bool isThreat, bool isMateThreat, bool isCheckEvasion) {
    int16_t extension = 0;

    Move m = searchBoard.getLastMove();

    int32_t movedPieceType = TYPEOF(searchBoard.pieceAt(m.getDestination()));

    bool isCheck = searchBoard.isCheck();

    // Schach
    if(isCheckEvasion || isCheck)
        extension += ONE_PLY * 3;
    
    // Schlagzug oder Bauernumwandlung
    if(!m.isQuiet()) {
        int32_t seeScore = MIN_SCORE;
        bool seeCacheHit = seeCache.probe(m, seeScore);

        if(!seeCacheHit || seeScore >= NEUTRAL_SEE_SCORE)
            extension += ONE_PLY * 2; // Erweiterung wenn der Schlagzug eine gute/neutrale SEE-Bewertung hat
        else
            extension += TWO_THIRDS_PLY; // Geringere Erweiterung wenn der Schlagzug eine schlechte SEE-Bewertung hat
    }

    // Freibauerzüge
    if(movedPieceType == PAWN) {
        int32_t side = searchBoard.getSideToMove() ^ COLOR_MASK;
        int32_t otherSide = searchBoard.getSideToMove();

        if(!(sentryMasks[side / COLOR_MASK][Mailbox::mailbox[m.getDestination()]]
            & searchBoard.getPieceBitboard(otherSide | PAWN)))
            extension += TWO_THIRDS_PLY;
    }

    // Wenn Gefahr/Mattgefahr besteht
    if(isThreat) {
        extension += ONE_PLY;
    }

    if(isMateThreat) {
        extension += ONE_PLY * 2;
    }
    
    return extension;
}

inline int16_t ScoutEngine::determineReduction(int16_t depth, int16_t ply, int32_t moveCount, bool isCheckEvasion) {
    int16_t reduction = 0;

    bool isCheck = searchBoard.isCheck();

    int32_t unreducedMoves = 1;
    
    if(moveCount <= unreducedMoves || isCheck || isCheckEvasion)
        return 0;

    reduction += (int16_t)(sqrt(depth) + sqrt(std::max(moveCount - unreducedMoves, 0) * ONE_PLY));

    if(isMateLine()) {
        int16_t maxSearchPly = (int16_t)(currentMaxDepth / ONE_PLY);

        if(maxSearchPly > mateDistance + ply) {
            reduction -= (maxSearchPly - mateDistance - ply) * ONE_PLY;
            reduction = std::max(reduction, (int16_t)0);
        }
    }
    
    return reduction;
}

int16_t ScoutEngine::quiescence(int16_t alpha, int16_t beta) {
    if(!searching)
        return 0;

    int16_t staticEvaluationScore = (int16_t)evaluator.evaluate();

    int16_t score = staticEvaluationScore;

    if(score >= beta)
        return score;
    
    if(score > alpha)
        alpha = score;
    
    int16_t bestScore = score;
    
    Array<Move, 256> moves;

    if(searchBoard.isCheck()) {
        moves = searchBoard.generateLegalMoves();
        if(moves.size() == 0)
            return -MATE_SCORE + MAX_PLY;

        sortAndCutMovesForQuiescence(moves, MIN_SCORE, MVVLVA);
    }
    else {
        moves = searchBoard.generateLegalCaptures();

        sortAndCutMovesForQuiescence(moves, NEUTRAL_SEE_SCORE, SEE);
    }
    
    for(Move move : moves) {
        nodesSearched++;

        searchBoard.makeMove(move);

        score = -quiescence(-beta, -alpha);

        searchBoard.undoMove();

        if(!searching)
            return 0;

        if(score >= beta)
            return score;
        
        if(score > bestScore)
            bestScore = score;
        
        if(score > alpha)
            alpha = score;
    }

    return bestScore;
}