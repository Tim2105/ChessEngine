#include "uci/PortabilityHelper.h"

std::string skippedInput = "";

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

void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        printf("\033[2J\033[1;1H");
    #endif
}

bool nkbhit() {
    #ifdef _WIN32
        return _kbhit();
    #else
        struct termios term;
        tcgetattr(STDIN_FILENO, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN_FILENO, TCSANOW, &term);
        setbuf(stdin, NULL);

        int bytesWaiting;
        ioctl(0, FIONREAD, &bytesWaiting);

        term.c_lflag |= ICANON;
        tcsetattr(STDIN_FILENO, TCSANOW, &term);

        return bytesWaiting > 0;
    #endif
}

bool isEnterWaiting() {
    // Überprüfe, ob ein Enter gedrückt wurde
    // ohne es aus dem Input-Buffer zu entfernen

    #ifdef _WIN32
        if(nkbhit()) {
            int c = ngetch();
            if(c == '\r')
                return true;

            skippedInput += c;
        }

        return false;
    #else
        if(nkbhit()) {
            int c = ngetch();
            if(c == '\n')
                return true;

            skippedInput += c;
        }

        return false;
    #endif
}

std::string getSkippedInput() {
    std::string input = skippedInput;
    skippedInput = "";
    return input;
}