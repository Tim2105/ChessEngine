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

#include "test/Tournament.h"
#include "test/Perft.h"

#include "core/utils/magics/Magics.h"
#include "core/utils/magics/MagicsFinder.h"

#include <fstream>

int main() {
    initializeConsole();

    std::ofstream file("magics.txt");
    MagicsFinder::searchForRookMagics(file, std::chrono::seconds(50));

    // std::cout << MagicsFinder::findRookMagic(0, 12) << std::endl;
    // std::cout << MagicsFinder::findRookMagic(1, 12) << std::endl;

    return 0;
}

#endif