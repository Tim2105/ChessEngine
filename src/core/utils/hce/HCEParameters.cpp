#include "core/utils/hce/HCEParameters.h"
#include "core/utils/hce/HCEData.h"

#include <cstring>
#include <iomanip>
#include <streambuf>

HCEParameters HCE_PARAMS;

struct membuf : std::streambuf {
    membuf(char* begin, char* end) {
        this->setg(begin, begin, end);
    }
};

void HCEParameters::unpackPSQT() {
    // Die Positionstabellen der Bauern sind vollständig
    // vorhanden und müssen nicht entpackt werden.
    memcpy(mgPSQT[0], mgPSQTPawn, 64 * sizeof(int16_t));
    memcpy(egPSQT[0], egPSQTPawn, 64 * sizeof(int16_t));

    // Die Positionstabellen aller anderen Figuren müssen
    // entlang der vertikalen Achse gespiegelt werden.
    for(size_t i = 0; i < 5; i++) {
        for(size_t j = 0; j < 32; j++) {
            size_t idx = j + (j >> 2) * 4;
            mgPSQT[i + 1][idx] = mgPSQT[i + 1][idx ^ 7] = mgPSQTPacked[i][j];
            egPSQT[i + 1][idx] = egPSQT[i + 1][idx ^ 7] = egPSQTPacked[i][j];
        }
    }
}

HCEParameters::HCEParameters() {
    #if defined(USE_HCE)
        membuf buf(_binary_resources_params_hce_start, _binary_resources_params_hce_end);
        std::istream is(&buf);

        loadParameters(is);
    #endif

    // unpackPSQT();
}

HCEParameters::HCEParameters(std::istream& is) {
    loadParameters(is);
}

HCEParameters::~HCEParameters() {}

void HCEParameters::loadParameters(std::istream& is) {
    is.read((char*)this, size() * sizeof(int16_t));
    unpackPSQT();
}

void HCEParameters::saveParameters(std::ostream& os) const {
    os.write((char*)this, size() * sizeof(int16_t));
}

int16_t& HCEParameters::operator[](size_t index) {
    return ((int16_t*)this)[index];
}

int16_t HCEParameters::operator[](size_t index) const {
    return ((int16_t*)this)[index];
}

size_t HCEParameters::indexOf(int16_t* ptr) const {
    return (size_t)(ptr - (int16_t*)this);
}

bool HCEParameters::isParameterDead(size_t index) const {
    void* mgPSQTPawnFirstRowStart = (void*)&mgPSQTPawn[0];
    void* mgPSQTPawnFirstRowEnd = (void*)((char*)mgPSQTPawnFirstRowStart + 8 * sizeof(int16_t));
    void* mgPSQTPawnLastRowStart = (void*)&mgPSQTPawn[56];
    void* mgPSQTPawnLastRowEnd = (void*)((char*)mgPSQTPawnLastRowStart + 8 * sizeof(int16_t));

    void* egPSQTPawnFirstRowStart = (void*)&egPSQTPawn[0];
    void* egPSQTPawnFirstRowEnd = (void*)((char*)egPSQTPawnFirstRowStart + 8 * sizeof(int16_t));
    void* egPSQTPawnLastRowStart = (void*)&egPSQTPawn[56];
    void* egPSQTPawnLastRowEnd = (void*)((char*)egPSQTPawnLastRowStart + 8 * sizeof(int16_t));

    void* idxAddress = (void*)&((int16_t*)this)[index];

    return (idxAddress >= mgPSQTPawnFirstRowStart && idxAddress < mgPSQTPawnFirstRowEnd) ||
            (idxAddress >= mgPSQTPawnLastRowStart && idxAddress < mgPSQTPawnLastRowEnd) ||
            (idxAddress >= egPSQTPawnFirstRowStart && idxAddress < egPSQTPawnFirstRowEnd) ||
            (idxAddress >= egPSQTPawnLastRowStart && idxAddress < egPSQTPawnLastRowEnd);
}

bool HCEParameters::isOptimizable(size_t index) const {
    return !isParameterDead(index);
}

void HCEParameters::displayParameters(std::ostream& os) const {
    os << "Linear Piece Values: [";
    for(size_t i = 0; i < 5; i++)
        os << std::setw(4) << pieceValues[i] << (i == 4 ? "]\n" : ", ");

    os << "Quadratic Piece Values: [";
    for(size_t i = 0; i < 5; i++)
        os << std::setw(4) << pieceValues[i + 5] << (i == 4 ? "]\n" : ", ");

    os << "Crossed Piece Values: [\n";
    for(size_t i = 1; i < 5; i++)
        for(size_t j = 0; j < i; j++)
            os << std::setw(4) << pieceValues[10 + i * (i - 1) / 2 + j] << (j == i - 1 ? "\n" : ", ");

    os << "]\n";

    os << "Piece Imbalance Values: [\n";
    for(size_t i = 1; i < 5; i++)
        for(size_t j = 0; j < i; j++)
            os << std::setw(4) << pieceImbalanceValues[i * (i - 1) / 2 + j] << (j == i - 1 ? "\n" : ", ");

    os << "]\n";

    os << "Pawn PSQT MG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 8; j++)
            os << std::setw(3) << mgPSQT[0][i * 8 + j] << (j == 7 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "Pawn PSQT EG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 8; j++)
            os << std::setw(3) << egPSQT[0][i * 8 + j] << (j == 7 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "Knight PSQT MG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 8; j++)
            os << std::setw(3) << mgPSQT[1][i * 8 + j] << (j == 7 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "Knight PSQT EG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 8; j++)
            os << std::setw(3) << egPSQT[1][i * 8 + j] << (j == 7 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "Bishop PSQT MG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 8; j++)
            os << std::setw(3) << mgPSQT[2][i * 8 + j] << (j == 7 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "Bishop PSQT EG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 8; j++)
            os << std::setw(3) << egPSQT[2][i * 8 + j] << (j == 7 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "Rook PSQT MG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 8; j++)
            os << std::setw(3) << mgPSQT[3][i * 8 + j] << (j == 7 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "Rook PSQT EG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 8; j++)
            os << std::setw(3) << egPSQT[3][i * 8 + j] << (j == 7 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "Queen PSQT MG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 8; j++)
            os << std::setw(3) << mgPSQT[4][i * 8 + j] << (j == 7 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "Queen PSQT EG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 8; j++)
            os << std::setw(3) << egPSQT[4][i * 8 + j] << (j == 7 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "King PSQT MG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 8; j++)
            os << std::setw(3) << mgPSQT[5][i * 8 + j] << (j == 7 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "King PSQT EG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 8; j++)
            os << std::setw(3) << egPSQT[5][i * 8 + j] << (j == 7 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "Tempo Bonus MG: " << mgTempoBonus << "\n";
    os << "Tempo Bonus EG: " << egTempoBonus << "\n";

    os << "Attack By Minor Piece Bonus MG: [";
    for(size_t i = 0; i < 5; i++)
        os << std::setw(3) << mgAttackByMinorPieceBonus[i] << (i == 4 ? "]\n" : ", ");

    os << "Attack By Minor Piece Bonus EG: [";
    for(size_t i = 0; i < 5; i++)
        os << std::setw(3) << egAttackByMinorPieceBonus[i] << (i == 4 ? "]\n" : ", ");

    os << "Attack By Rook Bonus MG: [";
    for(size_t i = 0; i < 5; i++)
        os << std::setw(3) << mgAttackByRookBonus[i] << (i == 4 ? "]\n" : ", ");

    os << "Attack By Rook Bonus EG: [";
    for(size_t i = 0; i < 5; i++)
        os << std::setw(3) << egAttackByRookBonus[i] << (i == 4 ? "]\n" : ", ");

    os << "Connected Pawn Bonus MG: " << "[  0, ";
    for(size_t i = 0; i < 6; i++)
        os << std::setw(3) << mgConnectedPawnBonus[i] << ", ";

    os << "  0]\n";

    os << "Connected Pawn Bonus EG: " << "[  0, ";
    for(size_t i = 0; i < 6; i++)
        os << std::setw(3) << egConnectedPawnBonus[i] << ", ";

    os << "  0]\n";

    os << "Doubled Pawn Penalty MG: " << mgDoubledPawnPenalty << "\n";
    os << "Doubled Pawn Penalty EG: " << egDoubledPawnPenalty << "\n";
    os << "Isolated Pawn Penalty MG: " << mgIsolatedPawnPenalty << "\n";
    os << "Isolated Pawn Penalty EG: " << egIsolatedPawnPenalty << "\n";
    os << "Backward Pawn Penalty MG: " << mgBackwardPawnPenalty << "\n";
    os << "Backward Pawn Penalty EG: " << egBackwardPawnPenalty << "\n";

    os << "Passed Pawn Bonus MG: [  0, ";
    for(size_t i = 0; i < 6; i++)
        os << std::setw(3) << mgPassedPawnBonus[i] << ", ";

    os << "  0]\n";

    os << "Passed Pawn Bonus EG: [  0, ";
    for(size_t i = 0; i < 6; i++)
        os << std::setw(3) << egPassedPawnBonus[i] << ", ";

    os << "  0]\n";

    os << "Candidate Passed Pawn Bonus MG: [  0, ";
    for(size_t i = 0; i < 5; i++)
        os << std::setw(3) << mgCandidatePassedPawnBonus[i] << ", ";

    os << "  0,   0]\n";

    os << "Candidate Passed Pawn Bonus EG: [  0, ";
    for(size_t i = 0; i < 5; i++)
        os << std::setw(3) << egCandidatePassedPawnBonus[i] << ", ";

    os << "  0,   0]\n";

    os << "Connected Passed Pawn Bonus MG: [  0, ";
    for(size_t i = 0; i < 6; i++)
        os << std::setw(3) << mgConnectedPassedPawnBonus[i] << ", ";

    os << "  0]\n";

    os << "Connected Passed Pawn Bonus EG: [  0, ";
    for(size_t i = 0; i < 6; i++)
        os << std::setw(3) << egConnectedPassedPawnBonus[i] << ", ";

    os << "  0]\n";

    os << "Strong Square Bonus MG: " << mgStrongSquareBonus << "\n";

    os << "Space Bonus MG: " << mgSpaceBonus << "\n";

    os << "Num Attacker Weight: [";
    for(size_t i = 0; i < 9; i++)
        os << std::setw(3) << numAttackerWeight[i] << (i == 8 ? "]\n" : ", ");

    os << "Knight Attack Bonus: " << knightAttackBonus << "\n";
    os << "Bishop Attack Bonus: " << bishopAttackBonus << "\n";
    os << "Rook Attack Bonus: " << rookAttackBonus << "\n";
    os << "Queen Attack Bonus: " << queenAttackBonus << "\n";

    os << "Pawn Shield Size Bonus MG: [  0, ";
    for(size_t i = 0; i < 3; i++)
        os << std::setw(3) << mgPawnShieldSizeBonus[i] << (i == 2 ? "]\n" : ", ");

    os << "King Open File Penalty MG: [  0, ";
    for(size_t i = 0; i < 3; i++)
        os << std::setw(3) << mgKingOpenFilePenalty[i] << (i == 2 ? "]\n" : ", ");

    os << "Pawn Storm Bonus MG: [  0, ";
    for(size_t i = 0; i < 6; i++)
        os << std::setw(3) << mgPawnStormPenalty[i] << ", ";

    os << "  0]\n";

    os << "Piece Mobility Bonus MG: [  0, ";
    for(size_t i = 0; i < 4; i++)
        os << std::setw(3) << mgPieceMobilityBonus[i] << (i == 3 ? "]\n" : ", ");

    os << "Piece Mobility Bonus EG: [  0, ";
    for(size_t i = 0; i < 4; i++)
        os << std::setw(3) << egPieceMobilityBonus[i] << (i == 3 ? "]\n" : ", ");

    os << "Piece No Mobility Penalty MG: [  0, ";
    for(size_t i = 0; i < 4; i++)
        os << std::setw(3) << mgPieceNoMobilityPenalty[i] << (i == 3 ? "]\n" : ", ");

    os << "Piece No Mobility Penalty EG: [  0, ";
    for(size_t i = 0; i < 4; i++)
        os << std::setw(3) << egPieceNoMobilityPenalty[i] << (i == 3 ? "]\n" : ", ");

    os << "Knight On Strong Square Bonus MG: " << mgKnightOnStrongSquareBonus << "\n";
    os << "Bishop On Strong Square Bonus MG: " << mgBishopOnStrongSquareBonus << "\n";

    os << "Bad Bishop Penalty MG: " << mgBadBishopPenalty << "\n";
    os << "Bad Bishop Penalty EG: " << egBadBishopPenalty << "\n";

    os << "Rook On Open File Bonus MG: " << mgRookOnOpenFileBonus << "\n";
    os << "Rook On Semi-Open File Bonus MG: " << mgRookOnSemiOpenFileBonus << "\n";
    os << "Rook behind Passed Pawn Bonus EG: " << egRookBehindPassedPawnBonus << "\n";

    os << "Enemy Passed Pawn Blocked Bonus EG: " << egBlockedEnemyPassedPawnBonus << "\n";

    os << "King Pawn Proximity Weight: " << egKingProximityPawnWeight << "\n";
    os << "King Backward Pawn Proximity Weight: " << egKingProximityBackwardPawnWeight << "\n";
    os << "King Candidate Passed Pawn Proximity Weight: " << egKingProximityCandidatePassedPawnWeight << "\n";
    os << "King Passed Pawn Proximity Weight: " << egKingProximityPassedPawnWeight << "\n";

    os << "Rule Of The Square Bonus: " << ruleOfTheSquareBonus << std::endl;
}