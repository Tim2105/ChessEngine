#ifndef REN_REN_H
#define REN_REN_H

#include "core/utils/ren/HalfKAv2_hm.h"
#include "core/utils/ren/RENLayer.h"

namespace REN {
    class Network {
        public:
            static constexpr uint32_t SUPPORTED_VERSION = 0x20u;
            static constexpr size_t INPUT_SIZE = 22540;
            static constexpr size_t SINGLE_SUBNET_SIZE = 64;
            static constexpr size_t REN_SIZE = SINGLE_SUBNET_SIZE * 2;

        private:
            HalfKAv2_hmLayer halfKAv2Layer;
            RENLayer renLayer;
            
    };
}


#endif