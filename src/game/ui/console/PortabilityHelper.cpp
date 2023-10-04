#include "game/ui/console/PortabilityHelper.h"

void initializeConsole() {
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        CONSOLE_FONT_INFOEX cfi;
        cfi.cbSize = sizeof(cfi);
        cfi.nFont = 0;
        cfi.dwFontSize.X = 0;
        cfi.dwFontSize.Y = 24;
        cfi.FontFamily = FF_DONTCARE;
        cfi.FontWeight = FW_NORMAL;
        std::wcscpy(cfi.FaceName, L"DejaVu Sans Mono");
        SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);
    #else
        initscr();
        keypad(stdscr, TRUE);
    #endif
}

int ngetch() {
    #ifdef _WIN32
        return _getch();
    #else
        return getch();
    #endif
}

int getchArrowKey() {
    #ifdef _WIN32
        int ch = getch();
        if(ch == 0 || ch == 224) {
            ch = getch();
            switch(ch) {
                case 72:
                    return KEY_ARROW_UP;
                case 80:
                    return KEY_ARROW_DOWN;
                case 77:
                    return KEY_ARROW_RIGHT;
                case 75:
                    return KEY_ARROW_LEFT;
                default:
                    return KEY_NONE;
            }
        }

        return ch;
    #else
        int ch = getch();
        switch(ch) {
            case KEY_UP:
                return KEY_ARROW_UP;
            case KEY_DOWN:
                return KEY_ARROW_DOWN;
            case KEY_RIGHT:
                return KEY_ARROW_RIGHT;
            case KEY_LEFT:
                return KEY_ARROW_LEFT;
            default:
                return ch;
        }
    #endif
}

void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        clear();
    #endif
}