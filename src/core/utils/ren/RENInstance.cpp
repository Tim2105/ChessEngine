#include "core/utils/ren/RENInstance.h"
#include "core/utils/nnue/NNUEUtils.h"

#include <algorithm>

using namespace REN;

Instance::Instance(const Network& net) noexcept : network(net), accumulator(network) {
    pastAccumulators.reserve(128);
}

Instance::~Instance() noexcept {}

int Instance::evaluate(int color, size_t iter) const noexcept {
    alignas(REQUIRED_ALIGNMENT) int16_t halfKAv2_hmOutput[Network::REN_SIZE];
    alignas(REQUIRED_ALIGNMENT) int32_t dynamicBias[Network::REN_SIZE];
    alignas(REQUIRED_ALIGNMENT) int8_t buf0[Network::REN_SIZE];
    alignas(REQUIRED_ALIGNMENT) int8_t buf1[Network::REN_SIZE];
    int8_t* renInput = buf0;
    int8_t* renOutput = buf1;
    alignas(REQUIRED_ALIGNMENT) int32_t output[1];

    // Baue die Eingabe für die erste Schicht aus den Akkumulatoren auf
    const int16_t* acc = accumulator.getOutput(color);
    const int16_t* accOther = accumulator.getOutput(color ^ COLOR_MASK);

    std::copy(acc, acc + Network::REN_SIZE / 2, halfKAv2_hmOutput);
    std::copy(accOther, accOther + Network::REN_SIZE / 2, halfKAv2_hmOutput + Network::REN_SIZE / 2);

    if(iter == 0) {
        network.getOutputLayer().forward(halfKAv2_hmOutput, output);
    } else {
        network.getRENLayer().createDynamicBias(halfKAv2_hmOutput, network.getRENLayer().getBiasPtr(), dynamicBias);
        network.getRENLayer().forward(halfKAv2_hmOutput, dynamicBias, renInput);

        for(size_t i = 1; i < iter; i++) {
            network.getRENLayer().forward(renInput, dynamicBias, renOutput);
            std::swap(renInput, renOutput);
        }

        network.getOutputLayer().forward(renInput, output);
    }

    return (int)((int64_t)output[0] * 100 / 6656);
}

void Instance::initializeFromBoard(const Board& board, int color) noexcept {
    accumulator.refresh(getHalfKPFeatures(board, color), color);
}

void Instance::updateAfterOppKingMove(const Board& board, int color, Move move) noexcept {
    int colorMoved = color ^ COLOR_MASK;

    Array<int, 8> addedFeatures;
    Array<int, 8> removedFeatures;

    // Bewege den König von seiner alten Position zu seiner neuen Position
    int ownKingSq = board.getKingSquare(color);
    int oldKingSq = move.getOrigin();
    int newKingSq = move.getDestination();

    if(color == WHITE) {
        removedFeatures.push_back(getHalfKPIndex<WHITE>(ownKingSq, oldKingSq, colorMoved | KING));
        addedFeatures.push_back(getHalfKPIndex<WHITE>(ownKingSq, newKingSq, colorMoved | KING));
    } else {
        removedFeatures.push_back(getHalfKPIndex<BLACK>(ownKingSq, oldKingSq, colorMoved | KING));
        addedFeatures.push_back(getHalfKPIndex<BLACK>(ownKingSq, newKingSq, colorMoved | KING));
    }

    // Der Gegner hat auf jeden Fall alle Rochadenrechte verloren
    int oldCastlingRights = board.getLastMoveHistoryEntry().castlingPermission;
    if(color == WHITE) {
        if(oldCastlingRights & BLACK_KINGSIDE_CASTLE)
            removedFeatures.push_back(castlingIdx + 2);
        if(oldCastlingRights & BLACK_QUEENSIDE_CASTLE)
            removedFeatures.push_back(castlingIdx + 3);
    } else {
        if(oldCastlingRights & WHITE_KINGSIDE_CASTLE)
            removedFeatures.push_back(castlingIdx + 2);
        if(oldCastlingRights & WHITE_QUEENSIDE_CASTLE)
            removedFeatures.push_back(castlingIdx + 3);
    }

    if(move.isCapture()) {
        int capturedPiece = board.getLastMoveHistoryEntry().capturedPiece;
        int otherKingSq = board.getKingSquare(colorMoved ^ COLOR_MASK);

        if(colorMoved == WHITE)
            removedFeatures.push_back(getHalfKPIndex<BLACK>(otherKingSq, move.getDestination(), capturedPiece));
        else
            removedFeatures.push_back(getHalfKPIndex<WHITE>(otherKingSq, move.getDestination(), capturedPiece));

        // Wird ein Turm geschlagen, verlieren wir eventuell Rochadenrechte
        if(TYPEOF(capturedPiece) == ROOK) {
            if(colorMoved == WHITE) {
                if(move.getDestination() == H8 && (oldCastlingRights & BLACK_KINGSIDE_CASTLE))
                    removedFeatures.push_back(castlingIdx);
                else if(move.getDestination() == A8 && (oldCastlingRights & BLACK_QUEENSIDE_CASTLE))
                    removedFeatures.push_back(castlingIdx + 1);
            } else {
                if(move.getDestination() == H1 && (oldCastlingRights & WHITE_KINGSIDE_CASTLE))
                    removedFeatures.push_back(castlingIdx);
                else if(move.getDestination() == A1 && (oldCastlingRights & WHITE_QUEENSIDE_CASTLE))
                    removedFeatures.push_back(castlingIdx + 1);
            }
        }
    } else if(move.isCastle()) {
        int kingSq = board.getKingSquare(colorMoved);
        int otherKingSq = board.getKingSquare(colorMoved ^ COLOR_MASK);
        int rookOrigin, rookDestination;

        if(move.isKingsideCastle()) {
            rookOrigin = kingSq + EAST;
            rookDestination = kingSq + WEST;
        } else {
            rookOrigin = kingSq + WEST * 2;
            rookDestination = kingSq + EAST;
        }

        if(colorMoved == WHITE) {
            removedFeatures.push_back(getHalfKPIndex<BLACK>(otherKingSq, rookOrigin, colorMoved | ROOK));
            addedFeatures.push_back(getHalfKPIndex<BLACK>(otherKingSq, rookDestination, colorMoved | ROOK));
        } else {
            removedFeatures.push_back(getHalfKPIndex<WHITE>(otherKingSq, rookOrigin, colorMoved | ROOK));
            addedFeatures.push_back(getHalfKPIndex<WHITE>(otherKingSq, rookDestination, colorMoved | ROOK));
        }
    }

    // En-Passant
    int oldEp = board.getLastMoveHistoryEntry().enPassantSquare;
    if(oldEp != NO_SQ)
        removedFeatures.push_back(getHalfKPIndexForEnPassant(oldEp, ownKingSq));

    accumulator.update(addedFeatures, removedFeatures, color);
}

void Instance::initializeFromBoard(const Board& board) noexcept {
    accumulator.refresh(getHalfKPFeatures(board, WHITE), WHITE);
    accumulator.refresh(getHalfKPFeatures(board, BLACK), BLACK);
}

void Instance::updateAfterMove(const Board& board) noexcept {
    pastAccumulators.push_back({});
    std::copy(accumulator.getOutput(WHITE), accumulator.getOutput(WHITE) + Network::REN_SIZE / 2, pastAccumulators.back().begin());
    std::copy(accumulator.getOutput(BLACK), accumulator.getOutput(BLACK) + Network::REN_SIZE / 2, pastAccumulators.back().begin() + Network::REN_SIZE / 2);

    int whiteKingSq = board.getKingSquare(WHITE);
    int blackKingSq = board.getKingSquare(BLACK);

    Move move = board.getLastMove();
    if(move.isNullMove()) {
        // Entferne En-Passant Feature, falls vorhanden
        int oldEp = board.getLastMoveHistoryEntry().enPassantSquare;
        if(oldEp != NO_SQ) {
            Array<int, 8> removedFeaturesWhite;
            Array<int, 8> removedFeaturesBlack;
            removedFeaturesWhite.push_back(getHalfKPIndexForEnPassant(oldEp, whiteKingSq));
            removedFeaturesBlack.push_back(getHalfKPIndexForEnPassant(oldEp, blackKingSq));
            accumulator.update({}, removedFeaturesWhite, WHITE);
            accumulator.update({}, removedFeaturesBlack, BLACK);
        }

        return;
    }

    int movedPiece = board.pieceAt(move.getDestination());
    int colorMoved = movedPiece & COLOR_MASK;

    if(TYPEOF(movedPiece) == KING) {
        // Wenn der König gezogen wurde,
        // muss das Netzwerk der Farbe des Königs neu initialisiert werden
        initializeFromBoard(board, colorMoved);
        updateAfterOppKingMove(board, colorMoved ^ COLOR_MASK, move);
        return;
    }

    Array<int, 8> addedFeaturesWhite;
    Array<int, 8> addedFeaturesBlack;
    Array<int, 8> removedFeaturesWhite;
    Array<int, 8> removedFeaturesBlack;

    // Wenn der Zug eine Promotion ist, wäre die gezogene Figur sonst
    // die aufgewertete Figur
    if(move.isPromotion())
        movedPiece = PAWN | colorMoved;

    // Entferne die gezogene Figur von ihrer Ausgangsposition
    removedFeaturesWhite.push_back(getHalfKPIndex<WHITE>(whiteKingSq, move.getOrigin(), movedPiece));
    removedFeaturesBlack.push_back(getHalfKPIndex<BLACK>(blackKingSq, move.getOrigin(), movedPiece));

    // Wenn die gezogene Figur eine gegnerische Figur geschlagen hat, entferne diese auch
    if(move.isCapture()) {
        // Spezialfall: En-Passant
        if(move.isEnPassant()) {
            int capturedPawnSq = move.getDestination() + (colorMoved == WHITE ? SOUTH : NORTH);
            int capturedPiece = colorMoved == WHITE ? BLACK | PAWN : WHITE | PAWN;

            removedFeaturesWhite.push_back(getHalfKPIndex<WHITE>(whiteKingSq, capturedPawnSq, capturedPiece));
            removedFeaturesBlack.push_back(getHalfKPIndex<BLACK>(blackKingSq, capturedPawnSq, capturedPiece));
        } else {
            int capturedPiece = board.getLastMoveHistoryEntry().capturedPiece;

            removedFeaturesWhite.push_back(getHalfKPIndex<WHITE>(whiteKingSq, move.getDestination(), capturedPiece));
            removedFeaturesBlack.push_back(getHalfKPIndex<BLACK>(blackKingSq, move.getDestination(), capturedPiece));

            // Spezialfall: Wenn ein Turm geschlagen wird, verlieren wir eventuell Rochadenrechte
            if(TYPEOF(capturedPiece) == ROOK) {
                int oldCastlingRights = board.getLastMoveHistoryEntry().castlingPermission;

                if(move.getDestination() == H8 && (oldCastlingRights & BLACK_KINGSIDE_CASTLE)) {
                    removedFeaturesWhite.push_back(castlingIdx + 2);
                    removedFeaturesBlack.push_back(castlingIdx);
                } else if(move.getDestination() == A8 && (oldCastlingRights & BLACK_QUEENSIDE_CASTLE)) {
                    removedFeaturesWhite.push_back(castlingIdx + 3);
                    removedFeaturesBlack.push_back(castlingIdx + 1);
                } else if(move.getDestination() == H1 && (oldCastlingRights & WHITE_KINGSIDE_CASTLE)) {
                    removedFeaturesWhite.push_back(castlingIdx);
                    removedFeaturesBlack.push_back(castlingIdx + 2);
                } else if(move.getDestination() == A1 && (oldCastlingRights & WHITE_QUEENSIDE_CASTLE)) {
                    removedFeaturesWhite.push_back(castlingIdx + 1);
                    removedFeaturesBlack.push_back(castlingIdx + 3);
                }
            }
        }
    }

    // Füge die gezogene Figur an ihre neue Position ein

    // Spezialfall: Promotion
    if(move.isPromotion()) {
        int promotedPiece = QUEEN;
        if(move.isPromotionRook())
            promotedPiece = ROOK;
        else if(move.isPromotionBishop())
            promotedPiece = BISHOP;
        else if(move.isPromotionKnight())
            promotedPiece = KNIGHT;

        promotedPiece |= colorMoved;

        addedFeaturesWhite.push_back(getHalfKPIndex<WHITE>(whiteKingSq, move.getDestination(), promotedPiece));
        addedFeaturesBlack.push_back(getHalfKPIndex<BLACK>(blackKingSq, move.getDestination(), promotedPiece));
    } else {
        addedFeaturesWhite.push_back(getHalfKPIndex<WHITE>(whiteKingSq, move.getDestination(), movedPiece));
        addedFeaturesBlack.push_back(getHalfKPIndex<BLACK>(blackKingSq, move.getDestination(), movedPiece));
    }

    // Wird ein Turm gezogen, verlieren wir eventuell Rochadenrechte
    if(TYPEOF(movedPiece) == ROOK) {
        int oldCastlingRights = board.getLastMoveHistoryEntry().castlingPermission;

        if(colorMoved == WHITE) {
            if(move.getOrigin() == H1 && (oldCastlingRights & WHITE_KINGSIDE_CASTLE)) {
                removedFeaturesWhite.push_back(castlingIdx);
                removedFeaturesBlack.push_back(castlingIdx + 2);
            } else if(move.getOrigin() == A1 && (oldCastlingRights & WHITE_QUEENSIDE_CASTLE)) {
                removedFeaturesWhite.push_back(castlingIdx + 1);
                removedFeaturesBlack.push_back(castlingIdx + 3);
            }
        } else {
            if(move.getOrigin() == H8 && (oldCastlingRights & BLACK_KINGSIDE_CASTLE)) {
                removedFeaturesBlack.push_back(castlingIdx);
                removedFeaturesWhite.push_back(castlingIdx + 2);
            } else if(move.getOrigin() == A8 && (oldCastlingRights & BLACK_QUEENSIDE_CASTLE)) {
                removedFeaturesBlack.push_back(castlingIdx + 1);
                removedFeaturesWhite.push_back(castlingIdx + 3);
            }
        }
    }

    // En-Passant
    if(move.isDoublePawn()) {
        int ep = board.getEnPassantSquare();

        addedFeaturesWhite.push_back(getHalfKPIndexForEnPassant(ep, whiteKingSq));
        addedFeaturesBlack.push_back(getHalfKPIndexForEnPassant(ep, blackKingSq));
    }

    int oldEp = board.getLastMoveHistoryEntry().enPassantSquare;
    if(oldEp != NO_SQ) {
        removedFeaturesWhite.push_back(getHalfKPIndexForEnPassant(oldEp, whiteKingSq));
        removedFeaturesBlack.push_back(getHalfKPIndexForEnPassant(oldEp, blackKingSq));
    }

    accumulator.update(addedFeaturesWhite, removedFeaturesWhite, WHITE);
    accumulator.update(addedFeaturesBlack, removedFeaturesBlack, BLACK);
}

void Instance::undoMove() noexcept {
    accumulator.setOutput(WHITE, pastAccumulators.back().begin());
    accumulator.setOutput(BLACK, pastAccumulators.back().begin() + Network::REN_SIZE / 2);

    pastAccumulators.pop_back();
}