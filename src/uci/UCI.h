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
    static std::string ENGINE_VERSION = "5.0 (HCE)";
    #else
    static std::string ENGINE_VERSION = "5.0 (NNUE)";
    #endif
    
    static std::string ENGINE_AUTHOR = "Tim Plotzki";

    void listen(const std::vector<std::string>& args = {});

    struct SearchParams {
        int depth = std::numeric_limits<int>::max();
        uint64_t nodes = std::numeric_limits<uint64_t>::max();
        Array<Move, 256> searchmoves = {};
        uint32_t movetime = std::numeric_limits<uint32_t>::max();
        bool useMovetime = false;
        uint32_t wtime = std::numeric_limits<uint32_t>::max();
        uint32_t btime = std::numeric_limits<uint32_t>::max();
        uint32_t winc = 0;
        uint32_t binc = 0;
        unsigned int movestogo = std::numeric_limits<unsigned int>::max();
        bool useWBTime = false;
        int mate = 0;
        bool infinite = false;
        bool ponder = false;
    };
}

#endif