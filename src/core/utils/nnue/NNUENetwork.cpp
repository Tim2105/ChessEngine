#include "core/utils/nnue/NNUENetwork.h"

#include <iomanip>
#include <sstream>

using namespace NNUE;

Network::Network() {
    halfKPLayer = new HalfKPLayer;
    accumulator.setLayer(halfKPLayer);
    pastAccumulators.reserve(128);
}

Network::~Network() {
    delete halfKPLayer;
}

int32_t Network::evaluate(int32_t color) const noexcept {
    int8_t kpActivationOutput[512];
    int32_t layer1Output[32];
    int8_t activation1Output[32];
    int32_t layer2Output[32];
    int8_t activation2Output[32];
    int32_t output[1];

    halfKPActivation.forward(accumulator.getOutput(color), kpActivationOutput);
    halfKPActivation.forward(accumulator.getOutput(color ^ COLOR_MASK), kpActivationOutput + SINGLE_SUBNET_SIZE);
    layer1.forward(kpActivationOutput, layer1Output);
    activation1.forward(layer1Output, activation1Output);
    layer2.forward(activation1Output, layer2Output);
    activation2.forward(layer2Output, activation2Output);
    layer3.forward(activation2Output, output);

    return output[0] * 100 / 3328;
}

constexpr int32_t rotate(int32_t square) noexcept {
    return square ^ 63;
}

int32_t KP_INDICES[7][2] = {
    {0, 0},
    {1, 65},
    {129, 193},
    {257, 321},
    {385, 449},
    {513, 577},
    {641, 705}
};

template <int32_t COLOR>
int32_t getHalfKPIndex(int32_t kingSq, int32_t pieceSq, int32_t piece) noexcept {
    int32_t pieceType = TYPEOF(piece);
    int32_t pieceColor = piece & COLOR_MASK;

    if constexpr(COLOR == BLACK) {
        kingSq = rotate(kingSq);
        pieceSq = rotate(pieceSq);
        pieceColor ^= COLOR_MASK;
    }

    return pieceSq + KP_INDICES[pieceType][pieceColor / COLOR_MASK] + kingSq * 641;
}

void Network::initializeFromBoard(const Board& board, int32_t color) noexcept {
    Array<int32_t, 63> activeFeatures;

    int32_t kingSq = board.getKingSquare(color);

    // Weiße Figuren
    if(color == WHITE) {
        for(int32_t piece = (WHITE | PAWN); piece <= (WHITE | QUEEN); piece++) {
            for(int32_t sq : board.getPieceList(piece)) {
                activeFeatures.push_back(getHalfKPIndex<WHITE>(kingSq, sq, piece));
            }
        }

        // Schwarze Figuren
        for(int32_t piece = (BLACK | PAWN); piece <= (BLACK | QUEEN); piece++) {
            for(int32_t sq : board.getPieceList(piece)) {
                activeFeatures.push_back(getHalfKPIndex<WHITE>(kingSq, sq, piece));
            }
        }
    } else {
        for(int32_t piece = (WHITE | PAWN); piece <= (WHITE | QUEEN); piece++) {
            for(int32_t sq : board.getPieceList(piece)) {
                activeFeatures.push_back(getHalfKPIndex<BLACK>(kingSq, sq, piece));
            }
        }

        // Schwarze Figuren
        for(int32_t piece = (BLACK | PAWN); piece <= (BLACK | QUEEN); piece++) {
            for(int32_t sq : board.getPieceList(piece)) {
                activeFeatures.push_back(getHalfKPIndex<BLACK>(kingSq, sq, piece));
            }
        }
    }

    accumulator.refresh(activeFeatures, color);
}

void Network::updateAfterOppKingMove(const Board& board, int32_t color, Move move) noexcept {
    int32_t colorMoved = color ^ COLOR_MASK;

    Array<int32_t, 3> addedFeatures;
    Array<int32_t, 3> removedFeatures;

    if(move.isCapture()) {
        int32_t capturedPiece = board.getLastMoveHistoryEntry().capturedPiece;
        int32_t otherKingSq = board.getKingSquare(colorMoved ^ COLOR_MASK);

        if(colorMoved == WHITE)
            removedFeatures.push_back(getHalfKPIndex<BLACK>(otherKingSq, move.getDestination(), capturedPiece));
        else
            removedFeatures.push_back(getHalfKPIndex<WHITE>(otherKingSq, move.getDestination(), capturedPiece));

        accumulator.update(addedFeatures, removedFeatures, color);
    } else if(move.isCastle()) {
        int32_t kingSq = board.getKingSquare(colorMoved);
        int32_t otherKingSq = board.getKingSquare(colorMoved ^ COLOR_MASK);
        int32_t rookOrigin, rookDestination;

        if(move.isKingsideCastle()) {
            rookOrigin = kingSq + EAST;
            rookDestination = kingSq + WEST;
        } else {
            rookOrigin = kingSq + WEST * 2;
            rookDestination = kingSq + EAST;
        }

        if(colorMoved == WHITE) {
            removedFeatures.push_back(getHalfKPIndex<BLACK>(otherKingSq, rookOrigin, colorMoved | ROOK));
            addedFeatures.push_back(getHalfKPIndex<BLACK>(otherKingSq, rookDestination, colorMoved | ROOK));
        } else {
            removedFeatures.push_back(getHalfKPIndex<WHITE>(otherKingSq, rookOrigin, colorMoved | ROOK));
            addedFeatures.push_back(getHalfKPIndex<WHITE>(otherKingSq, rookDestination, colorMoved | ROOK));
        }

        accumulator.update(addedFeatures, removedFeatures, color);
    }
}

void Network::initializeFromBoard(const Board& board) noexcept {
    Array<int32_t, 63> activeFeaturesWhite;
    Array<int32_t, 63> activeFeaturesBlack;

    // Weiße Figuren
    for(int32_t piece = (WHITE | PAWN); piece <= (WHITE | QUEEN); piece++) {
        for(int32_t sq : board.getPieceList(piece)) {
            activeFeaturesWhite.push_back(getHalfKPIndex<WHITE>(board.getKingSquare(WHITE), sq, piece));
            activeFeaturesBlack.push_back(getHalfKPIndex<BLACK>(board.getKingSquare(BLACK), sq, piece));
        }
    }

    // Schwarze Figuren
    for(int32_t piece = (BLACK | PAWN); piece <= (BLACK | QUEEN); piece++) {
        for(int32_t sq : board.getPieceList(piece)) {
            activeFeaturesWhite.push_back(getHalfKPIndex<WHITE>(board.getKingSquare(WHITE), sq, piece));
            activeFeaturesBlack.push_back(getHalfKPIndex<BLACK>(board.getKingSquare(BLACK), sq, piece));
        }
    }

    accumulator.refresh(activeFeaturesWhite, WHITE);
    accumulator.refresh(activeFeaturesBlack, BLACK);
}

void Network::updateAfterMove(const Board& board) noexcept {
    pastAccumulators.push_back(Array<int16_t, 512>());
    std::copy(accumulator.getOutput(WHITE), accumulator.getOutput(WHITE) + SINGLE_SUBNET_SIZE, pastAccumulators.back().begin());
    std::copy(accumulator.getOutput(BLACK), accumulator.getOutput(BLACK) + SINGLE_SUBNET_SIZE, pastAccumulators.back().begin() + SINGLE_SUBNET_SIZE);

    Move move = board.getLastMove();

    int32_t movedPiece = board.pieceAt(move.getDestination());
    int32_t colorMoved = movedPiece & COLOR_MASK;

    if(TYPEOF(movedPiece) == KING) {
        // Wenn der König gezogen wurde,
        // muss das Netzwerk der Farbe des Königs neu initialisiert werden
        initializeFromBoard(board, colorMoved);
        updateAfterOppKingMove(board, colorMoved ^ COLOR_MASK, move);
        return;
    }

    Array<int32_t, 3> addedFeaturesWhite;
    Array<int32_t, 3> addedFeaturesBlack;
    Array<int32_t, 3> removedFeaturesWhite;
    Array<int32_t, 3> removedFeaturesBlack;

    int32_t whiteKingSq = board.getKingSquare(WHITE);
    int32_t blackKingSq = board.getKingSquare(BLACK);

    // Wenn der Zug eine Promotion ist, wäre die gezogene Figur sonst
    // die aufgewertete Figur
    if(move.isPromotion())
        movedPiece = PAWN | colorMoved;

    // Entferne die gezogene Figur von ihrer Ausgangsposition
    removedFeaturesWhite.push_back(getHalfKPIndex<WHITE>(whiteKingSq, move.getOrigin(), movedPiece));
    removedFeaturesBlack.push_back(getHalfKPIndex<BLACK>(blackKingSq, move.getOrigin(), movedPiece));

    // Wenn die gezogene Figur eine gegnerische Figur geschlagen hat, entferne diese auch
    if(move.isCapture()) {
        // Spezialfall: En-Passant
        if(move.isEnPassant()) {
            int32_t capturedPawnSq = move.getDestination() + (colorMoved == WHITE ? SOUTH : NORTH);
            int32_t capturedPiece = colorMoved == WHITE ? BLACK | PAWN : WHITE | PAWN;

            removedFeaturesWhite.push_back(getHalfKPIndex<WHITE>(whiteKingSq, capturedPawnSq, capturedPiece));
            removedFeaturesBlack.push_back(getHalfKPIndex<BLACK>(blackKingSq, capturedPawnSq, capturedPiece));
        } else {
            int32_t capturedPiece = board.getLastMoveHistoryEntry().capturedPiece;

            removedFeaturesWhite.push_back(getHalfKPIndex<WHITE>(whiteKingSq, move.getDestination(), capturedPiece));
            removedFeaturesBlack.push_back(getHalfKPIndex<BLACK>(blackKingSq, move.getDestination(), capturedPiece));
        }
    }

    // Füge die gezogene Figur an ihre neue Position ein

    // Spezialfall: Promotion
    if(move.isPromotion()) {
        int32_t promotedPiece = QUEEN;
        if(move.isPromotionRook())
            promotedPiece = ROOK;
        else if(move.isPromotionBishop())
            promotedPiece = BISHOP;
        else if(move.isPromotionKnight())
            promotedPiece = KNIGHT;

        promotedPiece |= colorMoved;

        addedFeaturesWhite.push_back(getHalfKPIndex<WHITE>(whiteKingSq, move.getDestination(), promotedPiece));
        addedFeaturesBlack.push_back(getHalfKPIndex<BLACK>(blackKingSq, move.getDestination(), promotedPiece));
    } else {
        addedFeaturesWhite.push_back(getHalfKPIndex<WHITE>(whiteKingSq, move.getDestination(), movedPiece));
        addedFeaturesBlack.push_back(getHalfKPIndex<BLACK>(blackKingSq, move.getDestination(), movedPiece));
    }

    accumulator.update(addedFeaturesWhite, removedFeaturesWhite, WHITE);
    accumulator.update(addedFeaturesBlack, removedFeaturesBlack, BLACK);
}

void Network::undoMove() noexcept {
    accumulator.setOutput(WHITE, pastAccumulators.back().begin());
    accumulator.setOutput(BLACK, pastAccumulators.back().begin() + SINGLE_SUBNET_SIZE);

    pastAccumulators.pop_back();
}

namespace NNUE {
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