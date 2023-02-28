#ifndef __EMSCRIPTEN__
#include <iostream>
#include <chrono>
#include "core/engine/SingleThreadedSearchTree.h"
#include "core/utils/MoveNotations.h"
#include <iomanip>
#include <random>
#include "core/engine/StaticEvaluator.h"
#include "test/Tournament.h"

#ifdef _WIN32
    #include <windows.h>
    #include <cwchar>
#endif


void perft(Board& board, int depth, int& count) {
    if(depth <= 1) {
        count += board.generateLegalMoves().size();
        return;
    }

    for(Move m : board.generateLegalMoves()) {
        board.makeMove(m);
        perft(board, depth - 1, count);
        board.undoMove();
    }
}

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
    SingleThreadedSearchTree st(evaluator);

    // st.search(20000);
    
    Move move;
    while(board.generateLegalMoves().size() > 0 && !evaluator.isDraw()) {
        move = getUserMove(board);
        board.makeMove(move);

        if(board.generateLegalMoves().size() == 0 || evaluator.isDraw()) {
            break;
        }

        std::cout << std::endl << "Thinking..." << std::endl;
        st.search(5000);
        move = st.getPrincipalVariation()[0];
        std::cout << std::endl << "Depth: " << st.getLastSearchDepth() << " Computer move: " << toFigurineAlgebraicNotation(move, board) << std::endl << std::endl;
        board.makeMove(move);
    }

    return 0;
}

#endif