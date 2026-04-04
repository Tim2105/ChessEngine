#include "core/utils/nnue/NNUEData.h"
#include "core/utils/nnue/NNUENetwork.h"

#include <iomanip>
#include <sstream>
#include <streambuf>

using namespace NNUE;

struct membuf : std::streambuf {
    membuf(char* begin, char* end) {
        this->setg(begin, begin, end);
    }
};

Network::Network() {
    #if not defined(USE_HCE)
        membuf buf(_binary_resources_network_nnue_start, _binary_resources_network_nnue_end);
        std::istream is(&buf);

        is >> *this;
    #endif
}

namespace NNUE {
    Network DEFAULT_NETWORK;

    std::istream& operator>>(std::istream& is, Network& network) {
        readLittleEndian(is, network.version);

        if(network.version != Network::SUPPORTED_VERSION)
            throw std::runtime_error("Unsupported network version");

        is >> network.halfKPLayer >>
            network.layer1 >>
            network.layer2 >>
            network.layer3;

        if(!is.good())
            throw std::runtime_error("Error while reading network");
        else if(is.rdbuf()->in_avail() > 0)
            throw std::runtime_error("Network file is too large. " +
                                    std::to_string(is.rdbuf()->in_avail()) +
                                    " bytes remaining!");

        return is;
    }

    std::ostream& operator<<(std::ostream& os, const Network& network) {
        writeLittleEndian(os, network.version);

        os << network.halfKPLayer <<
           network.layer1 <<
           network.layer2 <<
           network.layer3;

        if(!os.good())
            throw std::runtime_error("Error while writing network");

        return os;
    }
}