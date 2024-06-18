#ifndef NEW_MOVEGEN_H
#define NEW_MOVEGEN_H

#include "core/chess/Board.h"
#include "core/utils/magics/Magics.h"

/**
 * Die Klasse Movegen stellt statische Hilfsfunktionen bereit, um
 * legale (Schlag-)Züge für eine gegebene Stellung zu generieren.
 */
class Movegen {
    struct PrecomputedInfo {
        Bitboard pinnedPieces;
        Bitboard attackingRays;
        const Array<int32_t, 64>& pinDirections;
        int32_t numAttackers;
    };

    public:
        template <int32_t color>
        static void generateLegalMoves(const Board& board, Array<Move, 256>& moves,
                                int32_t numAttackers, Bitboard attackingRays,
                                Bitboard pinnedPieces, const Array<int32_t, 64>& pinDirections) noexcept;

        template <int32_t color>
        static void generateLegalCaptures(const Board& board, Array<Move, 256>& moves,
                                int32_t numAttackers, Bitboard attackingRays,
                                Bitboard pinnedPieces, const Array<int32_t, 64>& pinDirections) noexcept;

    private:
        template <int32_t color, bool checkEnPassant>
        static inline void generatePawnMoves(const Board& board, Array<Move, 256>& moves, const PrecomputedInfo& precomputedInfo) noexcept;

        template <int32_t color>
        static inline void generateKnightMoves(const Board& board, Array<Move, 256>& moves, const PrecomputedInfo& precomputedInfo) noexcept;

        template <int32_t color>
        static inline void generateDiagonalSlidingMoves(const Board& board, Array<Move, 256>& moves, const PrecomputedInfo& precomputedInfo) noexcept;

        template <int32_t color>
        static inline void generateHorizontalSlidingMoves(const Board& board, Array<Move, 256>& moves, const PrecomputedInfo& precomputedInfo) noexcept;

        template <int32_t color>
        static inline void generateKingMoves(const Board& board, Array<Move, 256>& moves) noexcept;

        template <int32_t color, bool checkEnPassant>
        static inline void generatePawnCaptures(const Board& board, Array<Move, 256>& moves, const PrecomputedInfo& precomputedInfo) noexcept;

        template <int32_t color>
        static inline void generateKnightCaptures(const Board& board, Array<Move, 256>& moves, const PrecomputedInfo& precomputedInfo) noexcept;

        template <int32_t color>
        static inline void generateDiagonalSlidingCaptures(const Board& board, Array<Move, 256>& moves, const PrecomputedInfo& precomputedInfo) noexcept;

        template <int32_t color>
        static inline void generateHorizontalSlidingCaptures(const Board& board, Array<Move, 256>& moves, const PrecomputedInfo& precomputedInfo) noexcept;

        template <int32_t color>
        static inline void generateKingCaptures(const Board& board, Array<Move, 256>& moves) noexcept;
};

template <int32_t color>
void Movegen::generateLegalMoves(const Board& board, Array<Move, 256>& moves,
                                 int32_t numAttackers, Bitboard attackingRays,
                                 Bitboard pinnedPieces, const Array<int32_t, 64>& pinDirections) noexcept {

    PrecomputedInfo info = {pinnedPieces, attackingRays, pinDirections, numAttackers};

    if(numAttackers < 2) {
        bool checkEnPassant = board.getEnPassantSquare() != NO_SQ;
        if(checkEnPassant)
            generatePawnMoves<color, true>(board, moves, info);
        else
            generatePawnMoves<color, false>(board, moves, info);
        
        generateKnightMoves<color>(board, moves, info);
        generateDiagonalSlidingMoves<color>(board, moves, info);
        generateHorizontalSlidingMoves<color>(board, moves, info);
    }

    generateKingMoves<color>(board, moves);
}

template <int32_t color>
void Movegen::generateLegalCaptures(const Board& board, Array<Move, 256>& moves,
                                    int32_t numAttackers, Bitboard attackingRays,
                                    Bitboard pinnedPieces, const Array<int32_t, 64>& pinDirections) noexcept {

    PrecomputedInfo info = {pinnedPieces, attackingRays, pinDirections, numAttackers};

    if(numAttackers < 2) {
        bool checkEnPassant = board.getEnPassantSquare() != NO_SQ;
        if(checkEnPassant)
            generatePawnCaptures<color, true>(board, moves, info);
        else
            generatePawnCaptures<color, true>(board, moves, info);
        
        generateKnightCaptures<color>(board, moves, info);
        generateDiagonalSlidingCaptures<color>(board, moves, info);
        generateHorizontalSlidingCaptures<color>(board, moves, info);
    }

    generateKingCaptures<color>(board, moves);
}

template <int32_t color, bool checkEnPassant>
inline void Movegen::generatePawnMoves(const Board& board, Array<Move, 256>& moves, const PrecomputedInfo& precomputedInfo) noexcept {
    constexpr int32_t pawnPiece = color == WHITE ? WHITE_PAWN : BLACK_PAWN;
    constexpr int32_t forw = color == WHITE ? NORTH : SOUTH;
    constexpr int32_t forw2 = color == WHITE ? 2 * NORTH : 2 * SOUTH;
    constexpr int32_t startingRank = color == WHITE ? RANK_2 : RANK_7;
    constexpr int32_t promotionRank = color == WHITE ? RANK_7 : RANK_2;
    constexpr int32_t ownColor = color == WHITE ? WHITE : BLACK;
    constexpr int32_t enPasRankReq = color == WHITE ? RANK_5 : RANK_4;

    Bitboard enemyPieces = color == WHITE ? board.pieceBitboard[BLACK] : board.pieceBitboard[WHITE];
    
    Bitboard pawns = board.getPieceBitboard(pawnPiece);

    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Bauer den Angreifer schlagen oder sich dazwischen stellen
    if(precomputedInfo.numAttackers == 1) {
        while(pawns) {
            int32_t sq = pawns.popFSB();
            int32_t rank = SQ2R(sq);
            int32_t forwSq = sq + forw;
            int32_t forw2Sq = sq + forw2;

            // Wenn der Bauer gefesselt ist, kann er sich nicht bewegen
            if(precomputedInfo.pinnedPieces.getBit(sq))
                continue;

            if constexpr(checkEnPassant) {
                // En-Passant könnte den Angreifer blockieren oder schlagen
                if(rank == enPasRankReq) {
                    int32_t captureSq = board.getEnPassantSquare();
                    int32_t enPasSq = captureSq - forw;
                    Bitboard captureSqBB = Bitboard(1ULL << captureSq);
                    Bitboard enPasSqBB = Bitboard(1ULL << enPasSq);

                    if((pawnAttackBitboard(sq, ownColor) & captureSqBB) &&
                    (captureSqBB | enPasSqBB) & precomputedInfo.attackingRays)
                        moves.push_back(Move(sq, captureSq, MOVE_EN_PASSANT));
                }
            }

            // Der Bauer könnte sich dazwischen stellen
            if(precomputedInfo.attackingRays.getBit(forwSq) &&
               board.pieces[forwSq] == EMPTY) {

                if(rank == promotionRank) {
                    moves.push_back(Move(sq, forwSq, MOVE_PROMOTION_QUEEN));
                    moves.push_back(Move(sq, forwSq, MOVE_PROMOTION_ROOK));
                    moves.push_back(Move(sq, forwSq, MOVE_PROMOTION_BISHOP));
                    moves.push_back(Move(sq, forwSq, MOVE_PROMOTION_KNIGHT));
                } else
                    moves.push_back(Move(sq, forwSq, MOVE_QUIET));
            } else if(rank == startingRank &&
                      precomputedInfo.attackingRays.getBit(forw2Sq) &&
                      board.pieces[forwSq] == EMPTY &&
                        board.pieces[forw2Sq] == EMPTY) {

                moves.push_back(Move(sq, forw2Sq, MOVE_DOUBLE_PAWN));
            }

            // Der Bauer könnte den Angreifer schlagen
            Bitboard pawnAttacks = pawnAttackBitboard(sq, ownColor) &
                                   precomputedInfo.attackingRays &
                                   enemyPieces;

            // Es kann maximal einen Angreifer geben, also ist das hier sicher
            if(pawnAttacks) {
                int32_t captureSq = pawnAttacks.getFSB();
                if(rank == promotionRank) {
                    moves.push_back(Move(sq, captureSq, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                    moves.push_back(Move(sq, captureSq, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                    moves.push_back(Move(sq, captureSq, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                    moves.push_back(Move(sq, captureSq, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                } else
                    moves.push_back(Move(sq, captureSq, MOVE_CAPTURE));
            }
        }
    } else {
        while(pawns) {
            int32_t sq = pawns.popFSB();
            int32_t rank = SQ2R(sq);
            int32_t forwSq = sq + forw;
            int32_t forw2Sq = sq + forw2;

            bool isPinned = precomputedInfo.pinnedPieces.getBit(sq);

            if constexpr(checkEnPassant) {
                // En-Passant
                if(rank == enPasRankReq) {
                    int32_t captureSq = board.getEnPassantSquare();
                    Bitboard enPassantCaptures = pawnAttackBitboard(sq, ownColor) &
                                                Bitboard(1ULL << captureSq);

                    // Es kann maximal ein En-Passant pro Zug geben, also ist das hier sicher
                    if(enPassantCaptures) {
                        int32_t enPasSq = enPassantCaptures.getFSB();
                        int32_t captureDir = std::abs(enPasSq - sq);
                        
                        if(isPinned) {
                            // Überprüfe, ob der Zug entlang der Pin-Richtung verläuft
                            if(captureDir == precomputedInfo.pinDirections[sq] ||
                            captureDir == -precomputedInfo.pinDirections[sq])
                                moves.push_back(Move(sq, enPasSq, MOVE_EN_PASSANT));
                        } else {
                            // En-Passant entfernt zwei Figuren von einem
                            // Rang, also muss manuell überprüft werden, ob der
                            // König danach im Schach steht

                            int32_t ownKingSq = color == WHITE ?
                                board.pieceBitboard[WHITE_KING].getFSB() :
                                board.pieceBitboard[BLACK_KING].getFSB();

                            int32_t ownKingRank = SQ2R(ownKingSq);
                            constexpr int32_t dangerousKingRank = color == WHITE ?
                                RANK_5 : RANK_4;

                            Bitboard enemyKing = color == WHITE ?
                                board.pieceBitboard[BLACK_KING] : board.pieceBitboard[WHITE_KING];

                            Bitboard enemyQueensAndRooks = color == WHITE ?
                                board.pieceBitboard[BLACK_QUEEN] | board.pieceBitboard[BLACK_ROOK] :
                                board.pieceBitboard[WHITE_QUEEN] | board.pieceBitboard[WHITE_ROOK];

                            Bitboard piecesAfterMove = board.pieceBitboard[ALL_PIECES] | enemyKing;
                            piecesAfterMove.clearBit(sq);
                            piecesAfterMove.clearBit(captureSq - forw);
                            piecesAfterMove.setBit(enPasSq);

                            if(ownKingRank != dangerousKingRank ||
                            !(horizontalAttackBitboard(ownKingSq, piecesAfterMove) & enemyQueensAndRooks))
                                moves.push_back(Move(sq, enPasSq, MOVE_EN_PASSANT));
                            
                        }
                    }
                }
            }

            // Wenn der Bauer gefesselt ist, kann er nur
            // den Angreifer schlagen oder nach vorne ziehen
            // (wenn die Fesselrichtung vertikal ist)
            if(isPinned) {
                Bitboard pawnAttacks = pawnAttackBitboard(sq, ownColor) &
                                       enemyPieces;

                // Es kann maximal einen Angreifer geben, also ist das hier sicher
                while(pawnAttacks) {
                    int32_t captureSq = pawnAttacks.popFSB();
                    int32_t captureDir = std::abs(captureSq - sq);

                    // Überprüfe, ob der Zug nicht entlang der Pin-Richtung verläuft
                    if(captureDir != precomputedInfo.pinDirections[sq] &&
                       captureDir != -precomputedInfo.pinDirections[sq])
                        continue;

                    if(rank == promotionRank) {
                        moves.push_back(Move(sq, captureSq, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                        moves.push_back(Move(sq, captureSq, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                        moves.push_back(Move(sq, captureSq, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                        moves.push_back(Move(sq, captureSq, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                    } else
                        moves.push_back(Move(sq, captureSq, MOVE_CAPTURE));
                }

                // Überprüfe, ob der Bauer nach vorne ziehen kann
                if(precomputedInfo.pinDirections[sq] == NORTH ||
                   precomputedInfo.pinDirections[sq] == SOUTH) {
                    if(board.pieces[forwSq] == EMPTY) {
                        if(rank == promotionRank) {
                            moves.push_back(Move(sq, forwSq, MOVE_PROMOTION_QUEEN));
                            moves.push_back(Move(sq, forwSq, MOVE_PROMOTION_ROOK));
                            moves.push_back(Move(sq, forwSq, MOVE_PROMOTION_BISHOP));
                            moves.push_back(Move(sq, forwSq, MOVE_PROMOTION_KNIGHT));
                        } else {
                            moves.push_back(Move(sq, forwSq, MOVE_QUIET));
                            if(rank == startingRank &&
                               board.pieces[forw2Sq] == EMPTY)
                                moves.push_back(Move(sq, forw2Sq, MOVE_DOUBLE_PAWN));
                        }
                    }
                }
            } else {
                // Ansonsten sind beliebige Züge möglich
                if(board.pieces[forwSq] == EMPTY) {
                    if(rank == promotionRank) {
                        moves.push_back(Move(sq, forwSq, MOVE_PROMOTION_QUEEN));
                        moves.push_back(Move(sq, forwSq, MOVE_PROMOTION_ROOK));
                        moves.push_back(Move(sq, forwSq, MOVE_PROMOTION_BISHOP));
                        moves.push_back(Move(sq, forwSq, MOVE_PROMOTION_KNIGHT));
                    } else {
                        moves.push_back(Move(sq, forwSq, MOVE_QUIET));
                        if(rank == startingRank &&
                           board.pieces[forw2Sq] == EMPTY)
                            moves.push_back(Move(sq, forw2Sq, MOVE_DOUBLE_PAWN));
                    }
                }

                Bitboard pawnAttacks = pawnAttackBitboard(sq, ownColor) &
                                       enemyPieces;

                while(pawnAttacks) {
                    int32_t captureSq = pawnAttacks.popFSB();

                    if(rank == promotionRank) {
                        moves.push_back(Move(sq, captureSq, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                        moves.push_back(Move(sq, captureSq, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                        moves.push_back(Move(sq, captureSq, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                        moves.push_back(Move(sq, captureSq, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                    } else
                        moves.push_back(Move(sq, captureSq, MOVE_CAPTURE));
                }
            }
        }
    }
}

template <int32_t color>
inline void Movegen::generateKnightMoves(const Board& board, Array<Move, 256>& moves, const PrecomputedInfo& precomputedInfo) noexcept {
    constexpr int32_t knightPiece = color == WHITE ? WHITE_KNIGHT : BLACK_KNIGHT;
    Bitboard ownPieces = color == WHITE ?
        board.pieceBitboard[WHITE] : board.pieceBitboard[BLACK];
    Bitboard ownKing = color == WHITE ?
        board.pieceBitboard[WHITE_KING] : board.pieceBitboard[BLACK_KING];
    
    Bitboard knights = board.getPieceBitboard(knightPiece);

    while(knights) {
        int32_t sq = knights.popFSB();

        // Wenn der Springer gefesselt ist, kann er sich nicht bewegen
        if(precomputedInfo.pinnedPieces.getBit(sq))
            continue;

        Bitboard knightAttacks = knightAttackBitboard(sq) & ~(ownPieces | ownKing);

        // Wenn es einen Angreifer gibt, muss der Springer ihn schlagen
        // oder sich dazwischen stellen
        if(precomputedInfo.numAttackers == 1)
            knightAttacks &= precomputedInfo.attackingRays;
        
        while(knightAttacks) {
            int32_t captureSq = knightAttacks.popFSB();

            if(board.pieces[captureSq] == EMPTY)
                moves.push_back(Move(sq, captureSq, MOVE_QUIET));
            else
                moves.push_back(Move(sq, captureSq, MOVE_CAPTURE));
        }
    }
}

template <int32_t color>
inline void Movegen::generateDiagonalSlidingMoves(const Board& board, Array<Move, 256>& moves, const PrecomputedInfo& precomputedInfo) noexcept {
    constexpr int32_t bishopPiece = color == WHITE ? WHITE_BISHOP : BLACK_BISHOP;
    constexpr int32_t queenPiece = color == WHITE ? WHITE_QUEEN : BLACK_QUEEN;
    Bitboard ownPieces = color == WHITE ?
        board.pieceBitboard[WHITE] : board.pieceBitboard[BLACK];
    Bitboard ownKing = color == WHITE ?
        board.pieceBitboard[WHITE_KING] : board.pieceBitboard[BLACK_KING];
    
    Bitboard diagSlidingPieces = board.getPieceBitboard(bishopPiece) |
                                 board.getPieceBitboard(queenPiece);

    if(precomputedInfo.numAttackers == 1) {
        while(diagSlidingPieces) {
            int32_t sq = diagSlidingPieces.popFSB();

            // Wenn die Figur gefesselt ist, kann sie sich nicht bewegen
            if(precomputedInfo.pinnedPieces.getBit(sq))
                continue;

            Bitboard attacks = diagonalAttackBitboard(sq,
                                board.pieceBitboard[ALL_PIECES] | ownKing)
                                & ~(ownPieces | ownKing)
                                & precomputedInfo.attackingRays;

            while(attacks) {
                int32_t captureSq = attacks.popFSB();

                if(board.pieces[captureSq] == EMPTY)
                    moves.push_back(Move(sq, captureSq, MOVE_QUIET));
                else
                    moves.push_back(Move(sq, captureSq, MOVE_CAPTURE));
            }
        }
    } else {
        while(diagSlidingPieces) {
            int32_t sq = diagSlidingPieces.popFSB();

            Bitboard attacks = Bitboard::ONES;

            // Wenn die Figur gefesselt ist, kann sie sich nur
            // entlang der Pin-Richtung bewegen
            if(precomputedInfo.pinnedPieces.getBit(sq)) {
                int32_t pinDir = precomputedInfo.pinDirections[sq];
                if(pinDir == NORTH_WEST || pinDir == SOUTH_EAST)
                    attacks = (Magics::bishopAttackTopLeftMask(sq) |
                               Magics::bishopAttackBottomRightMask(sq));
                else if(pinDir == NORTH_EAST || pinDir == SOUTH_WEST)
                    attacks = (Magics::bishopAttackTopRightMask(sq) |
                               Magics::bishopAttackBottomLeftMask(sq));
                else
                    continue;
            }

            attacks &= diagonalAttackBitboard(sq,
                        board.pieceBitboard[ALL_PIECES] | ownKing)
                        & ~(ownPieces | ownKing);

            while(attacks) {
                int32_t captureSq = attacks.popFSB();

                if(board.pieces[captureSq] == EMPTY)
                    moves.push_back(Move(sq, captureSq, MOVE_QUIET));
                else
                    moves.push_back(Move(sq, captureSq, MOVE_CAPTURE));
            }
        }
    }
}

template <int32_t color>
inline void Movegen::generateHorizontalSlidingMoves(const Board& board, Array<Move, 256>& moves, const PrecomputedInfo& precomputedInfo) noexcept {
    constexpr int32_t rookPiece = color == WHITE ? WHITE_ROOK : BLACK_ROOK;
    constexpr int32_t queenPiece = color == WHITE ? WHITE_QUEEN : BLACK_QUEEN;
    Bitboard ownPieces = color == WHITE ?
        board.pieceBitboard[WHITE] : board.pieceBitboard[BLACK];
    Bitboard ownKing = color == WHITE ?
        board.pieceBitboard[WHITE_KING] : board.pieceBitboard[BLACK_KING];
    
    Bitboard horSlidingPieces = board.getPieceBitboard(rookPiece) |
                                board.getPieceBitboard(queenPiece);

    if(precomputedInfo.numAttackers == 1) {
        while(horSlidingPieces) {
            int32_t sq = horSlidingPieces.popFSB();

            // Wenn die Figur gefesselt ist, kann sie sich nicht bewegen
            if(precomputedInfo.pinnedPieces.getBit(sq))
                continue;

            Bitboard attacks = horizontalAttackBitboard(sq,
                                board.pieceBitboard[ALL_PIECES] | ownKing)
                                & ~(ownPieces | ownKing)
                                & precomputedInfo.attackingRays;

            while(attacks) {
                int32_t captureSq = attacks.popFSB();

                if(board.pieces[captureSq] == EMPTY)
                    moves.push_back(Move(sq, captureSq, MOVE_QUIET));
                else
                    moves.push_back(Move(sq, captureSq, MOVE_CAPTURE));
            }
        }
    } else {
        while(horSlidingPieces) {
            int32_t sq = horSlidingPieces.popFSB();

            Bitboard attacks = Bitboard::ONES;

            // Wenn die Figur gefesselt ist, kann sie sich nur
            // entlang der Pin-Richtung bewegen
            if(precomputedInfo.pinnedPieces.getBit(sq)) {
                int32_t pinDir = precomputedInfo.pinDirections[sq];
                if(pinDir == NORTH || pinDir == SOUTH)
                    attacks = (Magics::rookAttackTopMask(sq) |
                               Magics::rookAttackBottomMask(sq));
                else if(pinDir == EAST || pinDir == WEST)
                    attacks = (Magics::rookAttackRightMask(sq) |
                               Magics::rookAttackLeftMask(sq));
                else
                    continue;
            }

            attacks &= horizontalAttackBitboard(sq,
                        board.pieceBitboard[ALL_PIECES] | ownKing)
                        & ~(ownPieces | ownKing);

            while(attacks) {
                int32_t captureSq = attacks.popFSB();

                if(board.pieces[captureSq] == EMPTY)
                    moves.push_back(Move(sq, captureSq, MOVE_QUIET));
                else
                    moves.push_back(Move(sq, captureSq, MOVE_CAPTURE));
            }
        }
    }
}

template <int32_t color>
inline void Movegen::generateKingMoves(const Board& board, Array<Move, 256>& moves) noexcept {
    Bitboard ownPieces = color == WHITE ?
        board.pieceBitboard[WHITE] : board.pieceBitboard[BLACK];

    int32_t kingSq = color == WHITE ?
        board.pieceBitboard[WHITE_KING].getFSB() :
        board.pieceBitboard[BLACK_KING].getFSB();

    Bitboard enemyAttacks = color == WHITE ?
        board.attackBitboard[BLACK] : board.attackBitboard[WHITE];

    constexpr int32_t kingsideCastlingFlag = color == WHITE ?
        WHITE_KINGSIDE_CASTLE : BLACK_KINGSIDE_CASTLE;

    constexpr int32_t queensideCastlingFlag = color == WHITE ?
        WHITE_QUEENSIDE_CASTLE : BLACK_QUEENSIDE_CASTLE;

    constexpr Bitboard sqB1 = Bitboard(1ULL << B1);
    constexpr Bitboard sqC1 = Bitboard(1ULL << C1);
    constexpr Bitboard sqD1 = Bitboard(1ULL << D1);
    constexpr Bitboard sqE1 = Bitboard(1ULL << E1);
    constexpr Bitboard sqF1 = Bitboard(1ULL << F1);
    constexpr Bitboard sqG1 = Bitboard(1ULL << G1);

    constexpr Bitboard sqB8 = Bitboard(1ULL << B8);
    constexpr Bitboard sqC8 = Bitboard(1ULL << C8);
    constexpr Bitboard sqD8 = Bitboard(1ULL << D8);
    constexpr Bitboard sqE8 = Bitboard(1ULL << E8);
    constexpr Bitboard sqF8 = Bitboard(1ULL << F8);
    constexpr Bitboard sqG8 = Bitboard(1ULL << G8);
    

    // Es gibt immer exakt einen König, deshalb brauchen wir keine Schleife
    Bitboard kingAttacks = kingAttackBitboard(kingSq) &
                           ~ownPieces & ~enemyAttacks;

    while(kingAttacks) {
        int32_t captureSq = kingAttacks.popFSB();

        if(board.pieces[captureSq] == EMPTY)
            moves.push_back(Move(kingSq, captureSq, MOVE_QUIET));
        else
            moves.push_back(Move(kingSq, captureSq, MOVE_CAPTURE));
    }

    // Rochade
    if(board.castlingPermission & kingsideCastlingFlag) {
        constexpr int32_t castingOrigin = color == WHITE ? E1 : E8;
        constexpr int32_t castingDestination = color == WHITE ? G1 : G8;

        constexpr Bitboard reqEmptySqs = color == WHITE ?
            (sqF1 | sqG1) : (sqF8 | sqG8);

        constexpr Bitboard reqNotAttackedSqs = color == WHITE ?
            (sqE1 | sqF1 | sqG1) : (sqE8 | sqF8 | sqG8);

        if(!(reqEmptySqs & board.pieceBitboard[ALL_PIECES]) &&
           !(reqNotAttackedSqs & enemyAttacks)) {

            moves.push_back(Move(castingOrigin, castingDestination, MOVE_KINGSIDE_CASTLE));
        }
    }

    if(board.castlingPermission & queensideCastlingFlag) {
        constexpr int32_t castingOrigin = color == WHITE ? E1 : E8;
        constexpr int32_t castingDestination = color == WHITE ? C1 : C8;

        constexpr Bitboard reqEmptySqs = color == WHITE ?
            (sqB1 | sqC1 | sqD1) : (sqB8 | sqC8 | sqD8);

        constexpr Bitboard reqNotAttackedSqs = color == WHITE ?
            (sqC1 | sqD1 | sqE1) : (sqC8 | sqD8 | sqE8);

        if(!(reqEmptySqs & board.pieceBitboard[ALL_PIECES]) &&
           !(reqNotAttackedSqs & enemyAttacks)) {

            moves.push_back(Move(castingOrigin, castingDestination, MOVE_QUEENSIDE_CASTLE));
        }
    }
}

template <int32_t color, bool checkEnPassant>
inline void Movegen::generatePawnCaptures(const Board& board, Array<Move, 256>& moves, const PrecomputedInfo& precomputedInfo) noexcept {
    constexpr int32_t pawnPiece = color == WHITE ? WHITE_PAWN : BLACK_PAWN;
    constexpr int32_t forw = color == WHITE ? NORTH : SOUTH;
    constexpr int32_t promotionRank = color == WHITE ? RANK_7 : RANK_2;
    constexpr int32_t ownColor = color == WHITE ? WHITE : BLACK;
    constexpr int32_t enPasRankReq = color == WHITE ? RANK_5 : RANK_4;

    Bitboard enemyPieces = color == WHITE ? board.pieceBitboard[BLACK] : board.pieceBitboard[WHITE];
    
    Bitboard pawns = board.getPieceBitboard(pawnPiece);

    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Bauer den Angreifer schlagen oder sich dazwischen stellen
    if(precomputedInfo.numAttackers == 1) {
        while(pawns) {
            int32_t sq = pawns.popFSB();
            int32_t rank = SQ2R(sq);

            // Wenn der Bauer gefesselt ist, kann er sich nicht bewegen
            if(precomputedInfo.pinnedPieces.getBit(sq))
                continue;

            if constexpr(checkEnPassant) {
                // En-Passant könnte den Angreifer blockieren oder schlagen
                if(rank == enPasRankReq) {
                    int32_t captureSq = board.getEnPassantSquare();
                    int32_t enPasSq = captureSq - forw;
                    Bitboard captureSqBB = Bitboard(1ULL << captureSq);
                    Bitboard enPasSqBB = Bitboard(1ULL << enPasSq);

                    if((pawnAttackBitboard(sq, ownColor) & captureSqBB) &&
                    (captureSqBB | enPasSqBB) & precomputedInfo.attackingRays)
                        moves.push_back(Move(sq, captureSq, MOVE_EN_PASSANT));
                }
            }

            // Der Bauer könnte den Angreifer schlagen
            Bitboard pawnAttacks = pawnAttackBitboard(sq, ownColor) &
                                   precomputedInfo.attackingRays &
                                   enemyPieces;

            // Es kann maximal einen Angreifer geben, also ist das hier sicher
            if(pawnAttacks) {
                int32_t captureSq = pawnAttacks.getFSB();
                if(rank == promotionRank) {
                    moves.push_back(Move(sq, captureSq, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                    moves.push_back(Move(sq, captureSq, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                    moves.push_back(Move(sq, captureSq, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                    moves.push_back(Move(sq, captureSq, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                } else
                    moves.push_back(Move(sq, captureSq, MOVE_CAPTURE));
            }
        }
    } else {
        while(pawns) {
            int32_t sq = pawns.popFSB();
            int32_t rank = SQ2R(sq);

            bool isPinned = precomputedInfo.pinnedPieces.getBit(sq);

            // En-Passant
            if constexpr(checkEnPassant) {
                if(rank == enPasRankReq) {
                    int32_t captureSq = board.getEnPassantSquare();
                    Bitboard enPassantCaptures = pawnAttackBitboard(sq, ownColor) &
                                                Bitboard(1ULL << captureSq);

                    // Es kann maximal ein En-Passant pro Zug geben, also ist das hier sicher
                    if(enPassantCaptures) {
                        int32_t enPasSq = enPassantCaptures.getFSB();
                        int32_t captureDir = std::abs(enPasSq - sq);
                        
                        if(isPinned) {
                            // Überprüfe, ob der Zug entlang der Pin-Richtung verläuft
                            if(captureDir == precomputedInfo.pinDirections[sq] ||
                            captureDir == -precomputedInfo.pinDirections[sq])
                                moves.push_back(Move(sq, enPasSq, MOVE_EN_PASSANT));
                        } else {
                            // En-Passant entfernt zwei Figuren von einem
                            // Rang, also muss manuell überprüft werden, ob der
                            // König danach im Schach steht

                            int32_t ownKingSq = color == WHITE ?
                                board.pieceBitboard[WHITE_KING].getFSB() :
                                board.pieceBitboard[BLACK_KING].getFSB();

                            int32_t ownKingRank = SQ2R(ownKingSq);
                            constexpr int32_t dangerousKingRank = color == WHITE ?
                                RANK_5 : RANK_4;

                            Bitboard enemyKing = color == WHITE ?
                                board.pieceBitboard[BLACK_KING] : board.pieceBitboard[WHITE_KING];

                            Bitboard enemyQueensAndRooks = color == WHITE ?
                                board.pieceBitboard[BLACK_QUEEN] | board.pieceBitboard[BLACK_ROOK] :
                                board.pieceBitboard[WHITE_QUEEN] | board.pieceBitboard[WHITE_ROOK];

                            Bitboard piecesAfterMove = board.pieceBitboard[ALL_PIECES] | enemyKing;
                            piecesAfterMove.clearBit(sq);
                            piecesAfterMove.clearBit(captureSq - forw);
                            piecesAfterMove.setBit(enPasSq);

                            if(ownKingRank != dangerousKingRank ||
                            !(horizontalAttackBitboard(ownKingSq, piecesAfterMove) & enemyQueensAndRooks))
                                moves.push_back(Move(sq, enPasSq, MOVE_EN_PASSANT));
                        }
                    }
                }
            }

            // Wenn der Bauer gefesselt ist, kann er nur
            // den Angreifer schlagen oder nach vorne ziehen
            // (wenn die Fesselrichtung vertikal ist)
            if(isPinned) {
                Bitboard pawnAttacks = pawnAttackBitboard(sq, ownColor) &
                                       enemyPieces;

                // Es kann maximal einen Angreifer geben, also ist das hier sicher
                while(pawnAttacks) {
                    int32_t captureSq = pawnAttacks.popFSB();
                    int32_t captureDir = std::abs(captureSq - sq);

                    // Überprüfe, ob der Zug nicht entlang der Pin-Richtung verläuft
                    if(captureDir != precomputedInfo.pinDirections[sq] &&
                       captureDir != -precomputedInfo.pinDirections[sq])
                        continue;

                    if(rank == promotionRank) {
                        moves.push_back(Move(sq, captureSq, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                        moves.push_back(Move(sq, captureSq, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                        moves.push_back(Move(sq, captureSq, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                        moves.push_back(Move(sq, captureSq, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                    } else
                        moves.push_back(Move(sq, captureSq, MOVE_CAPTURE));
                }
            } else {
                // Ansonsten sind beliebige Züge möglich
                Bitboard pawnAttacks = pawnAttackBitboard(sq, ownColor) &
                                       enemyPieces;

                while(pawnAttacks) {
                    int32_t captureSq = pawnAttacks.popFSB();

                    if(rank == promotionRank) {
                        moves.push_back(Move(sq, captureSq, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                        moves.push_back(Move(sq, captureSq, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                        moves.push_back(Move(sq, captureSq, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                        moves.push_back(Move(sq, captureSq, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                    } else
                        moves.push_back(Move(sq, captureSq, MOVE_CAPTURE));
                }
            }
        }
    }
}

template <int32_t color>
inline void Movegen::generateKnightCaptures(const Board& board, Array<Move, 256>& moves, const PrecomputedInfo& precomputedInfo) noexcept {
    constexpr int32_t knightPiece = color == WHITE ? WHITE_KNIGHT : BLACK_KNIGHT;
    Bitboard enemyPieces = color == WHITE ?
        board.pieceBitboard[BLACK] : board.pieceBitboard[WHITE];
    
    Bitboard knights = board.getPieceBitboard(knightPiece);

    while(knights) {
        int32_t sq = knights.popFSB();

        // Wenn der Springer gefesselt ist, kann er sich nicht bewegen
        if(precomputedInfo.pinnedPieces.getBit(sq))
            continue;

        Bitboard knightAttacks = knightAttackBitboard(sq) & enemyPieces;

        // Wenn es einen Angreifer gibt, muss der Springer ihn schlagen
        if(precomputedInfo.numAttackers == 1)
            knightAttacks &= precomputedInfo.attackingRays;
        
        while(knightAttacks) {
            int32_t captureSq = knightAttacks.popFSB();
            moves.push_back(Move(sq, captureSq, MOVE_CAPTURE));
        }
    }
}

template <int32_t color>
inline void Movegen::generateDiagonalSlidingCaptures(const Board& board, Array<Move, 256>& moves, const PrecomputedInfo& precomputedInfo) noexcept {
    constexpr int32_t bishopPiece = color == WHITE ? WHITE_BISHOP : BLACK_BISHOP;
    constexpr int32_t queenPiece = color == WHITE ? WHITE_QUEEN : BLACK_QUEEN;
    Bitboard ownKing = color == WHITE ?
        board.pieceBitboard[WHITE_KING] : board.pieceBitboard[BLACK_KING];
    Bitboard enemyPieces = color == WHITE ?
        board.pieceBitboard[BLACK] : board.pieceBitboard[WHITE];
    
    Bitboard diagSlidingPieces = board.getPieceBitboard(bishopPiece) |
                                 board.getPieceBitboard(queenPiece);

    if(precomputedInfo.numAttackers == 1) {
        while(diagSlidingPieces) {
            int32_t sq = diagSlidingPieces.popFSB();

            // Wenn die Figur gefesselt ist, kann sie sich nicht bewegen
            if(precomputedInfo.pinnedPieces.getBit(sq))
                continue;

            Bitboard attacks = diagonalAttackBitboard(sq,
                                board.pieceBitboard[ALL_PIECES] | ownKing)
                                & enemyPieces
                                & precomputedInfo.attackingRays;

            while(attacks) {
                int32_t captureSq = attacks.popFSB();
                moves.push_back(Move(sq, captureSq, MOVE_CAPTURE));
            }
        }
    } else {
        while(diagSlidingPieces) {
            int32_t sq = diagSlidingPieces.popFSB();

            Bitboard attacks = Bitboard::ONES;

            // Wenn die Figur gefesselt ist, kann sie sich nur
            // entlang der Pin-Richtung bewegen
            if(precomputedInfo.pinnedPieces.getBit(sq)) {
                int32_t pinDir = precomputedInfo.pinDirections[sq];
                if(pinDir == NORTH_WEST || pinDir == SOUTH_EAST)
                    attacks = (Magics::bishopAttackTopLeftMask(sq) |
                               Magics::bishopAttackBottomRightMask(sq));
                else if(pinDir == NORTH_EAST || pinDir == SOUTH_WEST)
                    attacks = (Magics::bishopAttackTopRightMask(sq) |
                               Magics::bishopAttackBottomLeftMask(sq));
                else
                    continue;
            }

            attacks &= diagonalAttackBitboard(sq,
                        board.pieceBitboard[ALL_PIECES] | ownKing)
                        & enemyPieces;

            while(attacks) {
                int32_t captureSq = attacks.popFSB();
                moves.push_back(Move(sq, captureSq, MOVE_CAPTURE));
            }
        }
    }
}

template <int32_t color>
inline void Movegen::generateHorizontalSlidingCaptures(const Board& board, Array<Move, 256>& moves, const PrecomputedInfo& precomputedInfo) noexcept {
    constexpr int32_t rookPiece = color == WHITE ? WHITE_ROOK : BLACK_ROOK;
    constexpr int32_t queenPiece = color == WHITE ? WHITE_QUEEN : BLACK_QUEEN;
    Bitboard ownKing = color == WHITE ?
        board.pieceBitboard[WHITE_KING] : board.pieceBitboard[BLACK_KING];
    Bitboard enemyPieces = color == WHITE ?
        board.pieceBitboard[BLACK] : board.pieceBitboard[WHITE];
    
    Bitboard horSlidingPieces = board.getPieceBitboard(rookPiece) |
                                board.getPieceBitboard(queenPiece);

    if(precomputedInfo.numAttackers == 1) {
        while(horSlidingPieces) {
            int32_t sq = horSlidingPieces.popFSB();

            // Wenn die Figur gefesselt ist, kann sie sich nicht bewegen
            if(precomputedInfo.pinnedPieces.getBit(sq))
                continue;

            Bitboard attacks = horizontalAttackBitboard(sq,
                                board.pieceBitboard[ALL_PIECES] | ownKing)
                                & enemyPieces
                                & precomputedInfo.attackingRays;

            while(attacks) {
                int32_t captureSq = attacks.popFSB();
                moves.push_back(Move(sq, captureSq, MOVE_CAPTURE));
            }
        }
    } else {
        while(horSlidingPieces) {
            int32_t sq = horSlidingPieces.popFSB();

            Bitboard attacks = Bitboard::ONES;

            // Wenn die Figur gefesselt ist, kann sie sich nur
            // entlang der Pin-Richtung bewegen
            if(precomputedInfo.pinnedPieces.getBit(sq)) {
                int32_t pinDir = precomputedInfo.pinDirections[sq];
                if(pinDir == NORTH || pinDir == SOUTH)
                    attacks = (Magics::rookAttackTopMask(sq) |
                               Magics::rookAttackBottomMask(sq));
                else if(pinDir == EAST || pinDir == WEST)
                    attacks = (Magics::rookAttackRightMask(sq) |
                               Magics::rookAttackLeftMask(sq));
                else
                    continue;
            }

            attacks &= horizontalAttackBitboard(sq,
                        board.pieceBitboard[ALL_PIECES] | ownKing)
                        & enemyPieces;

            while(attacks) {
                int32_t captureSq = attacks.popFSB();
                moves.push_back(Move(sq, captureSq, MOVE_CAPTURE));
            }
        }
    }
}

template <int32_t color>
inline void Movegen::generateKingCaptures(const Board& board, Array<Move, 256>& moves) noexcept {
    Bitboard enemyPieces = color == WHITE ?
        board.pieceBitboard[BLACK] : board.pieceBitboard[WHITE];

    int32_t kingSq = color == WHITE ?
        board.pieceBitboard[WHITE_KING].getFSB() :
        board.pieceBitboard[BLACK_KING].getFSB();

    Bitboard enemyAttacks = color == WHITE ?
        board.attackBitboard[BLACK] : board.attackBitboard[WHITE];

    // Es gibt immer exakt einen König, deshalb brauchen wir keine Schleife
    Bitboard kingAttacks = kingAttackBitboard(kingSq) &
                           enemyPieces & ~enemyAttacks;

    while(kingAttacks) {
        int32_t captureSq = kingAttacks.popFSB();
        moves.push_back(Move(kingSq, captureSq, MOVE_CAPTURE));
    }
}

#endif