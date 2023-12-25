#ifndef __EMSCRIPTEN__
#include "core/chess/Board.h"
#include "core/engine/StaticEvaluator.h"
#include "core/engine/UpdatedEvaluator.h"
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

int main() {
    initializeConsole();

    Magics::initializeMagics();

    // ConsoleNavigator navigator;

    // navigator.navigate();

    Board b("3r2k1/ppq2pp1/4p2p/3n3P/3N2P1/2P5/PP2QP2/K2R4 b - - 0 1");
    UpdatedEvaluator evaluator(b);
    PVSEngine engine(evaluator);

    engine.search(40000);

    // StaticEvaluator staticEvaluator(b);
    // MinimaxEngine minimax(staticEvaluator);

    // Tournament tournament(engine, minimax, "PVS", "Minimax");
    // tournament.run();

    return 0;
}

#endif