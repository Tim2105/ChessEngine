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
        for(size_t j = 0; j < 4; j++)
            os << std::setw(3) << mgPSQT[1][i * 8 + j] << (j == 3 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "Knight PSQT EG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 4; j++)
            os << std::setw(3) << egPSQT[1][i * 8 + j] << (j == 3 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "Bishop PSQT MG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 4; j++)
            os << std::setw(3) << mgPSQT[2][i * 8 + j] << (j == 3 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "Bishop PSQT EG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 4; j++)
            os << std::setw(3) << egPSQT[2][i * 8 + j] << (j == 3 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "Rook PSQT MG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 4; j++)
            os << std::setw(3) << mgPSQT[3][i * 8 + j] << (j == 3 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "Rook PSQT EG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 4; j++)
            os << std::setw(3) << egPSQT[3][i * 8 + j] << (j == 3 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "Queen PSQT MG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 4; j++)
            os << std::setw(3) << mgPSQT[4][i * 8 + j] << (j == 3 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "Queen PSQT EG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 4; j++)
            os << std::setw(3) << egPSQT[4][i * 8 + j] << (j == 3 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "King PSQT MG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 4; j++)
            os << std::setw(3) << mgPSQT[5][i * 8 + j] << (j == 3 ? ",\n" : ", ");
    }
    os << "]\n";

    os << "King PSQT EG: [\n";
    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 4; j++)
            os << std::setw(3) << egPSQT[5][i * 8 + j] << (j == 3 ? ",\n" : ", ");
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

    os << "Doubled Pawn Penalty MG: [";
    for(size_t i = 0; i < 8; i++)
        os << std::setw(3) << mgDoubledPawnPenalty[i & 4 ? i ^ 7 : i] << (i == 7 ? "]\n" : ", ");

    os << "Doubled Pawn Penalty EG: [";
    for(size_t i = 0; i < 8; i++)
        os << std::setw(3) << egDoubledPawnPenalty[i & 4 ? i ^ 7 : i] << (i == 7 ? "]\n" : ", ");

    os << "Isolated Pawn Penalty MG: [";
    for(size_t i = 0; i < 8; i++)
        os << std::setw(3) << mgIsolatedPawnPenalty[i & 4 ? i ^ 7 : i] << (i == 7 ? "]\n" : ", ");

    os << "Isolated Pawn Penalty EG: [";
    for(size_t i = 0; i < 8; i++)
        os << std::setw(3) << egIsolatedPawnPenalty[i & 4 ? i ^ 7 : i] << (i == 7 ? "]\n" : ", ");

    os << "Backward Pawn Penalty MG: [  0, ";
    for(size_t i = 0; i < 5; i++)
        os << std::setw(3) << mgBackwardPawnPenalty[i] << ", ";

    os << "  0,   0]\n";

    os << "Backward Pawn Penalty EG: [  0, ";
    for(size_t i = 0; i < 5; i++)
        os << std::setw(3) << egBackwardPawnPenalty[i] << ", ";

    os << "  0,   0]\n";

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

    os << "Center Outpost Bonus MG: " << mgCenterOutpostBonus << "\n";
    os << "Edge Outpost Bonus MG: " << mgEdgeOutpostBonus << "\n";

    os << "Attack Weight MG: [  0, ";
    for(size_t i = 0; i < 4; i++)
        os << std::setw(3) << mgAttackWeight[i] << (i == 3 ? "]\n" : ", ");

    os << "Attack Weight EG: [  0, ";
    for(size_t i = 0; i < 4; i++)
        os << std::setw(3) << egAttackWeight[i] << (i == 3 ? "]\n" : ", ");

    os << "Undefended Attack Weight MG: [  0, ";
    for(size_t i = 0; i < 4; i++)
        os << std::setw(3) << mgUndefendedAttackWeight[i] << (i == 3 ? "]\n" : ", ");

    os << "Undefended Attack Weight EG: [  0, ";
    for(size_t i = 0; i < 4; i++)
        os << std::setw(3) << egUndefendedAttackWeight[i] << (i == 3 ? "]\n" : ", ");

    os << "Safe Check Weight MG: [  0, ";
    for(size_t i = 0; i < 4; i++)
        os << std::setw(3) << mgSafeCheckWeight[i] << (i == 3 ? "]\n" : ", ");

    os << "Safe Check Weight EG: [  0, ";
    for(size_t i = 0; i < 4; i++)
        os << std::setw(3) << egSafeCheckWeight[i] << (i == 3 ? "]\n" : ", ");

    os << "Safe Contact Check Weight MG: [  0,   0,   0, ";
    for(size_t i = 0; i < 2; i++)
        os << std::setw(3) << mgSafeContactCheckWeight[i] << (i == 1 ? "]\n" : ", ");

    os << "Safe Contact Check Weight EG: [  0,   0,   0, ";
    for(size_t i = 0; i < 2; i++)
        os << std::setw(3) << egSafeContactCheckWeight[i] << (i == 1 ? "]\n" : ", ");

    os << "Defense Weight MG: [  0, ";
    for(size_t i = 0; i < 4; i++)
        os << std::setw(3) << mgDefenseWeight[i] << (i == 3 ? "]\n" : ", ");

    os << "Defense Weight EG: [  0, ";
    for(size_t i = 0; i < 4; i++)
        os << std::setw(3) << egDefenseWeight[i] << (i == 3 ? "]\n" : ", ");

    os << "Pawn Shield Size Weight MG: [  0, ";
    for(size_t i = 0; i < 3; i++)
        os << std::setw(3) << mgPawnShieldSizeWeight[i] << (i == 2 ? "]\n" : ", ");

    os << "King Open File Weight MG: [  0, ";
    for(size_t i = 0; i < 3; i++)
        os << std::setw(3) << mgKingOpenFileWeight[i] << (i == 2 ? "]\n" : ", ");

    os << "Pawn Storm Weight MG: [  0, ";
    for(size_t i = 0; i < 5; i++)
        os << std::setw(3) << mgPawnStormWeight[i] << ", ";

    os << "  0,   0]\n";

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

    os << "Knight On Center Outpost Bonus MG: " << mgKnightOnCenterOutpostBonus << "\n";
    os << "Knight On Edge Outpost Bonus MG: " << mgKnightOnEdgeOutpostBonus << "\n";
    os << "Bishop On Center Outpost Bonus MG: " << mgBishopOnCenterOutpostBonus << "\n";
    os << "Bishop On Edge Outpost Bonus MG: " << mgBishopOnEdgeOutpostBonus << "\n";

    os << "Bad Bishop Penalty MG: " << mgBadBishopPenalty << "\n";
    os << "Bad Bishop Penalty EG: " << egBadBishopPenalty << "\n";

    os << "Rook On Open File Bonus MG: " << mgRookOnOpenFileBonus << "\n";
    os << "Rook On Semi-Open File Bonus MG: " << mgRookOnSemiOpenFileBonus << "\n";
    os << "Doubled Rooks On Open File Bonus MG: " << mgDoubledRooksOnOpenFileBonus << "\n";
    os << "Doubled Rooks On Semi-Open File Bonus MG: " << mgDoubledRooksOnSemiOpenFileBonus << "\n";

    os << "Rook behind Passed Pawn Bonus EG: " << egRookBehindPassedPawnBonus << "\n";

    os << "Enemy Passed Pawn Blocked Bonus EG: " << egBlockedEnemyPassedPawnBonus << "\n";

    os << "Attack On Passed Pawn Path Bonus EG: " << egAttackOnPassedPawnPathBonus << "\n";

    os << "King Pawn Proximity Weight: " << egKingProximityPawnWeight << "\n";
    os << "King Backward Pawn Proximity Weight: " << egKingProximityBackwardPawnWeight << "\n";
    os << "King Passed Pawn Proximity Weight: " << egKingProximityPassedPawnWeight << "\n";

    os << "Rule Of The Square Bonus: " << ruleOfTheSquareBonus << "\n";

    os << "Opposite Color Bishops Endgame Winnable Penalty: " << oppositeColorBishopsEndgameWinnablePenalty << "\n";

    os << "Opposite Color Bishops Winnable Penalty MG: " << mgOppositeColorBishopsWinnablePenalty << "\n";
    os << "Opposite Color Bishops Winnable Penalty EG: " << egOppositeColorBishopsWinnablePenalty << "\n";

    os << "Rook Endgame Winnable Penalty: " << rookEndgameWinnablePenalty << "\n";

    os << "Default Winnable Penalty MG: " << mgDefaultWinnablePenalty << "\n";
    os << "Default Winnable Penalty EG: " << egDefaultWinnablePenalty << "\n";

    os << "King and Pawn Endgame Winnable Bonus: " << kingAndPawnEndgameWinnableBonus << "\n";

    os << "Pawn Winnable Bonus MG: " << mgPawnWinnableBonus << "\n";
    os << "Pawn Winnable Bonus EG: " << egPawnWinnableBonus << "\n";

    os << "Passed Pawn Winnable Bonus MG: " << mgPassedPawnWinnableBonus << "\n";
    os << "Passed Pawn Winnable Bonus EG: " << egPassedPawnWinnableBonus << "\n";

    os << "King Infiltration Winnable Bonus: " << kingInfiltrationWinnableBonus << "\n";

    os << "Mopup Base Bonus: " << egMopupBaseBonus << "\n";
    os << "Mopup Progress Bonus: " << egMopupProgressBonus << "\n";

    os << std::endl;
}