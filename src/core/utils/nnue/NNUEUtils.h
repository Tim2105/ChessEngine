#ifndef NNUE_UTILS_H
#define NNUE_UTILS_H

#include "core/chess/BoardDefinitions.h"

#include <tuple>

namespace NNUE {
    constexpr int32_t rotate(int32_t square) noexcept {
        return square ^ 63;
    }

    constexpr int32_t KP_INDICES[7][2] = {
        {0, 0},
        {1, 65},
        {129, 193},
        {257, 321},
        {385, 449},
        {513, 577},
        {641, 705}
    };

    template <int32_t COLOR>
    constexpr int32_t getHalfKPIndex(int32_t kingSq, int32_t pieceSq, int32_t piece) noexcept {
        int32_t pieceType = TYPEOF(piece);
        int32_t pieceColor = piece & COLOR_MASK;

        if constexpr(COLOR == BLACK) {
            kingSq = rotate(kingSq);
            pieceSq = rotate(pieceSq);
            pieceColor ^= COLOR_MASK;
        }

        return pieceSq + KP_INDICES[pieceType][pieceColor / COLOR_MASK] + kingSq * 641;
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
            for(int32_t piece = (BLACK | PAWN); piece <= (BLACK | QUEEN); piece++) {
                Bitboard pieceBitboard = board.getPieceBitboard(piece);
                while(pieceBitboard) {
                    int32_t sq = pieceBitboard.popFSB();
                    activeFeatures.push_back(getHalfKPIndex<WHITE>(kingSq, sq, piece));
                }
            }
        } else {
            for(int32_t piece = (WHITE | PAWN); piece <= (WHITE | QUEEN); piece++) {
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
}

#endif