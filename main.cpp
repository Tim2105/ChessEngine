#include <iostream>
#include "Board.h"

int main() {
    Board board("rnbqk1nr/p1p1b1Pp/1p1P4/8/4pP2/1P6/P2P2PP/RNBQKBNR b KQkq f3 0 1");

    for(Move m : board.generateBlackPawnMoves())
        std::cout << m << std::endl;

    return 0;
}