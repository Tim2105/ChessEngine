#include "core/utils/hce/HCEParameters.h"
#include "core/utils/hce/HCEData.h"

#include <streambuf>

const HCEParameters HCE_PARAMS;

struct membuf : std::streambuf {
    membuf(char* begin, char* end) {
        this->setg(begin, begin, end);
    }
};

HCEParameters::HCEParameters() {
    #if defined(USE_HCE)
        membuf buf(_binary_resources_params_hce_start, _binary_resources_params_hce_end);
        std::istream is(&buf);

        loadParameters(is);
    #endif
}

HCEParameters::HCEParameters(std::istream& is) {
    loadParameters(is);
}

HCEParameters::~HCEParameters() {}

void HCEParameters::loadParameters(std::istream& is) {
    is.read((char*)this, sizeof(HCEParameters));
}

void HCEParameters::saveParameters(std::ostream& os) const {
    os.write((char*)this, sizeof(HCEParameters));
}

int32_t& HCEParameters::operator[](size_t index) {
    return ((int32_t*)this)[index];
}

int32_t HCEParameters::operator[](size_t index) const {
    return ((int32_t*)this)[index];
}

bool HCEParameters::isParameterDead(size_t index) const {
    void* mgPSQTPawnFirstRowStart = (void*)&MG_PSQT[0][0];
    void* mgPSQTPawnFirstRowEnd = (void*)((char*)mgPSQTPawnFirstRowStart + 8 * sizeof(int32_t));
    void* mgPSQTPawnLastRowStart = (void*)&MG_PSQT[0][56];
    void* mgPSQTPawnLastRowEnd = (void*)((char*)mgPSQTPawnLastRowStart + 8 * sizeof(int32_t));

    void* egPSQTPawnFirstRowStart = (void*)&EG_PSQT[0][0];
    void* egPSQTPawnFirstRowEnd = (void*)((char*)egPSQTPawnFirstRowStart + 8 * sizeof(int32_t));
    void* egPSQTPawnLastRowStart = (void*)&EG_PSQT[0][56];
    void* egPSQTPawnLastRowEnd = (void*)((char*)egPSQTPawnLastRowStart + 8 * sizeof(int32_t));

    void* idxAddress = (void*)&((int32_t*)this)[index];

    return (idxAddress >= mgPSQTPawnFirstRowStart && idxAddress < mgPSQTPawnFirstRowEnd) ||
            (idxAddress >= mgPSQTPawnLastRowStart && idxAddress < mgPSQTPawnLastRowEnd) ||
            (idxAddress >= egPSQTPawnFirstRowStart && idxAddress < egPSQTPawnFirstRowEnd) ||
            (idxAddress >= egPSQTPawnLastRowStart && idxAddress < egPSQTPawnLastRowEnd);
}