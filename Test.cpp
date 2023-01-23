#include "Test.h"

#include <iostream>
#include <fstream>
#include "Array.h"

MovegenTestCase::MovegenTestCase(Board b, std::string d, std::vector<std::string> e) : board(b), description(d), expectedOutcomes(e) {
    std::cout << "Created test case: " << description << std::endl << std::endl;
}

MovegenTestCase::~MovegenTestCase() {

}

bool MovegenTestCase::run() {
    std::cout << "Running test case: " << description << std::endl;
    std::string fen = board.fenString();
    Array<Move, 256> moves = board.generateLegalMoves();

    std::cout << "Generated " << moves.size() << " moves." << std::endl;
    std::cout << "Expected " << expectedOutcomes.size() << " moves." << std::endl;
    if(moves.size() != expectedOutcomes.size()) {
        std::cout << "Test case failed!" << std::endl 
            << "FEN: " << fen << std::endl << std::endl;

        return false;
    }
    
    for(Move m : moves) {
        bool found = false;
        for(std::string s : expectedOutcomes) {
            board.makeMove(m);
            if(board.fenString() == s) {
                found = true;
                board.undoMove();
                break;
            }
            board.undoMove();
            
        }
        if(!found) {
            std::cout << "Test case failed!" << std::endl;
            std::cout << "Move " << m.toString() << " not found in expected outcomes."
                << std::endl << "FEN: " << fen
                << std::endl << std::endl;
            return false;
        }
    }
    std::cout << "Test case passed!" << std::endl << std::endl;
    return true;
}

std::vector<MovegenTestCase> movegenTestCasesFromJSON(std::string filename) {
    std::vector<MovegenTestCase> testCases;

    std::ifstream file(filename);

    if(!file.is_open()) {
        std::cout << "Could not open file " << filename << std::endl;
        return testCases;
    }

    std::string line;

    // bis zum Array 'testCases' springen
    while(std::getline(file, line)) {
        if(line.find("testCases") != std::string::npos) {
            break;
        }
    }

    // Array 'testCases' durchlaufen
    Board board;
    std::string description;
    std::vector<std::string> expectedOutcomes;

    while(std::getline(file, line)) {
        if(line.find("]") != std::string::npos) {
            break;
        }

        if(line.find("fen") != std::string::npos) {
            std::string fen = line.substr(line.find(":") + 3, line.find_last_of("\"") - line.find(":") - 3);
            board = Board(fen);
        }

        if(line.find("description") != std::string::npos) {
            description = line.substr(line.find(":") + 3, line.find_last_of("\"") - line.find(":") - 3);
        }

        if(line.find("expected") != std::string::npos) {
            while(std::getline(file, line)) {
                if(line.find("]") != std::string::npos) {
                    break;
                }

                if(line.find("fen") != std::string::npos) {
                    std::string fen = line.substr(line.find(":") + 3, line.find_last_of("\"") - line.find(":") - 3);
                    expectedOutcomes.push_back(fen);
                }
            }

            testCases.push_back(MovegenTestCase(board, description, expectedOutcomes));
            expectedOutcomes.clear();
        }
    }


    return testCases;
}
