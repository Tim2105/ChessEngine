#include <iostream>
#include "Board.h"

int main() {
    Board board("rnb1k2r/pp1q1p1p/1bp1pnp1/3p4/4PB1N/1BN2PP1/PPPQ2PP/R3K2R w KQkq - 0 1");

    for(Move m : board.generateLegalMoves())
        std::cout << m << std::endl;

    return 0;
}