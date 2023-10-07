#ifndef REFEREE_H
#define REFEREE_H

#include "core/chess/Board.h"

class Referee {

    public:
        static bool isDraw(Board& b);
        static bool isDrawByMaterial(Board& b);
        static bool isCheckmate(Board& b);
        static bool isGameOver(Board& b);

};

#endif