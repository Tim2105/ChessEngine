#ifndef PORTABILITY_HELPER_H
#define PORTABILITY_HELPER_H

#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
    #include <cwchar>
#else
    #include <cstdio>
    #include <termios.h>
    #include <unistd.h>
    #include <stdio.h>
    #include <sys/ioctl.h>
#endif

#include <string>

#define KEY_NONE 0
#define KEY_ARROW_UP 1
#define KEY_ARROW_DOWN 2
#define KEY_ARROW_RIGHT 3
#define KEY_ARROW_LEFT 4

int ngetch();

void clearScreen();

bool nkbhit();

bool isEnterWaiting();

std::string getSkippedInput();

#endif