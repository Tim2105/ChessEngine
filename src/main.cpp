#ifndef __EMSCRIPTEN__
#ifndef TUNE
#include "core/utils/magics/Magics.h"

#include "uci/UCI.h"

int main() {
    Magics::initializeMagics();

    UCI::listen();

    return 0;
}

#endif
#endif