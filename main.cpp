#include <iostream>
#include "Board.h"
#include <chrono>
#include "Bitboard.h"

// perft(4) dauert knapp 7s
void perft(Board board, int depth, int& count, int& captures, int& enPassants, int& castles, int& promotions) {
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

        board.makeMove(m);
        perft(board, depth - 1, count, captures, enPassants, castles, promotions);
        board.undoMove();
    }
}

int main() {
    Board board("r3k2r/2p5/8/8/3Q4/8/8/R3K2R w KQkq - 0 1");

    for(Move m : board.generateLegalMoves()) {
        std::cout << m << std::endl;
    }

    // for(int i = 0; i < 5; i++) {
    //     int count = 0, captures = 0, enPassants = 0, castles = 0, promotions = 0;

    //     // Zeitmessung
    //     auto start = std::chrono::high_resolution_clock::now();

    //     perft(board, i, count, captures, enPassants, castles, promotions);

    //     auto end = std::chrono::high_resolution_clock::now();

    //     std::chrono::duration<double> elapsed = end - start;

    //     std::cout << "Perft(depth " << i << ", " << elapsed.count() << "s): " << std::endl;
    //     std::cout << "    Nodes: " << count << std::endl;
    //     std::cout << "    Captures: " << captures << std::endl;
    //     std::cout << "    En Passants: " << enPassants << std::endl;
    //     std::cout << "    Castles: " << castles << std::endl;
    // }   

    return 0;
}