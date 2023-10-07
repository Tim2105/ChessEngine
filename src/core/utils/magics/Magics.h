#ifndef MAGICS_H
#define MAGICS_H

#include <stdint.h>

class Magics {

    public:
        static uint64_t rookAttacks[64][4096];
        static uint64_t bishopAttacks[64][512];

        static void initializeMagics();

};

#endif