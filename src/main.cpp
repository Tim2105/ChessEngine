#ifndef __EMSCRIPTEN__
#include "core/chess/Board.h"
#include "core/engine/StaticEvaluator.h"
#include "core/engine/MinimaxEngine.h"
#include "core/engine/OldEngine.h"
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

    Board board;

    StaticEvaluator evaluator(board);
    StaticEvaluator evaluator2(board);

    MinimaxEngine engine(evaluator);
    OldEngine engine2(evaluator2);

    Tournament tournament(engine, engine2, "v3.1 new", "v3 old");
    tournament.run();

    return 0;
}

#endif