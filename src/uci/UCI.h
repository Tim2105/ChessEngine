#ifndef UCI_H
#define UCI_H

#include <initializer_list>
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
}

#endif