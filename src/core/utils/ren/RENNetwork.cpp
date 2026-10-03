#include "core/utils/ren/RENNetwork.h"
#include "core/utils/ren/RENData.h"

#include <iomanip>
#include <sstream>
#include <streambuf>

using namespace REN;

struct membuf : std::streambuf {
    membuf(char* begin, char* end) {
        this->setg(begin, begin, end);
    }
};

Network::Network() {
    #ifdef USE_REN
        membuf buf(_binary_resources_network_ren_start, _binary_resources_network_ren_end);
        std::istream is(&buf);
        is >> *this;
    #endif
}

namespace REN {
    Network DEFAULT_NETWORK;

    std::istream& operator>>(std::istream& is, Network& network) {
        uint32_t version;
        readLittleEndian(is, version);

        if(version != Network::SUPPORTED_VERSION)
            throw std::runtime_error("Unsupported network version (" + std::to_string(version) + ")."
                "Supported version is " + std::to_string(Network::SUPPORTED_VERSION) + ".");

        is >> network.halfKAv2_hmLayer >>
            network.renLayer >>
            network.outputLayer;

        if(!is.good())
            throw std::runtime_error("Error while reading network");
        else if(is.rdbuf()->in_avail() > 0)
            throw std::runtime_error("Network file is too large. " +
                                    std::to_string(is.rdbuf()->in_avail()) +
                                    " bytes remaining!");

        return is;
    }

    std::ostream& operator<<(std::ostream& os, const Network& network) {
        writeLittleEndian(os, Network::SUPPORTED_VERSION);

        os << network.halfKAv2_hmLayer <<
           network.renLayer <<
           network.outputLayer;

        if(!os.good())
            throw std::runtime_error("Error while writing network");

        return os;
    }
}