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
    halfKPLayer = new HalfKPLayer;

    membuf buf(_binary_resources_network_nnue_start, _binary_resources_network_nnue_end);
    std::istream is(&buf);

    is >> *this;
}

Network::~Network() {
    delete halfKPLayer;
}

namespace NNUE {
    Network network;

    std::istream& operator>>(std::istream& is, Network& network) {
        readLittleEndian(is, network.version);

        if(network.version != Network::SUPPORTED_VERSION)
            throw std::runtime_error("Unsupported network version");
        
        readLittleEndian(is, network.hash);
        readLittleEndian(is, network.headerSize);

        char c;
        std::stringstream ss;
        for(size_t i = 0; i < network.headerSize; i++) {
            if(!is.get(c))
                break;
            
            ss << c;
        }

        network.header = ss.str();

        readLittleEndian(is, network.halfKPHash);

        is >> *network.halfKPLayer;

        readLittleEndian(is, network.layer1Hash);

        is >> network.layer1 >>
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
        os << "Version: 0x" << std::hex << std::setw(8) <<
            std::setfill('0') << network.version << std::dec << ", ";

        os << "Hash: 0x" << std::hex << std::setw(8) <<
            std::setfill('0') << network.hash << std::dec << ", ";

        os << "Header Size: " << network.headerSize << ", ";

        os << "Header: " << network.header;

        os << std::setfill(' ') << std::endl;

        return os;
    }
}