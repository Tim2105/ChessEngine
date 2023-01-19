#include <iostream>
#include "Board.h"
#include <chrono>
#include "Bitboard.h"
#include "Test.h"
#include <map>

void testBoardIntegrity(Board& board) {
    for(int piece = 0; piece < 15; piece++) {
        for(int sq : board.pieceList[piece]) {
            ASSERT(piece == board.pieces[sq]);
            ASSERT(board.pieceBitboard[piece].getBit(board.mailbox[sq]));
        }
    }

    for(int sq = 0; sq < 120; sq++) {
        int32_t piece = board.pieces[sq];
        if(piece != EMPTY) {
            bool found = false;
            for(int sq2 : board.pieceList[piece]) {
                if(sq2 == sq) {
                    found = true;
                    break;
                }
            }
            ASSERT(found);
            ASSERT(board.mailbox[sq] != NO_SQ);
            ASSERT(board.pieceBitboard[piece].getBit(board.mailbox[sq]));
        }
    }

    Bitboard whitePiecesBitboard = board.pieceBitboard[WHITE_PAWN] | board.pieceBitboard[WHITE_KNIGHT] | board.pieceBitboard[WHITE_BISHOP] | board.pieceBitboard[WHITE_ROOK] | board.pieceBitboard[WHITE_QUEEN];
    Bitboard blackPiecesBitboard = board.pieceBitboard[BLACK_PAWN] | board.pieceBitboard[BLACK_KNIGHT] | board.pieceBitboard[BLACK_BISHOP] | board.pieceBitboard[BLACK_ROOK] | board.pieceBitboard[BLACK_QUEEN];

    ASSERT(whitePiecesBitboard == board.whitePiecesBitboard);
    ASSERT(blackPiecesBitboard == board.blackPiecesBitboard);

    ASSERT((whitePiecesBitboard | blackPiecesBitboard) == board.allPiecesBitboard);
}

// perft(4) dauert knapp 7s
void perft(Board& board, int depth, int& count, int& captures, int& enPassants, int& castles, int& promotions) {
    if(depth == 0) {
        return;
    }

    for(Move m : board.generateLegalMoves()) {
        if(depth == 1) {
            count++;
            if(m.isCapture())
                captures++;
            if(m.isEnPassant())
                enPassants++;
            if(m.isCastle())
                castles++;
            if(m.isPromotion())
                promotions++;
        }

        int countPrev = count;

        board.makeMove(m);
        perft(board, depth - 1, count, captures, enPassants, castles, promotions);
        board.undoMove();
    }
}

int main() {
    Board board;

    for(int i = 1; i < 5; i++) {
        int count = 0, captures = 0, enPassants = 0, castles = 0, promotions = 0;

        // Zeitmessung
        auto start = std::chrono::high_resolution_clock::now();

        perft(board, i, count, captures, enPassants, castles, promotions);

        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> elapsed = end - start;

        std::cout << "Perft(depth " << i << ", " << elapsed.count() << "s): " << std::endl;
        std::cout << "    Nodes: " << count << std::endl;
        std::cout << "    Captures: " << captures << std::endl;
        std::cout << "    En Passants: " << enPassants << std::endl;
        std::cout << "    Castles: " << castles << std::endl;
        std::cout << "    Promotions: " << promotions << std::endl;
        std::cout << "    Nodes/s: " << count / elapsed.count() << std::endl;
    }

    return 0;
}