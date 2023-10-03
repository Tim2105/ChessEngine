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

#include <fstream>

#ifdef _WIN32
    #include <windows.h>
    #include <cwchar>
#endif

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

    return 0;
}

#endif