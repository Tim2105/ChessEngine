#include <iostream>
#include "Board.h"
#include <chrono>
#include "SearchTree.h"
#include <iomanip>
#include "EvaluationDefinitions.h"
#include "MoveNotations.h"
#include <random>

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
            if(move == m.toString()) {
                return m;
            }
        }

        std::cout << "Invalid move" << std::endl;
    }
}

int32_t runGame(SearchTree& s1, SearchTree& s2, Board& board, int32_t time = 100) {
    s1.setBoard(board);
    s2.setBoard(board);
    BoardEvaluator evaluator(board);
    bool white = board.getSideToMove() == WHITE;

    while(board.generateLegalMoves().size() != 0 && !evaluator.isDraw()) {
        int16_t score = s1.search(time);
        Move m = s1.getPrincipalVariation()[0];
        board.makeMove(m);

        white = !white;

        if(board.generateLegalMoves().size() == 0)
            break;

        int16_t score2 = s2.search(time);
        Move m2 = s2.getPrincipalVariation()[0];
        board.makeMove(m2);

        white = !white;
    }

    if(evaluator.isDraw()) {
        return 0;
    } else {
        if(white) {
            return -1;
        } else {
            return 1;
        }
    }
}

std::string openings[] = {
    "r1bqkbnr/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 0 1", "Ruy Lopez Opening",
    "rnbqkbnr/pppp1ppp/4p3/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1", "French Defense",
    "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 1", "Sicilian Defense",
    "rnbqkbnr/pp1ppppp/2p5/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1", "Caro-Kann Defense",
    "r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 0 1", "Italian Game",
    "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 1", "Scandinavian Defense",
    "rnbqkb1r/ppp1pppp/3p1n2/8/3PP3/8/PPP2PPP/RNBQKBNR w KQkq - 0 1", "Pirc Defense",
    "rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 1", "Queen's Gambit",
    "rnbqkb1r/pppppp1p/5np1/8/2PP4/8/PP2PPPP/RNBQKBNR w KQkq - 0 1", "King's Indian Defense",
    "rnbqk2r/pppp1ppp/4pn2/8/1bPP4/2N5/PP2PPPP/R1BQKBNR w KQkq - 0 1", "Nimzo-Indian Defense",
    "rnbqkb1r/ppp1pp1p/5np1/3p4/2PP4/2N5/PP2PPPP/R1BQKBNR w KQkq d6 0 1", "Grünfeld Defense",
    "rnbqkb1r/ppp1pppp/5n2/3p4/3P1B2/5N2/PPP1PPPP/RN1QKB1R b KQkq - 0 1", "London System",
    "rnbqkbnr/pppppppp/8/8/2P5/8/PP1PPPPP/RNBQKBNR b KQkq c3 0 1", "English Opening",
    "rnbqkbnr/ppp1pppp/8/3p4/8/5NP1/PPPPPP1P/RNBQKB1R b KQkq - 0 1", "King's Indian Attack"
};

void runTournament(SearchTree& s1, SearchTree& s2, std::string name1, std::string name2) {
    int32_t timeControls[] = {100, 200, 500, 1000};
    int32_t games[] = {10, 6, 4, 3};

    int32_t engine1Wins[4] = {0};
    int32_t engine2Wins[4] = {0};
    int32_t draws[4] = {0};

    for(int i = 0; i < 4; i++) {
        std::cout << "Time control: " << timeControls[i] << "ms" << std::endl;

        for(int j = 0; j < games[i]; j++) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 13);

            int32_t opening = dis(gen);

            std::string openingName = openings[2 * opening + 1];
            std::string openingFen = openings[2 * opening];
            std::cout << "Opening: " << openingName << std::endl;
            std::cout << std::endl;

            Board board(openingFen);
            int32_t engine1Color = board.getSideToMove() == WHITE ? 1 : -1;
            int32_t engine2Color = -engine1Color;

            int32_t result = runGame(s1, s2, board, timeControls[i]);

            if(result == engine1Color) {
                engine1Wins[i]++;
                std::cout << name1 << " wins with" << (engine1Color == 1 ? " white" : " black");
            } else if(result == engine2Color) {
                engine2Wins[i]++;
                std::cout << name2 << " wins with" << (engine2Color == 1 ? " white" : " black");
            } else {
                draws[i]++;
                std::cout << "Draw";
            }

            std::cout << "(" << j * 2 + 1 << "/" << games[i] * 2 << ")" << std::endl;

            board = Board(openingFen);
            engine1Color = -engine1Color;
            engine2Color = -engine2Color;

            result = runGame(s2, s1, board, timeControls[i]);

            if(result == engine2Color) {
                engine2Wins[i]++;
                std::cout << name2 << " wins with" << (engine2Color == 1 ? " white" : " black");
            } else if(result == engine1Color) {
                engine1Wins[i]++;
                std::cout << name1 << " wins with" << (engine1Color == 1 ? " white" : " black");
            } else {
                draws[i]++;
                std::cout << "Draw";
            }

            std::cout << "(" << j * 2 + 2 << "/" << games[i] * 2 << ")" << std::endl;
            std::cout << std::endl;
        }

        std::cout << std::endl;
    }

    std::cout << std::endl << std::endl;
    std::cout << "Results:" << std::endl;
    std::cout << std::setw(20) << "Time Control(ms)" << std::setw(20) << name1 << std::setw(20) << name2 << std::setw(20) << "Draw" << std::endl;
    for(int i = 0; i < 4; i++) {
        std::cout << std::setw(20) << timeControls[i] << std::setw(20) << engine1Wins[i] << std::setw(20) << engine2Wins[i] << std::setw(20) << draws[i] << std::endl;
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
    SearchTree st(board);
    
    st.search(50000);

    return 0;
}