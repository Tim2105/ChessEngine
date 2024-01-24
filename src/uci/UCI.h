#ifndef UCI_H
#define UCI_H

#include "core/chess/Move.h"
#include "core/utils/Array.h"

#include <initializer_list>
#include <limits>
#include <stdint.h>
#include <string>

namespace UCI {
    static std::string ENGINE_NAME = "Chess Engine";

    #if defined(USE_HCE)
    static std::string ENGINE_VERSION = "4.0 (HCE)";
    #else
    static std::string ENGINE_VERSION = "4.0 (NNUE)";
    #endif
    
    static std::string ENGINE_AUTHOR = "Tim Plotzki";

    void listen();

    struct SearchParams {
        uint32_t depth = std::numeric_limits<uint32_t>::max();
        uint64_t nodes = std::numeric_limits<uint64_t>::max();
        Array<Move, 256> searchmoves;
        bool useSearchmoves = false;
        uint32_t movetime = 0;
        bool useMovetime = false;
        uint32_t wtime = 0;
        uint32_t btime = 0;
        uint32_t winc = 0;
        uint32_t binc = 0;
        uint32_t movestogo = std::numeric_limits<uint32_t>::max();
        bool useWBTime = false;
        uint32_t mate = 0;
        bool infinite = false;
        bool ponder = false;
    };
}

#endif