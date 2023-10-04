#include "game/ui/console/BoardVisualizer.h"
#include "game/ui/console/ConsoleGameStateOutput.h"
#include "game/ui/console/PortabilityHelper.h"

#include <iostream>
#include <sstream>

void ConsoleGameStateOutput::outputGameState(std::string whiteAdditionalInfo, std::string blackAdditionalInfo) {
    size_t longestWhiteInfoLine = 0;
    size_t longestBlackInfoLine = 0;

    std::stringstream whiteAdditionalInfoStream(whiteAdditionalInfo);
    std::stringstream blackAdditionalInfoStream(blackAdditionalInfo);

    std::string line;
    while(std::getline(whiteAdditionalInfoStream, line)) {
        if(line.length() > longestWhiteInfoLine)
            longestWhiteInfoLine = line.length();
    }

    while(std::getline(blackAdditionalInfoStream, line)) {
        if(line.length() > longestBlackInfoLine)
            longestBlackInfoLine = line.length();
    }

    whiteAdditionalInfoStream.clear();
    blackAdditionalInfoStream.clear();

    whiteAdditionalInfoStream.seekg(0, std::ios::beg);
    blackAdditionalInfoStream.seekg(0, std::ios::beg);

    std::stringstream whiteAdditionalInfoStreamPadded;
    std::stringstream blackAdditionalInfoStreamPadded;

    size_t whitePaddingWidth = (BOARD_WIDTH - longestWhiteInfoLine) / 2;
    size_t blackPaddingWidth = (BOARD_WIDTH - longestBlackInfoLine) / 2;

    while(std::getline(whiteAdditionalInfoStream, line))
        whiteAdditionalInfoStreamPadded << std::string(whitePaddingWidth, ' ') << line << std::endl;

    while(std::getline(blackAdditionalInfoStream, line))
        blackAdditionalInfoStreamPadded << std::string(blackPaddingWidth, ' ') << line << std::endl;

    whiteAdditionalInfo = whiteAdditionalInfoStreamPadded.str();
    blackAdditionalInfo = blackAdditionalInfoStreamPadded.str();

    clearScreen();

    if(flipBoard) {
        std::cout << whiteAdditionalInfo << std::endl << std::endl;
        std::cout << visualizeBoardWithFigurines(board, true) << std::endl;
        std::cout << blackAdditionalInfo << std::endl;
    } else {
        std::cout << blackAdditionalInfo << std::endl << std::endl;
        std::cout << visualizeBoardWithFigurines(board) << std::endl;
        std::cout << whiteAdditionalInfo << std::endl;
    }

    return;
}