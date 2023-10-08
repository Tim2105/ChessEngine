#ifndef __EMSCRIPTEN__
#include "core/chess/Board.h"
#include "core/engine/StaticEvaluator.h"
#include "core/engine/MinimaxEngine.h"

#include "game/ComputerPlayer.h"
#include "game/TimeControlledGame.h"
#include "game/ui/console/BoardVisualizer.h"
#include "game/ui/console/ConsolePlayer.h"
#include "game/ui/console/ConsoleGameStateOutput.h"
#include "game/ui/console/ConsolePGNGameAnalyzer.h"

#include "game/ui/console/ConsoleNavigator.h"
#include "game/ui/console/PortabilityHelper.h"

// #include "test/Perft.h"
#include "test/TestMagics.h"
#include "test/Tournament.h"

#include "core/utils/magics/Magics.h"
#include "core/utils/magics/MagicsFinder.h"

#include <fstream>

uint64_t perft1(Board& board, int depth) {
    if (depth == 0) {
        return 1;
    }

    Array<Move, 256> moves = board.generatePseudoLegalMoves();

    uint64_t count = 0;

    for (Move m : moves) {
        if(!board.isMoveLegal(m)) {
            continue;
        }

        board.makeMove(m);

        count += perft1(board, depth - 1);

        board.undoMove();
    }

    return count;
}

void printPerft(Board& b, int depth) {
    std::cout << "Generating perft for depth " << depth << std::endl << std::endl;
    Array<Move, 256> moves = b.generatePseudoLegalMoves();
    uint64_t acc = 0;

    for (Move m : moves) {
        if(!b.isMoveLegal(m)) {
            continue;
        }

        b.makeMove(m);

        uint64_t count = perft1(b, depth - 1);
        acc += count;

        b.undoMove();

        std::cout << m << ": " << count << std::endl;
    }

    std::cout << "Total: " << acc << std::endl;
}

int main() {
    initializeConsole();

    Magics::initializeMagics();

    Board board("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");

    printPerft(board, 5);

    return 0;
}

#endif