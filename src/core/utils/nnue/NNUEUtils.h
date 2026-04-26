#ifndef NNUE_UTILS_H
#define NNUE_UTILS_H

#include "core/chess/BoardDefinitions.h"

#include <algorithm>
#include <limits>
#include <tuple>

namespace NNUE {
    constexpr int KP_INDICES[6][2] = {
        {0, 64}, // Bauern (Wir, Die)
        {128, 192}, // Springer (Wir, Die)
        {256, 320}, // Läufer (Wir, Die)
        {384, 448}, // Türme (Wir, Die)
        {512, 576}, // Damen (Wir, Die)
        {640, 640}  // Könige (Die, Die), nur gegnerische Könige werden kodiert
    };

    constexpr int castlingIdx = 32 * 11 * 64; // Nach allen regulären HalfKP-Features
    constexpr int enPassantIdx = 32 * 11 * 64 + 4; // Nach allen regulären HalfKP-Features + Castling-Features

    constexpr size_t INPUT_SIZE = 32 * 11 * 64 + 12;

    template <int COLOR>
    constexpr int getHalfKPIndex(int kingSq, int pieceSq, int piece) noexcept {
        int pieceType = TYPEOF(piece);
        int pieceColor = piece & COLOR_MASK;

        if constexpr(COLOR == BLACK) {
            kingSq = Square::flipY(kingSq);
            pieceSq = Square::flipY(pieceSq);
            pieceColor ^= COLOR_MASK;
        }

        if(Square::fileOf(kingSq) >= FILE_E) {
            pieceSq = Square::flipX(pieceSq);
            kingSq = Square::flipX(kingSq);
        }

        kingSq -= (Square::rankOf(kingSq) - RANK_1) * 4;

        return kingSq * 704 + KP_INDICES[pieceType - PAWN][pieceColor / COLOR_MASK] + pieceSq;
    }

    template <int COLOR>
    constexpr Array<int, 4> getHalfKPIndexForCastling(int castlingRights) noexcept {
        Array<int, 4> indices;

        if constexpr(COLOR == WHITE) {
            if(castlingRights & WHITE_KINGSIDE_CASTLE)
                indices.push_back(castlingIdx);
            if(castlingRights & WHITE_QUEENSIDE_CASTLE)
                indices.push_back(castlingIdx + 1);
            if(castlingRights & BLACK_KINGSIDE_CASTLE)
                indices.push_back(castlingIdx + 2);
            if(castlingRights & BLACK_QUEENSIDE_CASTLE)
                indices.push_back(castlingIdx + 3);
        } else {
            if(castlingRights & BLACK_KINGSIDE_CASTLE)
                indices.push_back(castlingIdx);
            if(castlingRights & BLACK_QUEENSIDE_CASTLE)
                indices.push_back(castlingIdx + 1);
            if(castlingRights & WHITE_KINGSIDE_CASTLE)
                indices.push_back(castlingIdx + 2);
            if(castlingRights & WHITE_QUEENSIDE_CASTLE)
                indices.push_back(castlingIdx + 3);
        }

        return indices;
    }

    constexpr Array<int, 1> getHalfKPIndexForEnPassant(int enPassantSquare, int kingSq) noexcept {
        Array<int, 1> indices;

        if(enPassantSquare != NO_SQ) {
            if(Square::fileOf(kingSq) >= FILE_E)
                enPassantSquare = Square::flipX(enPassantSquare);

            indices.push_back(enPassantIdx + Square::fileOf(enPassantSquare) - FILE_A);
        }

        return indices;
    }

    inline Array<int, 68> getHalfKPFeatures(const Board& board, int color) noexcept {
        Array<int, 68> activeFeatures;

        int kingSq = board.getKingSquare(color);

        // Weiße Figuren
        if(color == WHITE) {
            for(int piece = (WHITE | PAWN); piece <= (WHITE | QUEEN); piece++) {
                Bitboard pieceBitboard = board.getPieceBitboard(piece);
                while(pieceBitboard) {
                    int sq = pieceBitboard.popFSB();
                    activeFeatures.push_back(getHalfKPIndex<WHITE>(kingSq, sq, piece));
                }
            }

            // Schwarze Figuren
            for(int piece = (BLACK | PAWN); piece <= (BLACK | KING); piece++) {
                Bitboard pieceBitboard = board.getPieceBitboard(piece);
                while(pieceBitboard) {
                    int sq = pieceBitboard.popFSB();
                    activeFeatures.push_back(getHalfKPIndex<WHITE>(kingSq, sq, piece));
                }
            }

            // Rochaden
            activeFeatures.push_back(getHalfKPIndexForCastling<WHITE>(board.getCastlingPermission()));
        } else {
            for(int piece = (WHITE | PAWN); piece <= (WHITE | KING); piece++) {
                Bitboard pieceBitboard = board.getPieceBitboard(piece);
                while(pieceBitboard) {
                    int sq = pieceBitboard.popFSB();
                    activeFeatures.push_back(getHalfKPIndex<BLACK>(kingSq, sq, piece));
                }
            }

            // Schwarze Figuren
            for(int piece = (BLACK | PAWN); piece <= (BLACK | QUEEN); piece++) {
                Bitboard pieceBitboard = board.getPieceBitboard(piece);
                while(pieceBitboard) {
                    int sq = pieceBitboard.popFSB();
                    activeFeatures.push_back(getHalfKPIndex<BLACK>(kingSq, sq, piece));
                }
            }

            // Rochaden
            activeFeatures.push_back(getHalfKPIndexForCastling<BLACK>(board.getCastlingPermission()));
        }

        // En Passant
        activeFeatures.push_back(getHalfKPIndexForEnPassant(board.getEnPassantSquare(), kingSq));

        return activeFeatures;
    }
}

#endif