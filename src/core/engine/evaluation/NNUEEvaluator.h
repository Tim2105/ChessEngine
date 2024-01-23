#ifndef NNUE_EVALUATOR_H
#define NNUE_EVALUATOR_H

#include "core/chess/Referee.h"
#include "core/engine/evaluation/EndgameEvaluator.h"
#include "core/engine/evaluation/SimpleUpdatedEvaluator.h"
#include "core/utils/nnue/NNUENetwork.h"

class NNUEEvaluator: public Evaluator {

    private:
        NNUE::Network network;
        EndgameEvaluator endgameEvaluator;

        int32_t materialDifference = 0;
        int32_t pieceWeight = 0;
        double gamePhase = 0.0;

    public:
        NNUEEvaluator(Board& board) : Evaluator(board), endgameEvaluator(board) {
            initializeMaterialDifference();
            initializeGamePhase();
        }

        NNUEEvaluator(Board& board, std::istream& networkStream) : Evaluator(board), endgameEvaluator(board) {
            networkStream >> network;
            network.initializeFromBoard(board);
            initializeMaterialDifference();
            initializeGamePhase();
        }

        ~NNUEEvaluator() {}

        inline int32_t evaluate() override {
            int32_t numPawns = b->getPieceBitboard(WHITE | PAWN).getNumberOfSetBits() +
                               b->getPieceBitboard(BLACK | PAWN).getNumberOfSetBits();

            if(numPawns == 0 && std::abs(materialDifference) <= 300)
                return 0;

            if(gamePhase >= 1.0 && std::abs(materialDifference) >= 400)
                return endgameEvaluator.evaluate() * STATIC_EVAL_MULTIPLIER;

            return network.evaluate(b->getSideToMove());
        }

        inline void updateBeforeMove(Move move) override {
            int32_t capturedPiece = b->pieceAt(move.getDestination());
            int32_t promotedPieceType = EMPTY;

            materialDifference -= SIMPLE_PIECE_VALUES[capturedPiece];

            if(move.isPromotion()) {
                if(move.isPromotionQueen())
                    promotedPieceType = QUEEN;
                else if(move.isPromotionRook())
                    promotedPieceType = ROOK;
                else if(move.isPromotionBishop())
                    promotedPieceType = BISHOP;
                else
                    promotedPieceType = KNIGHT;
            } else if(move.isEnPassant())
                capturedPiece = PAWN;

            pieceWeight += PIECE_WEIGHTS[TYPEOF(capturedPiece)];
            pieceWeight -= PIECE_WEIGHTS[promotedPieceType] - PAWN_WEIGHT;

            gamePhase = (double)pieceWeight / TOTAL_WEIGHT;
            gamePhase = gamePhase * (MAX_PHASE - MIN_PHASE) + MIN_PHASE; // phase in [MIN_PHASE, MAX_PHASE]
            gamePhase = std::clamp(gamePhase, 0.0, 1.0); // phase auf [0, 1] begrenzen
        }

        inline void updateAfterMove() override {
            network.updateAfterMove(*b);
        }

        inline void updateBeforeUndo() override {
            network.undoMove();
        }

        inline void updateAfterUndo(Move move) override {
            int32_t capturedPiece = b->pieceAt(move.getDestination());
            int32_t promotedPieceType = EMPTY;

            materialDifference += SIMPLE_PIECE_VALUES[capturedPiece];

            if(move.isPromotion()) {
                if(move.isPromotionQueen())
                    promotedPieceType = QUEEN;
                else if(move.isPromotionRook())
                    promotedPieceType = ROOK;
                else if(move.isPromotionBishop())
                    promotedPieceType = BISHOP;
                else
                    promotedPieceType = KNIGHT;
            } else if(move.isEnPassant())
                capturedPiece = PAWN;

            pieceWeight -= PIECE_WEIGHTS[TYPEOF(capturedPiece)];
            pieceWeight += PIECE_WEIGHTS[promotedPieceType] - PAWN_WEIGHT;

            gamePhase = (double)pieceWeight / TOTAL_WEIGHT;
            gamePhase = gamePhase * (MAX_PHASE - MIN_PHASE) + MIN_PHASE; // phase in [MIN_PHASE, MAX_PHASE]
            gamePhase = std::clamp(gamePhase, 0.0, 1.0); // phase auf [0, 1] begrenzen
        }

        inline void setBoard(Board& board) override {
            Evaluator::setBoard(board);
            endgameEvaluator.setBoard(board);
            network.clearPastAccumulators();
            network.initializeFromBoard(board);
            initializeMaterialDifference();
            initializeGamePhase();
        }

        friend std::istream& operator>>(std::istream& is, NNUEEvaluator& evaluator) {
            is >> evaluator.network;
            return is;
        }

    private:
        static constexpr int32_t SIMPLE_PIECE_VALUES[] = {0, 100, 300, 300, 500, 900, 0, 0,
                                                          0, -100, -300, -300, -500, -900, 0};

        static constexpr double STATIC_EVAL_MULTIPLIER = 2.0;

        // Figurengewichte
        static constexpr int32_t PAWN_WEIGHT = 0;
        static constexpr int32_t KNIGHT_WEIGHT = 1;
        static constexpr int32_t BISHOP_WEIGHT = 1;
        static constexpr int32_t ROOK_WEIGHT = 2;
        static constexpr int32_t QUEEN_WEIGHT = 4;
        static constexpr int32_t PIECE_WEIGHTS[] = {0, PAWN_WEIGHT, KNIGHT_WEIGHT, BISHOP_WEIGHT, ROOK_WEIGHT, QUEEN_WEIGHT, 0};

        // Phasengrenzen, können unter 0 oder über 1 sein,
        // die berechnete Phase wird aber zwischen 0 und 1 eingeschränkt
        static constexpr double MIN_PHASE = -0.5;
        static constexpr double MAX_PHASE = 1.5;

        static constexpr double TOTAL_WEIGHT = PAWN_WEIGHT * 16 + KNIGHT_WEIGHT * 4 + BISHOP_WEIGHT * 4 + ROOK_WEIGHT * 4 + QUEEN_WEIGHT * 2;

        inline void initializeMaterialDifference() {
            materialDifference = 0;

            for(int32_t p = (WHITE | PAWN); p <= (WHITE | QUEEN); p++)
                materialDifference += SIMPLE_PIECE_VALUES[p] * b->getPieceBitboard(p).getNumberOfSetBits();

            for(int32_t p = (BLACK | PAWN); p <= (BLACK | QUEEN); p++)
                materialDifference += SIMPLE_PIECE_VALUES[p] * b->getPieceBitboard(p).getNumberOfSetBits();
        }

        inline void initializeGamePhase() {
            pieceWeight = TOTAL_WEIGHT;

            for(int32_t p = (WHITE | PAWN); p <= (WHITE | QUEEN); p++)
                pieceWeight -= PIECE_WEIGHTS[TYPEOF(p)] * b->getPieceBitboard(p).getNumberOfSetBits();

            for(int32_t p = (BLACK | PAWN); p <= (BLACK | QUEEN); p++)
                pieceWeight -= PIECE_WEIGHTS[TYPEOF(p)] * b->getPieceBitboard(p).getNumberOfSetBits();

            gamePhase = (double)pieceWeight / TOTAL_WEIGHT;
            gamePhase = gamePhase * (MAX_PHASE - MIN_PHASE) + MIN_PHASE;
            gamePhase = std::clamp(gamePhase, 0.0, 1.0);
        }
};

#endif