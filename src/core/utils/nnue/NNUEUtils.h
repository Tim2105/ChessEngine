#ifndef NNUE_UTILS_H
#define NNUE_UTILS_H

#include "core/chess/BoardDefinitions.h"

#include <algorithm>
#include <limits>
#include <tuple>

namespace NNUE {
    constexpr int32_t KP_INDICES[6][2] = {
        {0, 64}, // Bauern (Wir, Die)
        {128, 192}, // Springer (Wir, Die)
        {256, 320}, // Läufer (Wir, Die)
        {384, 448}, // Türme (Wir, Die)
        {512, 576}, // Damen (Wir, Die)
        {640, 640}  // Könige (Die, Die), nur gegnerische Könige werden kodiert
    };

    template <int32_t COLOR>
    constexpr int32_t getHalfKPIndex(int32_t kingSq, int32_t pieceSq, int32_t piece) noexcept {
        int32_t pieceType = TYPEOF(piece);
        int32_t pieceColor = piece & COLOR_MASK;

        if constexpr(COLOR == BLACK) {
            kingSq = Square::flipY(kingSq);
            pieceSq = Square::flipY(pieceSq);
            pieceColor ^= COLOR_MASK;
        }

        return kingSq * 704 + KP_INDICES[pieceType - PAWN][pieceColor / COLOR_MASK] + pieceSq;
    }

    inline Array<int32_t, 63> getHalfKPFeatures(const Board& board, int32_t color) noexcept {
        Array<int32_t, 63> activeFeatures;

        int32_t kingSq = board.getKingSquare(color);

        // Weiße Figuren
        if(color == WHITE) {
            for(int32_t piece = (WHITE | PAWN); piece <= (WHITE | QUEEN); piece++) {
                Bitboard pieceBitboard = board.getPieceBitboard(piece);
                while(pieceBitboard) {
                    int32_t sq = pieceBitboard.popFSB();
                    activeFeatures.push_back(getHalfKPIndex<WHITE>(kingSq, sq, piece));
                }
            }

            // Schwarze Figuren
            for(int32_t piece = (BLACK | PAWN); piece <= (BLACK | KING); piece++) {
                Bitboard pieceBitboard = board.getPieceBitboard(piece);
                while(pieceBitboard) {
                    int32_t sq = pieceBitboard.popFSB();
                    activeFeatures.push_back(getHalfKPIndex<WHITE>(kingSq, sq, piece));
                }
            }
        } else {
            for(int32_t piece = (WHITE | PAWN); piece <= (WHITE | KING); piece++) {
                Bitboard pieceBitboard = board.getPieceBitboard(piece);
                while(pieceBitboard) {
                    int32_t sq = pieceBitboard.popFSB();
                    activeFeatures.push_back(getHalfKPIndex<BLACK>(kingSq, sq, piece));
                }
            }

            // Schwarze Figuren
            for(int32_t piece = (BLACK | PAWN); piece <= (BLACK | QUEEN); piece++) {
                Bitboard pieceBitboard = board.getPieceBitboard(piece);
                while(pieceBitboard) {
                    int32_t sq = pieceBitboard.popFSB();
                    activeFeatures.push_back(getHalfKPIndex<BLACK>(kingSq, sq, piece));
                }
            }
        }

        return activeFeatures;
    }

    constexpr int16_t convertPieceCountToNNUEInput(int pieceCount) {
        return std::min(pieceCount * 15, (int)std::numeric_limits<int8_t>::max());
    }

    constexpr int16_t convertBoolToNNUEInput(bool value) {
        return value ? 64 : 0;
    }

    constexpr int16_t convertFiftyMoveCounterToNNUEInput(int fiftyMoveCounter) {
        return std::min(fiftyMoveCounter, 100);
    }

    static constexpr Bitboard lightSquares = 0x55aa55aa55aa55aaULL;

    inline void fillAdditionalInput(const Board& board, int16_t additionalInput[NNUE::Network::INPUT_ADDITION]) {
        int32_t side = board.getSideToMove();
        int32_t otherSide = side ^ COLOR_MASK;

        // Figurenanzahl für beide Seiten.
        additionalInput[0] = convertPieceCountToNNUEInput(board.getPieceBitboard(side | PAWN).popcount());
        additionalInput[1] = convertPieceCountToNNUEInput(board.getPieceBitboard(side | KNIGHT).popcount());
        Bitboard bishops = board.getPieceBitboard(side | BISHOP);
        additionalInput[2] = convertPieceCountToNNUEInput((bishops & lightSquares).popcount());
        additionalInput[3] = convertPieceCountToNNUEInput((bishops & ~lightSquares).popcount());
        additionalInput[4] = convertPieceCountToNNUEInput(board.getPieceBitboard(side | ROOK).popcount());
        additionalInput[5] = convertPieceCountToNNUEInput(board.getPieceBitboard(side | QUEEN).popcount());
        additionalInput[6] = convertPieceCountToNNUEInput(board.getPieceBitboard(otherSide | PAWN).popcount());
        additionalInput[7] = convertPieceCountToNNUEInput(board.getPieceBitboard(otherSide | KNIGHT).popcount());
        bishops = board.getPieceBitboard(otherSide | BISHOP);
        additionalInput[8] = convertPieceCountToNNUEInput((bishops & lightSquares).popcount());
        additionalInput[9] = convertPieceCountToNNUEInput((bishops & ~lightSquares).popcount());
        additionalInput[10] = convertPieceCountToNNUEInput(board.getPieceBitboard(otherSide | ROOK).popcount());
        additionalInput[11] = convertPieceCountToNNUEInput(board.getPieceBitboard(otherSide | QUEEN).popcount());

        // Rochadenrechte.
        int castlingRights = board.getCastlingPermission();
        if(side == WHITE) {
            additionalInput[12] = convertBoolToNNUEInput(castlingRights & WHITE_KINGSIDE_CASTLE);
            additionalInput[13] = convertBoolToNNUEInput(castlingRights & WHITE_QUEENSIDE_CASTLE);
            additionalInput[14] = convertBoolToNNUEInput(castlingRights & BLACK_KINGSIDE_CASTLE);
            additionalInput[15] = convertBoolToNNUEInput(castlingRights & BLACK_QUEENSIDE_CASTLE);
        } else {
            additionalInput[12] = convertBoolToNNUEInput(castlingRights & BLACK_KINGSIDE_CASTLE);
            additionalInput[13] = convertBoolToNNUEInput(castlingRights & BLACK_QUEENSIDE_CASTLE);
            additionalInput[14] = convertBoolToNNUEInput(castlingRights & WHITE_KINGSIDE_CASTLE);
            additionalInput[15] = convertBoolToNNUEInput(castlingRights & WHITE_QUEENSIDE_CASTLE);
        }

        // En Passant-Feld.
        int enPassantSquare = board.getEnPassantSquare();
        if(enPassantSquare != NO_SQ) {
            int enPassantFile = Square::fileOf(enPassantSquare);
            for(int i = 0; i < 8; i++)
                additionalInput[i + 16] = convertBoolToNNUEInput(enPassantFile == FILE_A + i);
        } else {
            for(int i = 16; i < 24; i++)
                additionalInput[i] = 0;
        }

        // 50-Züge-Regel.
        additionalInput[24] = convertFiftyMoveCounterToNNUEInput(board.getFiftyMoveCounter());

        // Restliche Eingabe sind unbelegt, fülle mit 0 auf.
        for(size_t i = 25; i < NNUE::Network::INPUT_ADDITION; i++)
            additionalInput[i] = 0;
    }
}

#endif