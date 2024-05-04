#include "core/utils/hce/HCEParameters.h"
#include "core/utils/hce/HCEData.h"

#include <iomanip>
#include <streambuf>

HCEParameters HCE_PARAMS;

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

void HCEParameters::displayParameters(std::ostream& os) const {
    os << "Piece Values MG: [";
    for(size_t i = 0; i < 5; i++)
        os << std::setw(4) << MG_PIECE_VALUE[i] / VALUE_ONE << (i == 4 ? "]\n" : ", ");

    os << "Piece Values EG: [";
    for(size_t i = 0; i < 5; i++)
        os << std::setw(4) << EG_PIECE_VALUE[i] / VALUE_ONE << (i == 4 ? "]\n" : ", ");

    os << "Pawn Imbalance: [\n";
    size_t idx = 0;
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j <= i; j++)
            os << std::setw(3) << PAWN_IMBALANCE_FACTOR[idx++] / VALUE_ONE << ", ";

        for(size_t j = i + 1; j < 8; j++)
            os << std::setw(5) << " ";

        os << "\n";
    }
    os << "]\n";

    os << "Knight Imbalance: [\n";
    idx = 0;
    for(size_t i = 0; i < 3; i++) {
        for(size_t j = 0; j <= i; j++)
            os << std::setw(3) << KNIGHT_IMBALANCE_FACTOR[idx++] / VALUE_ONE << ", ";

        for(size_t j = i + 1; j < 3; j++)
            os << std::setw(5) << " ";

        os << "\n";
    }
    os << "]\n";

    os << "Bishop Imbalance: [\n";
    idx = 0;
    for(size_t i = 0; i < 3; i++) {
        for(size_t j = 0; j <= i; j++)
            os << std::setw(3) << BISHOP_IMBALANCE_FACTOR[idx++] / VALUE_ONE << ", ";

        for(size_t j = i + 1; j < 3; j++)
            os << std::setw(5) << " ";

        os << "\n";
    }
    os << "]\n";

    os << "Rook Imbalance: [\n";
    idx = 0;
    for(size_t i = 0; i < 3; i++) {
        for(size_t j = 0; j <= i; j++)
            os << std::setw(3) << ROOK_IMBALANCE_FACTOR[idx++] / VALUE_ONE << ", ";

        for(size_t j = i + 1; j < 3; j++)
            os << std::setw(5) << " ";

        os << "\n";
    }
    os << "]\n";

    os << "Queen Imbalance: [\n";
    idx = 0;
    for(size_t i = 0; i < 2; i++) {
        for(size_t j = 0; j <= i; j++)
            os << std::setw(3) << QUEEN_IMBALANCE_FACTOR[idx++] / VALUE_ONE << ", ";

        for(size_t j = i + 1; j < 2; j++)
            os << std::setw(5) << " ";

        os << "\n";
    }
    os << "]\n";

    os << "Pawn PSQT MG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 8; j++)
            os << std::setw(3) << MG_PSQT[0][i * 8 + j] / VALUE_ONE << (j == 7 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "Pawn PSQT EG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 8; j++)
            os << std::setw(3) << EG_PSQT[0][i * 8 + j] / VALUE_ONE << (j == 7 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "Knight PSQT MG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 8; j++)
            os << std::setw(3) << MG_PSQT[1][i * 8 + j] / VALUE_ONE << (j == 7 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "Knight PSQT EG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 8; j++)
            os << std::setw(3) << EG_PSQT[1][i * 8 + j] / VALUE_ONE << (j == 7 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "Bishop PSQT MG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 8; j++)
            os << std::setw(3) << MG_PSQT[2][i * 8 + j] / VALUE_ONE << (j == 7 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "Bishop PSQT EG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 8; j++)
            os << std::setw(3) << EG_PSQT[2][i * 8 + j] / VALUE_ONE << (j == 7 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "Rook PSQT MG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 8; j++)
            os << std::setw(3) << MG_PSQT[3][i * 8 + j] / VALUE_ONE << (j == 7 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "Rook PSQT EG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 8; j++)
            os << std::setw(3) << EG_PSQT[3][i * 8 + j] / VALUE_ONE << (j == 7 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "Queen PSQT MG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 8; j++)
            os << std::setw(3) << MG_PSQT[4][i * 8 + j] / VALUE_ONE << (j == 7 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "Queen PSQT EG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 8; j++)
            os << std::setw(3) << EG_PSQT[4][i * 8 + j] / VALUE_ONE << (j == 7 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "King PSQT MG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 8; j++)
            os << std::setw(3) << MG_PSQT[5][i * 8 + j] / VALUE_ONE << (j == 7 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "King PSQT EG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 8; j++)
            os << std::setw(3) << EG_PSQT[5][i * 8 + j] / VALUE_ONE << (j == 7 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "Tempo Bonus MG: " << MG_TEMPO_BONUS / VALUE_ONE << "\n";
    os << "Tempo Bonus EG: " << EG_TEMPO_BONUS / VALUE_ONE << "\n";

    os << "Connected Pawn Bonus MG: [  0, ";
    for(size_t i = 0; i < 6; i++)
        os << std::setw(3) << MG_CONNECTED_PAWN_BONUS[i] / VALUE_ONE << ", ";

    os << "  0]\n";

    os << "Connected Pawn Bonus EG: [  0, ";
    for(size_t i = 0; i < 6; i++)
        os << std::setw(3) << EG_CONNECTED_PAWN_BONUS[i] / VALUE_ONE << ", ";

    os << "  0]\n";

    os << "Doubled Pawn Penalty MG: [";
    for(size_t i = 0; i < 8; i++)
        os << std::setw(3) << MG_DOUBLED_PAWN_PENALTY[i] / VALUE_ONE << (i == 7 ? "]\n" : ", ");

    os << "Doubled Pawn Penalty EG: [";
    for(size_t i = 0; i < 8; i++)
        os << std::setw(3) << EG_DOUBLED_PAWN_PENALTY[i] / VALUE_ONE << (i == 7 ? "]\n" : ", ");

    os << "Isolated Pawn Penalty MG: [";
    for(size_t i = 0; i < 8; i++)
        os << std::setw(3) << MG_ISOLATED_PAWN_PENALTY[i] / VALUE_ONE << (i == 7 ? "]\n" : ", ");

    os << "Isolated Pawn Penalty EG: [";
    for(size_t i = 0; i < 8; i++)
        os << std::setw(3) << EG_ISOLATED_PAWN_PENALTY[i] / VALUE_ONE << (i == 7 ? "]\n" : ", ");

    os << "Backward Pawn Penalty MG: [  0, ";
    for(size_t i = 0; i < 6; i++)
        os << std::setw(3) << MG_BACKWARD_PAWN_PENALTY[i] / VALUE_ONE << ", ";

    os << "  0]\n";

    os << "Backward Pawn Penalty EG: [  0, ";
    for(size_t i = 0; i < 6; i++)
        os << std::setw(3) << EG_BACKWARD_PAWN_PENALTY[i] / VALUE_ONE << ", ";

    os << "  0]\n";

    os << "Passed Pawn Bonus MG: [  0, ";
    for(size_t i = 0; i < 6; i++)
        os << std::setw(3) << MG_PASSED_PAWN_BONUS[i] / VALUE_ONE << ", ";

    os << "  0]\n";

    os << "Passed Pawn Bonus EG: [  0, ";
    for(size_t i = 0; i < 6; i++)
        os << std::setw(3) << EG_PASSED_PAWN_BONUS[i] / VALUE_ONE << ", ";

    os << "  0]\n";

    os << "Strong Square Bonus MG: [\n";
    os << "  0,   0,   0,   0,   0,   0,   0,   0,\n";
    os << "  0,   0,   0,   0,   0,   0,   0,   0,\n";
    for(size_t i = 0; i < 5; i++) {
        for(size_t j = 0; j < 8; j++)
            os << std::setw(3) << MG_STRONG_SQUARE_BONUS[i][j] / VALUE_ONE << (j == 7 ? ",\n" : ", ");
    }
    os << "  0,   0,   0,   0,   0,   0,   0,   0,\n";
    os << "]\n";

    os << "Strong Square Bonus EG: [\n";
    os << "  0,   0,   0,   0,   0,   0,   0,   0,\n";
    os << "  0,   0,   0,   0,   0,   0,   0,   0,\n";
    for(size_t i = 0; i < 5; i++) {
        for(size_t j = 0; j < 8; j++)
            os << std::setw(3) << EG_STRONG_SQUARE_BONUS[i][j] / VALUE_ONE << (j == 7 ? ",\n" : ", ");
    }
    os << "  0,   0,   0,   0,   0,   0,   0,   0,\n";
    os << "]\n";

    os << "Space Bonus MG: " << MG_SPACE_BONUS / VALUE_ONE << "\n";

    os << "Num Attacker Weight MG: [";
    for(size_t i = 0; i < 5; i++)
        os << std::setw(3) << MG_NUM_ATTACKER_WEIGHT[i] / VALUE_ONE << (i == 4 ? "]\n" : ", ");

    os << "Num Attacker Weight EG: [";
    for(size_t i = 0; i < 5; i++)
        os << std::setw(3) << EG_NUM_ATTACKER_WEIGHT[i] / VALUE_ONE << (i == 4 ? "]\n" : ", ");

    os << "Knight Attack Bonus MG: " << MG_KNIGHT_ATTACK_BONUS / VALUE_ONE << "\n";
    os << "Knight Attack Bonus EG: " << EG_KNIGHT_ATTACK_BONUS / VALUE_ONE << "\n";
    
    os << "Bishop Attack Bonus MG: " << MG_BISHOP_ATTACK_BONUS / VALUE_ONE << "\n";
    os << "Bishop Attack Bonus EG: " << EG_BISHOP_ATTACK_BONUS / VALUE_ONE << "\n";

    os << "Rook Attack Bonus MG: " << MG_ROOK_ATTACK_BONUS / VALUE_ONE << "\n";
    os << "Rook Attack Bonus EG: " << EG_ROOK_ATTACK_BONUS / VALUE_ONE << "\n";

    os << "Queen Attack Bonus MG: " << MG_QUEEN_ATTACK_BONUS / VALUE_ONE << "\n";
    os << "Queen Attack Bonus EG: " << EG_QUEEN_ATTACK_BONUS / VALUE_ONE << "\n";

    os << "Minor Piece Defense Bonus MG: " << MG_MINOR_PIECE_DEFENDER_BONUS / VALUE_ONE << "\n";
    os << "Minor Piece Defense Bonus EG: " << EG_MINOR_PIECE_DEFENDER_BONUS / VALUE_ONE << "\n";

    os << "Pawn Shield Size Bonus MG: [";
    for(size_t i = 0; i < 4; i++)
        os << std::setw(3) << MG_PAWN_SHIELD_SIZE_BONUS[i] / VALUE_ONE << (i == 3 ? "]\n" : ", ");

    os << "Pawn Shield Size Bonus EG: [";
    for(size_t i = 0; i < 4; i++)
        os << std::setw(3) << EG_PAWN_SHIELD_SIZE_BONUS[i] / VALUE_ONE << (i == 3 ? "]\n" : ", ");

    os << "King Open File Penalty MG: [";
    for(size_t i = 0; i < 4; i++)
        os << std::setw(4) << MG_KING_OPEN_FILE_PENALTY[i] / VALUE_ONE << (i == 3 ? "]\n" : ", ");

    os << "King Open File Penalty EG: [";
    for(size_t i = 0; i < 4; i++)
        os << std::setw(4) << EG_KING_OPEN_FILE_PENALTY[i] / VALUE_ONE << (i == 3 ? "]\n" : ", ");

    os << "Pawn Storm Bonus MG: [  0, ";
    for(size_t i = 0; i < 6; i++)
        os << std::setw(3) << MG_PAWN_STORM_BONUS[i] / VALUE_ONE << ", ";

    os << "  0]\n";

    os << "Pawn Storm Bonus EG: [  0, ";
    for(size_t i = 0; i < 6; i++)
        os << std::setw(3) << EG_PAWN_STORM_BONUS[i] / VALUE_ONE << ", ";

    os << "  0]\n";

    os << "Piece Mobility Bonus MG: [  0, ";
    for(size_t i = 0; i < 4; i++)
        os << std::setw(3) << MG_PIECE_MOBILITY_BONUS[i] / VALUE_ONE << (i == 3 ? "]\n" : ", ");

    os << "Piece Mobility Bonus EG: [  0, ";
    for(size_t i = 0; i < 4; i++)
        os << std::setw(3) << EG_PIECE_MOBILITY_BONUS[i] / VALUE_ONE << (i == 3 ? "]\n" : ", ");

    os << "Minor Piece On Strong Square Bonus MG: " << MG_MINOR_PIECE_ON_STRONG_SQUARE_BONUS / VALUE_ONE << "\n";
    os << "Minor Piece On Strong Square Bonus EG: " << EG_MINOR_PIECE_ON_STRONG_SQUARE_BONUS / VALUE_ONE << "\n";

    os << "Bishop Pair Bonus MG: " << MG_BISHOP_PAIR_BONUS / VALUE_ONE << "\n";
    os << "Bishop Pair Bonus EG: " << EG_BISHOP_PAIR_BONUS / VALUE_ONE << "\n";

    os << "Rook On Open File Bonus MG: " << MG_ROOK_ON_OPEN_FILE_BONUS / VALUE_ONE << "\n";
    os << "Rook On Open File Bonus EG: " << EG_ROOK_ON_OPEN_FILE_BONUS / VALUE_ONE << "\n";

    os << "Rook On Semi-Open File Bonus MG: " << MG_ROOK_ON_SEMI_OPEN_FILE_BONUS / VALUE_ONE << "\n";
    os << "Rook On Semi-Open File Bonus EG: " << EG_ROOK_ON_SEMI_OPEN_FILE_BONUS / VALUE_ONE << "\n";

    os << "Rook behind Passed Pawn Bonus MG: " << MG_ROOK_BEHIND_PASSED_PAWN_BONUS / VALUE_ONE << "\n";
    os << "Rook behind Passed Pawn Bonus EG: " << EG_ROOK_BEHIND_PASSED_PAWN_BONUS / VALUE_ONE << "\n";

    os << "King Pawn Proximity Weight: " << EG_KING_PROXIMITY_PAWN_WEIGHT / VALUE_ONE << "\n";
    os << "King Backward Pawn Proximity Weight: " << EG_KING_PROXIMITY_BACKWARD_PAWN_WEIGHT / VALUE_ONE << "\n";
    os << "King Passed Pawn Proximity Weight: " << EG_KING_PROXIMITY_PASSED_PAWN_WEIGHT / VALUE_ONE << std::endl;
}