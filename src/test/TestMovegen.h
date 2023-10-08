#ifndef TEST_MOVEGEN_H	
#define TEST_MOVEGEN_H

#include "core/chess/Board.h"
#include <string>
#include <vector>

class MovegenTestCase {

private:
    Board board;
    std::string description;
    std::vector<std::string> expectedOutcomes;

    public:
        MovegenTestCase(Board b, std::string d, std::vector<std::string> e);
        ~MovegenTestCase();

        bool run();
};

std::vector<MovegenTestCase> movegenTestCasesFromJSON(std::string filename);

#endif