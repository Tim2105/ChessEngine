#ifndef __EMSCRIPTEN__
#include "core/chess/Board.h"
#include "core/engine/StaticEvaluator.h"
#include "core/engine/SimpleUpdatedEvaluator.h"
#include "core/engine/NNUEEvaluator.h"
#include "core/engine/MinimaxEngine.h"
#include "core/engine/PVSEngine.h"
#include "core/utils/magics/Magics.h"

#include "game/ComputerPlayer.h"
#include "game/TimeControlledGame.h"
#include "game/ui/console/BoardVisualizer.h"
#include "game/ui/console/ConsolePlayer.h"
#include "game/ui/console/ConsoleGameStateOutput.h"
#include "game/ui/console/ConsolePGNGameAnalyzer.h"

#include "game/ui/console/ConsoleNavigator.h"
#include "game/ui/console/PortabilityHelper.h"

#include "test/Perft.h"
#include "test/TestMagics.h"
#include "test/Tournament.h"

#include <fstream>

int main() {
    initializeConsole();

    Magics::initializeMagics();

    // ConsoleNavigator navigator;

    // navigator.navigate();

    Board b;
    std::ifstream is("network.nnue", std::ios::binary);

    NNUEEvaluator evaluator(b, is);
    PVSEngine engine(evaluator);

    engine.search(20000);

    StaticEvaluator staticEvaluator(b);
    MinimaxEngine minimax(staticEvaluator);

    Tournament tournament(engine, minimax, "PVS", "Minimax");
    tournament.run();

    return 0;
}

#endif