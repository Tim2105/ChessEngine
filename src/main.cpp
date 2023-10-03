#ifndef __EMSCRIPTEN__
#include <iostream>
#include <chrono>
#include "core/engine/Engine.h"
#include "core/engine/MinimaxEngine.h"
#include "core/utils/MoveNotations.h"
#include <iomanip>
#include <random>
#include "core/engine/StaticEvaluator.h"
#include "test/Tournament.h"
#include "emscripten/WebAPI.h"
#include "core/chess/MailboxDefinitions.h"
#include <fstream>
#include "test/Perft.h"

#ifdef _WIN32
    #include <windows.h>
    #include <cwchar>
#endif

Move getUserMove(Board& board) {
    while(true) {
        std::string move;
        std::cout << "Enter move: ";
        std::cin >> move;

        for(Move m : board.generateLegalMoves()) {
            if(move == m.toString() || move == toStandardAlgebraicNotation(m, board)) {
                return m;
            }
        }

        std::cout << "Invalid move" << std::endl;
    }
}

int main() {
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        CONSOLE_FONT_INFOEX cfi;
        cfi.cbSize = sizeof(cfi);
        cfi.nFont = 0;
        cfi.dwFontSize.X = 0;
        cfi.dwFontSize.Y = 16;
        cfi.FontFamily = FF_DONTCARE;
        cfi.FontWeight = FW_NORMAL;
        std::wcscpy(cfi.FaceName, L"DejaVu Sans Mono");
        SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);
    #endif

    Board board;

    StaticEvaluator evaluator(board);
    MinimaxEngine engine(evaluator);

    uint32_t remainingTime = 60000;

    while(!evaluator.isDraw() && board.generateLegalMoves().size() != 0) {
        Move m = getUserMove(board);
        board.makeMove(m);

        if(evaluator.isDraw() || board.generateLegalMoves().size() == 0)
            break;

        std::cout << "Thinking..." << std::endl;

        std::chrono::time_point start = std::chrono::high_resolution_clock::now();
        engine.search(remainingTime, true);
        std::chrono::time_point end = std::chrono::high_resolution_clock::now();

        remainingTime -= std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        SearchDetails details = engine.getSearchDetails();

        std::cout << "score " << details.getBestMoveScore() << " depth " << details.depth << " kps " << details.kiloNodesPerSecond() << " pv ";

        for(std::string s : variationToFigurineAlgebraicNotation(engine.getPrincipalVariation(), board))
            std::cout << s << " ";

        std::cout << std::endl;
        std::cout << "Remaining time: " << remainingTime << std::endl;

        board.makeMove(engine.getBestMove());
    }

    return 0;
}

#endif