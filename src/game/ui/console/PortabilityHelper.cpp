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
        struct termios term;
        tcgetattr(STDIN_FILENO, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN_FILENO, TCSANOW, &term);
    #endif
}

int ngetch() {
    #ifdef _WIN32
        return _getch();
    #else
        char buf = 0;
        struct termios old;
        fflush(stdout);
        if(tcgetattr(0, &old) < 0)
            perror("tcsetattr()");
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        if(tcsetattr(0, TCSANOW, &old) < 0)
            perror("tcsetattr ICANON");
        if(read(0, &buf, 1) < 0)
            perror("read()");
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        if(tcsetattr(0, TCSADRAIN, &old) < 0)
            perror("tcsetattr ~ICANON");
        return buf;
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
        int ch = ngetch();
        if(ch == 27) {
            ch = ngetch();
            if(ch == 91) {
                ch = ngetch();
                switch(ch) {
                    case 65:
                        return KEY_ARROW_UP;
                    case 66:
                        return KEY_ARROW_DOWN;
                    case 67:
                        return KEY_ARROW_RIGHT;
                    case 68:
                        return KEY_ARROW_LEFT;
                    default:
                        return KEY_NONE;
                }
            }
        }

        return ch;
    #endif
}

void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        printf("\033[2J\033[1;1H");
    #endif
}