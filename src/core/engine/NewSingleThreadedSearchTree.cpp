#include "core/engine/NewSingleThreadedSearchTree.h"
#include <thread>
#include <algorithm>
#include <cmath>
#include "core/utils/MoveNotations.h"
#include "core/chess/MailboxDefinitions.h"

void NewSingleThreadedSearchTree::searchTimer(uint32_t searchTime) {
    auto start = std::chrono::system_clock::now();

    while(searching) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        auto end = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        if(elapsed >= searchTime && searchTime > 0)
            searching = false;
    }
}

void NewSingleThreadedSearchTree::clearRelativeHistory() {
    for(int i = 0; i < 2; i++) {
        for(int j = 0; j < 64; j++) {
            for(int k = 0; k < 64; k++) {
                relativeHistory[i][j][k] = 0;
            }
        }
    }
}

void NewSingleThreadedSearchTree::clearPvTable() {
    for(int i = 0; i < 64; i++) {
        pvTable[i].clear();
    }
}

void NewSingleThreadedSearchTree::clearKillerMoves() {
    for(int i = 0; i < 256; i++) {
        killerMoves[i][0] = Move();
        killerMoves[i][1] = Move();
    }
}

void NewSingleThreadedSearchTree::shiftKillerMoves() {
    for(int i = 1; i < 256; i++) {
        killerMoves[i - 1][0] = killerMoves[i][0];
        killerMoves[i - 1][1] = killerMoves[i][1];
    }
}

void NewSingleThreadedSearchTree::runSearch() {
    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    int16_t score = evaluator.evaluate();

    for(int16_t depth = ONE_PLY; searching; depth += ONE_PLY) {
        currentMaxDepth = depth;

        score = rootSearch(depth, score);

        auto endMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        if(searching) {
            std::cout << "(" << endMs - nowMs << "ms)" << "Depth: " << depth / ONE_PLY << " Nodes: " << nodesSearched << std::endl;

            for(Variation v : variations) {
                std::cout << v.score << " -";
                for(std::string s : variationToFigurineAlgebraicNotation(v.moves, searchBoard)) {
                    std::cout << " " << s;
                }
                std::cout << std::endl;
            }
        }
    }
}

void NewSingleThreadedSearchTree::search(uint32_t searchTime, bool dontBlock) {
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

    if(searchTime > 0)
        timerThread = std::thread(std::bind(&NewSingleThreadedSearchTree::searchTimer, this, searchTime));

    searchThread = std::thread(std::bind(&NewSingleThreadedSearchTree::runSearch, this));

    if(!dontBlock) {
        if(timerThread.joinable())
            timerThread.join();

        if(searchThread.joinable())
            searchThread.join();

        evaluator.setBoard(*board);
    }
}

void NewSingleThreadedSearchTree::stop() {
    searching = false;

    if(timerThread.joinable())
        timerThread.join();

    if(searchThread.joinable())
        searchThread.join();

    evaluator.setBoard(*board);
}

inline int32_t NewSingleThreadedSearchTree::scoreMove(Move& move, int16_t ply) {
    int32_t moveScore = 0;
    
    if(!move.isQuiet()) {
        int32_t seeScore = evaluator.evaluateMoveSEE(move);
        seeCache.put(move, seeScore);
        moveScore += seeScore + DEFAULT_CAPTURE_MOVE_SCORE;
    } else {
        if(killerMoves[ply][0] == move)
            moveScore += DEFAULT_CAPTURE_MOVE_SCORE + 80;
        else if(killerMoves[ply][1] == move)
            moveScore += DEFAULT_CAPTURE_MOVE_SCORE + 70;  
        else if(ply >= 2) {
            if(killerMoves[ply - 2][0] == move)
                moveScore += DEFAULT_CAPTURE_MOVE_SCORE + 60;
            else if(killerMoves[ply - 2][1] == move)
                moveScore += DEFAULT_CAPTURE_MOVE_SCORE + 50;
        }
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

    moveScore += relativeHistory[searchBoard.getSideToMove() / COLOR_MASK]
                        [Mailbox::mailbox[move.getOrigin()]]
                        [Mailbox::mailbox[move.getDestination()]] / ((currentMaxDepth / ONE_PLY) * (currentMaxDepth / ONE_PLY));

    return moveScore;
}

void NewSingleThreadedSearchTree::sortMovesAtRoot(Array<Move, 256>& moves) {
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

void NewSingleThreadedSearchTree::sortMoves(Array<Move, 256>& moves, int16_t ply) {
    sortAndCutMoves(moves, ply, MIN_SCORE);
}

void NewSingleThreadedSearchTree::sortAndCutMovesForQuiescence(Array<Move, 256>& moves, int32_t minScore, int32_t moveEvalFunc) {
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

void NewSingleThreadedSearchTree::sortAndCutMoves(Array<Move, 256>& moves, int16_t ply, int32_t minScore) {
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

int16_t NewSingleThreadedSearchTree::rootSearch(int16_t depth, int16_t expectedScore) {
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

int16_t NewSingleThreadedSearchTree::pvSearchRoot(int16_t depth, int16_t alpha, int16_t beta) {
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

        if(!isMateVariation && (worstVariationScore != MIN_SCORE && IS_MATE_SCORE(worstVariationScore)))
            matePly = MATE_SCORE - std::abs(worstVariationScore);

        mateDistance = matePly;

        searchBoard.makeMove(move);

        int16_t extension = determineExtension(false, false, isCheckEvasion);
        int16_t nwReduction = determineReduction(depth, moveNumber, isCheckEvasion);

        int16_t nwDepthDelta = -ONE_PLY - nwReduction + extension;
        
        int16_t minReduction = -ONE_PLY;

        nwDepthDelta = std::min(nwDepthDelta, minReduction);

        if(pvNodes > 0) {
            score = -pvSearch(depth - ONE_PLY, 1, -beta, -alpha);
        } else {
            score = -nwSearch(depth + nwDepthDelta, 1, -alpha - 1, -alpha);
            if(score > worstVariationScore)
                score = -pvSearch(depth - ONE_PLY, 1, -beta, -alpha);
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
            }

            relativeHistory[searchBoard.getSideToMove() / COLOR_MASK]
                           [Mailbox::mailbox[move.getOrigin()]]
                           [Mailbox::mailbox[move.getDestination()]] += (depth / ONE_PLY) * (depth / ONE_PLY);

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
                    [Mailbox::mailbox[bestMove.getDestination()]] += (depth / ONE_PLY) * (depth / ONE_PLY);
                
    if(worstVariationScore > oldAlpha) {
        setVariations(newVariations);
    }

    return worstVariationScore;
}

int16_t NewSingleThreadedSearchTree::pvSearch(int16_t depth, int16_t ply, int16_t alpha, int16_t beta, bool nullMoveAllowed) {
    if(!searching)
        return 0;

    if(evaluator.isDraw() || mateDistance < ply) {
        pvTable[ply].clear();
        return 0;
    }

    if(depth <= 0) {
        return quiescence(alpha, beta);
    }

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
    bool isThreat = false, isMateThreat = false;

    // Null move pruning
    if(nullMoveAllowed && !isCheckEvasion && (depth >= 4 * ONE_PLY)) {
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

            int16_t nullMoveScore = -nwSearch(depth - depthReduction, ply + 1, -beta, -alpha, false);

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

    sortMoves(moves, ply);

    for(Move move : moves) {
        nodesSearched++;

        searchBoard.makeMove(move);

        int16_t extension = determineExtension(isThreat, isMateThreat, isCheckEvasion);
        int16_t nwReduction = determineReduction(depth, moveNumber, isCheckEvasion);

        int16_t pvExtension = extension / 4;
        int16_t nwDepthDelta = -ONE_PLY - nwReduction + extension;

        int16_t maxExtension = 0;
        int16_t minReduction = -ONE_PLY;

        if(isThreat) {
            maxExtension = ONE_HALF_PLY;

            if(isMateThreat) {
                maxExtension = TWO_THIRDS_PLY;
            }
        }

        if(searchBoard.isCheck() || isCheckEvasion) {
            maxExtension = ONE_HALF_PLY;
        }

        pvExtension = std::min(pvExtension, maxExtension);
        nwDepthDelta = std::min(nwDepthDelta, minReduction);

        if(searchPv) {
            score = -pvSearch(depth + pvExtension - ONE_PLY, ply + 1, -beta, -alpha);
        } else {
            score = -nwSearch(depth + nwDepthDelta, ply + 1, -alpha - 1, -alpha);
            if(score > alpha)
                score = -pvSearch(depth + pvExtension - ONE_PLY, ply + 1, -beta, -alpha);
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
            }

            relativeHistory[searchBoard.getSideToMove() / COLOR_MASK]
                           [Mailbox::mailbox[move.getOrigin()]]
                           [Mailbox::mailbox[move.getDestination()]] += (depth / ONE_PLY) * (depth / ONE_PLY);

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
                    [Mailbox::mailbox[bestMove.getDestination()]] += (depth / ONE_PLY) * (depth / ONE_PLY);

    return bestScore;
}

int16_t NewSingleThreadedSearchTree::nwSearch(int16_t depth, int16_t ply, int16_t alpha, int16_t beta, bool nullMoveAllowed) {
    if(!searching)
        return 0;

    if(evaluator.isDraw() || mateDistance < ply)
        return 0;

    if(depth <= 0) {
        return quiescence(alpha, beta);
    }

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
    if(nullMoveAllowed && !isCheckEvasion && (depth >= 4 * ONE_PLY)) {
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

            int16_t nullMoveScore = -nwSearch(depth - depthReduction, ply + 1, -beta, -alpha, false);

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
        int16_t nwReduction = determineReduction(depth, moveNumber, isCheckEvasion);

        int16_t nwDepthDelta = -ONE_PLY - nwReduction + extension;
        int16_t minReduction = -ONE_PLY;

        nwDepthDelta = std::min(nwDepthDelta, minReduction);

        int16_t score = -nwSearch(depth + nwDepthDelta, ply + 1, -beta, -alpha);

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
            }

            relativeHistory[searchBoard.getSideToMove() / COLOR_MASK]
                           [Mailbox::mailbox[move.getOrigin()]]
                           [Mailbox::mailbox[move.getDestination()]] += (depth / ONE_PLY) * (depth / ONE_PLY);

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
                [Mailbox::mailbox[bestMove.getDestination()]] += (depth / ONE_PLY) * (depth / ONE_PLY);

    return bestScore;
}

inline int16_t NewSingleThreadedSearchTree::determineExtension(bool isThreat, bool isMateThreat, bool isCheckEvasion) {
    int16_t extension = 0;

    Move m = searchBoard.getLastMove();

    int32_t movedPieceType = TYPEOF(searchBoard.pieceAt(m.getDestination()));

    bool isCheck = searchBoard.isCheck();

    // Schach
    if(isCheckEvasion || isCheck)
        extension += 2 * ONE_PLY;
    
    // Schlagzug oder Bauernumwandlung
    if(!m.isQuiet()) {
        int32_t seeScore = MIN_SCORE;
        bool seeCacheHit = seeCache.probe(m, seeScore);

        if(!seeCacheHit || seeScore >= NEUTRAL_SEE_SCORE)
            extension += ONE_PLY; // Erweiterung wenn der Schlagzug eine gute/neutrale SEE-Bewertung hat
        else
            extension += ONE_HALF_PLY; // Geringere Erweiterung wenn der Schlagzug eine schlechte SEE-Bewertung hat
    } else {
        // History-Erweiterungen
        int historyExtension = (int16_t)(relativeHistory[(searchBoard.getSideToMove() ^ COLOR_MASK) / COLOR_MASK]
                            [Mailbox::mailbox[searchBoard.getLastMove().getOrigin()]]
                            [Mailbox::mailbox[searchBoard.getLastMove().getDestination()]] / ((currentMaxDepth / ONE_PLY) * (currentMaxDepth / ONE_PLY)));
                        
        historyExtension = std::max(historyExtension, 0);

        extension += historyExtension;
    }

    // Freibauerzüge
    if(movedPieceType == PAWN) {
        int32_t side = searchBoard.getSideToMove() ^ COLOR_MASK;
        int32_t otherSide = searchBoard.getSideToMove();

        if(!(sentryMasks[side / COLOR_MASK][Mailbox::mailbox[m.getDestination()]]
            & searchBoard.getPieceBitboard(otherSide | PAWN)))
            extension += TWO_THIRDS_PLY;
    }

    // Wenn Gefahr besteht
    if(isThreat)
        extension += ONE_PLY;
    
    // Wenn Mattgefahr besteht
    if(isMateThreat)
        extension += 4 * ONE_PLY;
    
    return extension;
}

inline int16_t NewSingleThreadedSearchTree::determineReduction(int16_t depth, int32_t moveCount, bool isCheckEvasion) {
    int16_t reduction = 0;

    bool isCheck = searchBoard.isCheck();

    int32_t unreducedMoves = 2;

    if(depth <= 3 * ONE_PLY || isCheckEvasion)
        unreducedMoves = 6;
    
    if(moveCount <= unreducedMoves)
        return 0;

    reduction += (int16_t)((sqrt(depth / ONE_PLY) + sqrt(moveCount - unreducedMoves)) * ONE_PLY);

    // History-Reduktionen
    int16_t historyReduction = (int16_t)(-relativeHistory[(searchBoard.getSideToMove() ^ COLOR_MASK) / COLOR_MASK]
                            [Mailbox::mailbox[searchBoard.getLastMove().getOrigin()]]
                            [Mailbox::mailbox[searchBoard.getLastMove().getDestination()]] / ((currentMaxDepth / ONE_PLY) * (currentMaxDepth / ONE_PLY)));

    historyReduction = std::max(historyReduction, (int16_t)0);

    reduction += historyReduction;

    if(isCheck)
        reduction /= 2;
    
    return reduction;
}

int16_t NewSingleThreadedSearchTree::quiescence(int16_t alpha, int16_t beta) {
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
            return -MATE_SCORE;

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