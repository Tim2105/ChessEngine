#ifndef CONSOLE_NAVIGATOR_H
#define CONSOLE_NAVIGATOR_H

#include "game/ui/Navigator.h"

class ConsoleNavigator : public Navigator {

    private:
        void startGame();
        void startAnalysis();
        void startLiveAnalysis();

    public:
        ConsoleNavigator() = default;

        void navigate() override;
};

#endif