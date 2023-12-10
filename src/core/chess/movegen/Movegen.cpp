#include "core/chess/movegen/Movegen.h"

void Movegen::generatePseudoLegalWhitePawnMoves(Array<Move, 256>& moves, const Board& b) {
    for(int sq : b.pieceList[WHITE_PAWN]) {
        int32_t file = SQ2F(sq);
        int32_t rank = SQ2R(sq);

        int32_t destForw = FR2SQ(file, rank + 1);

        if(b.pieces[destForw] == EMPTY) {
            if(rank == RANK_7) {
                moves.push_back(Move(sq, destForw, MOVE_PROMOTION_KNIGHT));
                moves.push_back(Move(sq, destForw, MOVE_PROMOTION_BISHOP));
                moves.push_back(Move(sq, destForw, MOVE_PROMOTION_ROOK));
                moves.push_back(Move(sq, destForw, MOVE_PROMOTION_QUEEN));
            }
            else
                moves.push_back(Move(sq, destForw, MOVE_QUIET));
        }

        if(rank == RANK_2) {
            int32_t destForw2 = FR2SQ(file, rank + 2);
            if(b.pieces[destForw] == EMPTY && b.pieces[destForw2] == EMPTY)
                moves.push_back(Move(sq, destForw2, MOVE_DOUBLE_PAWN));
        }

        Bitboard pawnAttacks = pawnAttackBitboard(sq, WHITE);
        while(pawnAttacks) {
            int32_t dest = pawnAttacks.getFirstSetBit();
            if(b.pieces[dest] != EMPTY && (b.pieces[dest] & COLOR_MASK) == BLACK) {
                if(rank == RANK_7) {
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                }
                else
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE));
            }
            else if(dest == b.enPassantSquare)
                moves.push_back(Move(sq, dest, MOVE_EN_PASSANT));

            pawnAttacks.clearBit(dest);
        }
    }
}

void Movegen::generatePseudoLegalBlackPawnMoves(Array<Move, 256>& moves, const Board& b) {
    for(int sq : b.pieceList[BLACK_PAWN]) {
        int32_t file = SQ2F(sq);
        int32_t rank = SQ2R(sq);

        int32_t destForw = FR2SQ(file, rank - 1);

        if(b.pieces[destForw] == EMPTY) {
            if(rank == RANK_2) {
                moves.push_back(Move(sq, destForw, MOVE_PROMOTION_KNIGHT));
                moves.push_back(Move(sq, destForw, MOVE_PROMOTION_BISHOP));
                moves.push_back(Move(sq, destForw, MOVE_PROMOTION_ROOK));
                moves.push_back(Move(sq, destForw, MOVE_PROMOTION_QUEEN));
            }
            else
                moves.push_back(Move(sq, destForw, MOVE_QUIET));
        }

        if(rank == RANK_7) {
            int32_t destForw2 = FR2SQ(file, rank - 2);
            if(b.pieces[destForw] == EMPTY && b.pieces[destForw2] == EMPTY)
                moves.push_back(Move(sq, destForw2, MOVE_DOUBLE_PAWN));
        }

        Bitboard pawnAttacks = pawnAttackBitboard(sq, BLACK);
        while(pawnAttacks) {
            int32_t dest = pawnAttacks.getFirstSetBit();
            if(b.pieces[dest] != EMPTY && (b.pieces[dest] & COLOR_MASK) == WHITE) {
                if(rank == RANK_2) {
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                }
                else
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE));
            }
            else if(dest == b.enPassantSquare)
                moves.push_back(Move(sq, dest, MOVE_EN_PASSANT));

            pawnAttacks.clearBit(dest);
        }
    }
}

void Movegen::generatePseudoLegalWhiteKnightMoves(Array<Move, 256>& moves, const Board& b) {
    for(int sq : b.pieceList[WHITE_KNIGHT]) {
        Bitboard knightAttacks = knightAttackBitboard(sq) & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];
        while(knightAttacks) {
            int32_t dest = knightAttacks.getFirstSetBit();
            if(b.pieces[dest] == EMPTY)
                moves.push_back(Move(sq, dest, MOVE_QUIET));
            else
                moves.push_back(Move(sq, dest, MOVE_CAPTURE));

            knightAttacks.clearBit(dest);
        }
    }
}

void Movegen:: generatePseudoLegalBlackKnightMoves(Array<Move, 256>& moves, const Board& b) {
    for(int sq : b.pieceList[BLACK_KNIGHT]) {
        Bitboard knightAttacks = knightAttackBitboard(sq) & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];
        while(knightAttacks) {
            int32_t dest = knightAttacks.getFirstSetBit();
            if(b.pieces[dest] == EMPTY)
                moves.push_back(Move(sq, dest, MOVE_QUIET));
            else
                moves.push_back(Move(sq, dest, MOVE_CAPTURE));

            knightAttacks.clearBit(dest);
        }
    }
}

void Movegen::generatePseudoLegalWhiteBishopMoves(Array<Move, 256>& moves, const Board& b) {
    for(int sq : b.pieceList[WHITE_BISHOP]) {
        Bitboard bishopAttacks = diagonalAttackBitboard(sq, b.allPiecesBitboard | b.pieceBitboard[WHITE_KING])
                                    & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];
        while(bishopAttacks) {
            int32_t dest = bishopAttacks.getFirstSetBit();
            if(b.pieces[dest] == EMPTY)
                moves.push_back(Move(sq, dest, MOVE_QUIET));
            else
                moves.push_back(Move(sq, dest, MOVE_CAPTURE));

            bishopAttacks.clearBit(dest);
        }
    }
}

void Movegen::generatePseudoLegalBlackBishopMoves(Array<Move, 256>& moves, const Board& b) {
    for(int sq : b.pieceList[BLACK_BISHOP]) {
        Bitboard bishopAttacks = diagonalAttackBitboard(sq, b.allPiecesBitboard | b.pieceBitboard[BLACK_KING])
                                    & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];
        while(bishopAttacks) {
            int32_t dest = bishopAttacks.getFirstSetBit();
            if(b.pieces[dest] == EMPTY)
                moves.push_back(Move(sq, dest, MOVE_QUIET));
            else
                moves.push_back(Move(sq, dest, MOVE_CAPTURE));

            bishopAttacks.clearBit(dest);
        }
    }
}

void Movegen::generatePseudoLegalWhiteRookMoves(Array<Move, 256>& moves, const Board& b) {
    for(int sq : b.pieceList[WHITE_ROOK]) {
        Bitboard rookAttacks = straightAttackBitboard(sq, b.allPiecesBitboard | b.pieceBitboard[WHITE_KING])
                                    & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];
        while(rookAttacks) {
            int32_t dest = rookAttacks.getFirstSetBit();
            if(b.pieces[dest] == EMPTY)
                moves.push_back(Move(sq, dest, MOVE_QUIET));
            else
                moves.push_back(Move(sq, dest, MOVE_CAPTURE));

            rookAttacks.clearBit(dest);
        }
    }
}

void Movegen::generatePseudoLegalBlackRookMoves(Array<Move, 256>& moves, const Board& b) {
    for(int sq : b.pieceList[BLACK_ROOK]) {
        Bitboard rookAttacks = straightAttackBitboard(sq, b.allPiecesBitboard | b.pieceBitboard[BLACK_KING])
                                    & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];
        while(rookAttacks) {
            int32_t dest = rookAttacks.getFirstSetBit();
            if(b.pieces[dest] == EMPTY)
                moves.push_back(Move(sq, dest, MOVE_QUIET));
            else
                moves.push_back(Move(sq, dest, MOVE_CAPTURE));

            rookAttacks.clearBit(dest);
        }
    }
}

void Movegen::generatePseudoLegalWhiteQueenMoves(Array<Move, 256>& moves, const Board& b) {
    for(int sq : b.pieceList[WHITE_QUEEN]) {
        Bitboard queenAttacks = (diagonalAttackBitboard(sq, b.allPiecesBitboard | b.pieceBitboard[WHITE_KING])
                                    | straightAttackBitboard(sq, b.allPiecesBitboard | b.pieceBitboard[WHITE_KING]))
                                    & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];
        while(queenAttacks) {
            int32_t dest = queenAttacks.getFirstSetBit();
            if(b.pieces[dest] == EMPTY)
                moves.push_back(Move(sq, dest, MOVE_QUIET));
            else
                moves.push_back(Move(sq, dest, MOVE_CAPTURE));

            queenAttacks.clearBit(dest);
        }
    }
}

void Movegen::generatePseudoLegalBlackQueenMoves(Array<Move, 256>& moves, const Board& b) {
    for(int sq : b.pieceList[BLACK_QUEEN]) {
        Bitboard queenAttacks = (diagonalAttackBitboard(sq, b.allPiecesBitboard | b.pieceBitboard[BLACK_KING])
                                    | straightAttackBitboard(sq, b.allPiecesBitboard | b.pieceBitboard[BLACK_KING]))
                                    & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];

        while(queenAttacks) {
            int32_t dest = queenAttacks.getFirstSetBit();
            if(b.pieces[dest] == EMPTY)
                moves.push_back(Move(sq, dest, MOVE_QUIET));
            else
                moves.push_back(Move(sq, dest, MOVE_CAPTURE));

            queenAttacks.clearBit(dest);
        }
    }
}

void Movegen::generatePseudoLegalWhiteKingMoves(Array<Move, 256>& moves, const Board& b) {
    int sq = b.pieceList[WHITE_KING].front();
    Bitboard kingAttacks = kingAttackBitboard(sq) & ~b.whitePiecesBitboard & ~b.blackAttackBitboard;
    while(kingAttacks) {
        int32_t dest = kingAttacks.getFirstSetBit();
        if(b.pieces[dest] == EMPTY)
            moves.push_back(Move(sq, dest, MOVE_QUIET));
        else
            moves.push_back(Move(sq, dest, MOVE_CAPTURE));

        kingAttacks.clearBit(dest);
    }

    if(b.castlingPermission & WHITE_KINGSIDE_CASTLE) {
        if(b.pieces[F1] == EMPTY && b.pieces[G1] == EMPTY) {
            moves.push_back(Move(E1, G1, MOVE_KINGSIDE_CASTLE));
        }
    }

    if(b.castlingPermission & WHITE_QUEENSIDE_CASTLE) {
        if(b.pieces[D1] == EMPTY && b.pieces[C1] == EMPTY && b.pieces[B1] == EMPTY) {
            moves.push_back(Move(E1, C1, MOVE_QUEENSIDE_CASTLE));
        }
    }
}

void Movegen::generatePseudoLegalBlackKingMoves(Array<Move, 256>& moves, const Board& b) {
    int sq = b.pieceList[BLACK_KING].front();
    Bitboard kingAttacks = kingAttackBitboard(sq) & ~b.blackPiecesBitboard & ~b.whiteAttackBitboard;
    while(kingAttacks) {
        int32_t dest = kingAttacks.getFirstSetBit();
        if(b.pieces[dest] == EMPTY)
            moves.push_back(Move(sq, dest, MOVE_QUIET));
        else
            moves.push_back(Move(sq, dest, MOVE_CAPTURE));

        kingAttacks.clearBit(dest);
    }

    if(b.castlingPermission & BLACK_KINGSIDE_CASTLE) {
        if(b.pieces[F8] == EMPTY && b.pieces[G8] == EMPTY) {
            moves.push_back(Move(E8, G8, MOVE_KINGSIDE_CASTLE));
        }
    }

    if(b.castlingPermission & BLACK_QUEENSIDE_CASTLE) {
        if(b.pieces[D8] == EMPTY && b.pieces[C8] == EMPTY && b.pieces[B8] == EMPTY) {
            moves.push_back(Move(E8, C8, MOVE_QUEENSIDE_CASTLE));
        }
    }
}

void Movegen::generateWhitePawnMoves(Array<Move, 256>& moves, const Board& b,
                                     int32_t numAttackers, Bitboard attackingRays,
                                     Bitboard pinnedPiecesBitboard, int32_t* pinDirections) {
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Bauernzug kann legal sein
    if(numAttackers > 1)
        return;

    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Bauer den Angreifer schlagen oder den König schützen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[WHITE_PAWN]) {
            int32_t rank = SQ2R(sq);

            int32_t destForw = sq + NORTH;
            int32_t destForw2 = sq + 2 * NORTH;
            int32_t destLeft = sq + NORTH_WEST;
            int32_t destRight = sq + NORTH_EAST;

            int32_t file = SQ2F(sq);

            bool pinned = pinnedPiecesBitboard.getBit(sq);

            // Wenn der Bauer gefesselt ist, kann er sich nicht bewegen
            if(pinned)
                continue;

            // En-Passant Züge befreien den König nur aus dem Schach, wenn der geschlagene Bauer der Angreifer ist
            if(b.enPassantSquare != NO_SQ) {
                int32_t captureSquare = b.enPassantSquare + SOUTH;

                if(b.enPassantSquare == destLeft && file != FILE_A) {
                    if(attackingRays.getBit(captureSquare))
                        moves.push_back(Move(sq, destLeft, MOVE_EN_PASSANT));
                } else if(b.enPassantSquare == destRight && file != FILE_H) {
                    if(attackingRays.getBit(captureSquare))
                        moves.push_back(Move(sq, destRight, MOVE_EN_PASSANT));
                }
            }
            
            // Wenn der Bauer den Angreifer blockiert, ist der Zug legal
            // Ein blockierender Zug kann kein normaler Schlagzug sein, weil der Bauer sonst nicht gefesselt wäre
            if(attackingRays.getBit(destForw) && b.pieces[destForw] == EMPTY) {
                // Promotion
                if(rank == RANK_7) {
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_QUEEN));
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_ROOK));
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_BISHOP));
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_KNIGHT));
                } else
                    moves.push_back(Move(sq, destForw, MOVE_QUIET));
            } else if(rank == RANK_2 && attackingRays.getBit(destForw2) && b.pieces[destForw2] == EMPTY && b.pieces[destForw] == EMPTY)
                moves.push_back(Move(sq, destForw2, MOVE_DOUBLE_PAWN));
            
            // Wenn der Bauer den Angreifer schlagen kann, ist der Zug legal
            Bitboard pawnAttacks = pawnAttackBitboard(sq, WHITE) & attackingRays & b.blackPiecesBitboard;

            while(pawnAttacks) {
                int32_t dest = pawnAttacks.getFirstSetBit();
                // Promotion
                if(rank == RANK_7) {
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                } else
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE));

                pawnAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[WHITE_PAWN]) {
            int32_t rank = SQ2R(sq);

            int32_t destForw = sq + NORTH;
            int32_t destForw2 = sq + 2 * NORTH;
            int32_t destLeft = sq + NORTH_WEST;
            int32_t destRight = sq + NORTH_EAST;

            int32_t file = SQ2F(sq);

            bool pinned = pinnedPiecesBitboard.getBit(sq);

            // En-Passant Züge entfernen 2 Figuren aus einer Reihe, daher muss immer überprüft werden, ob der König nach dem Zug im Schach stände
            // -> Ein En-Passant Zug mit einem ungefesselten Bauer kann illegal sein
            if(b.enPassantSquare != NO_SQ) {
                int32_t captureSquare = b.enPassantSquare + SOUTH;
                int32_t kingSquare = b.pieceList[WHITE_KING].front();

                if(b.enPassantSquare == destLeft && file != FILE_A) {
                    Bitboard enemyRooksAndQueens = b.pieceBitboard[BLACK_ROOK] | b.pieceBitboard[BLACK_QUEEN];
                    Bitboard allPiecesAfterMove = b.allPiecesBitboard | b.pieceBitboard[BLACK_KING];
                    allPiecesAfterMove.clearBit(sq);
                    allPiecesAfterMove.clearBit(captureSquare);
                    allPiecesAfterMove.setBit(destLeft);

                    if(pinned) {
                        int32_t pinDir = pinDirections[sq];
                        if(pinDir == NORTH_WEST || pinDir == SOUTH_EAST)
                            moves.push_back(Move(sq, destLeft, MOVE_EN_PASSANT));
                    } else {
                        if(SQ2R(kingSquare) != RANK_5 ||
                           !(straightAttackBitboard(kingSquare, allPiecesAfterMove) & enemyRooksAndQueens))
                            moves.push_back(Move(sq, destLeft, MOVE_EN_PASSANT));
                    }

                } else if(b.enPassantSquare == destRight && file != FILE_H) {
                    Bitboard enemyRooksAndQueens = b.pieceBitboard[BLACK_ROOK] | b.pieceBitboard[BLACK_QUEEN];
                    Bitboard allPiecesAfterMove = b.allPiecesBitboard | b.pieceBitboard[BLACK_KING];
                    allPiecesAfterMove.clearBit(sq);
                    allPiecesAfterMove.clearBit(captureSquare);
                    allPiecesAfterMove.setBit(destRight);

                    if(pinned) {
                        int32_t pinDir = pinDirections[sq];
                        if(pinDir == NORTH_EAST || pinDir == SOUTH_WEST)
                            moves.push_back(Move(sq, destRight, MOVE_EN_PASSANT));
                    } else {
                        if(SQ2R(kingSquare) != RANK_5 ||
                           !(straightAttackBitboard(kingSquare, allPiecesAfterMove) & enemyRooksAndQueens))
                            moves.push_back(Move(sq, destRight, MOVE_EN_PASSANT));
                    }
                }
            }

            // Überprüfe, ob der Bauer horizontal oder diagonal gefesselt ist
            // Wenn ja, kann er sich nur bewegen, wenn er den Angreifer schlägt(diagonal)
            if(pinned) {
                int32_t pinDir = pinDirections[sq];
                if(pinDir == SOUTH_EAST || pinDir == NORTH_WEST) {
                    if(b.pieces[destLeft] != EMPTY && (b.pieces[destLeft] & COLOR_MASK) == BLACK) {
                        // Promotion
                        if(rank == RANK_7) {
                                moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                                moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                                moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                                moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                        } else
                            moves.push_back(Move(sq, destLeft, MOVE_CAPTURE));
                    }
                } else if(pinDir == SOUTH_WEST || pinDir == NORTH_EAST) {
                    if(b.pieces[destRight] != EMPTY && (b.pieces[destRight] & COLOR_MASK) == BLACK) {
                        // Promotion
                        if(rank == RANK_7) {
                                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                        } else
                            moves.push_back(Move(sq, destRight, MOVE_CAPTURE));
                    }
                }
                
                if(!(pinDir == NORTH || pinDir == SOUTH))
                    continue;
            }
            
            // Wenn der Bauer nicht horizontal oder diagonal gefesselt ist, kann er sich nach vorne bewegen
            if(b.pieces[destForw] == EMPTY) {
                // Promotion
                if(rank == RANK_7) {
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_QUEEN));
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_ROOK));
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_BISHOP));
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_KNIGHT));
                } else
                    moves.push_back(Move(sq, destForw, MOVE_QUIET));
            }

            if(rank == RANK_2 && b.pieces[destForw2] == EMPTY && b.pieces[destForw] == EMPTY)
                moves.push_back(Move(sq, destForw2, MOVE_DOUBLE_PAWN));
            
            // Schlagzüge
            // sind nur möglich, wenn der Bauer nicht gefesselt ist
            if(!pinned) {
                Bitboard pawnAttacks = pawnAttackBitboard(sq, WHITE) & b.blackPiecesBitboard;

                while(pawnAttacks) {
                    int32_t dest = pawnAttacks.getFirstSetBit();
                    // Promotion
                    if(rank == RANK_7) {
                            moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                            moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                            moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                            moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                    } else
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE));

                    pawnAttacks.clearBit(dest);
                }
            }
        }
    }
}

void Movegen::generateBlackPawnMoves(Array<Move, 256>& moves, const Board& b,
                                     int32_t numAttackers, Bitboard attackingRays,
                                     Bitboard pinnedPiecesBitboard, int32_t* pinDirections) {
    
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Bauerzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Bauer den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[BLACK_PAWN]) {
            int32_t rank = SQ2R(sq);

            int32_t destForw = sq + SOUTH;
            int32_t destForw2 = sq + 2 * SOUTH;
            int32_t destLeft = sq + SOUTH_WEST;
            int32_t destRight = sq + SOUTH_EAST;

            int32_t file = SQ2F(sq);

            bool pinned = pinnedPiecesBitboard.getBit(sq);

            // Wenn der Bauer gefesselt ist, kann er sich nicht bewegen
            if(pinned)
                continue;
            
            // En-Passant Züge befreien den König nur aus dem Schach, wenn der geschlagene Bauer der Angreifer ist
            if(b.enPassantSquare != NO_SQ) {
                int32_t captureSquare = b.enPassantSquare + NORTH;

                if(b.enPassantSquare == destLeft && file != FILE_A) {
                    if(attackingRays.getBit(captureSquare))
                        moves.push_back(Move(sq, destLeft, MOVE_EN_PASSANT));
                } else if(b.enPassantSquare == destRight && file != FILE_H) {
                    if(attackingRays.getBit(captureSquare))
                        moves.push_back(Move(sq, destRight, MOVE_EN_PASSANT));
                }
            }
            
            // Wenn der Bauer den Angreifer blockiert, ist der Zug legal
            // Ein blockierender Zug kann kein normaler Schlagzug sein, weil der Bauer sonst nicht gefesselt wäre
            if(attackingRays.getBit(destForw) && b.pieces[destForw] == EMPTY) {
                // Promotion
                if(rank == RANK_2) {
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_QUEEN));
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_ROOK));
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_BISHOP));
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_KNIGHT));
                } else
                    moves.push_back(Move(sq, destForw, MOVE_QUIET));
            } else if(rank == RANK_7 && attackingRays.getBit(destForw2) && b.pieces[destForw2] == EMPTY && b.pieces[destForw] == EMPTY)
                moves.push_back(Move(sq, destForw2, MOVE_DOUBLE_PAWN));
            
            // Wenn der Bauer den Angreifer schlagen kann, ist der Zug legal
            Bitboard pawnAttacks = pawnAttackBitboard(sq, BLACK) & attackingRays & b.whitePiecesBitboard;

            while(pawnAttacks) {
                int32_t dest = pawnAttacks.getFirstSetBit();
                // Promotion
                if(rank == RANK_2) {
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                } else
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE));

                pawnAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[BLACK_PAWN]) {
            int32_t rank = SQ2R(sq);

            int32_t destForw = sq + SOUTH;
            int32_t destForw2 = sq + 2 * SOUTH;
            int32_t destLeft = sq + SOUTH_WEST;
            int32_t destRight = sq + SOUTH_EAST;

            int32_t file = SQ2F(sq);

            bool pinned = pinnedPiecesBitboard.getBit(sq);

            // En-Passant Züge entfernen 2 Figuren aus einer Reihe, daher muss immer überprüft werden, ob der König nach dem Zug im Schach stände
            // -> Ein En-Passant Zug mit einem ungefesselten Bauer kann illegal sein
            if(b.enPassantSquare != NO_SQ) {
                int32_t captureSquare = b.enPassantSquare + NORTH;
                int32_t kingSquare = b.pieceList[BLACK_KING].front();

                if(b.enPassantSquare == destLeft && file != FILE_A) {
                    Bitboard enemyRooksAndQueens = b.pieceBitboard[WHITE_ROOK] | b.pieceBitboard[WHITE_QUEEN];
                    Bitboard allPiecesAfterMove = b.allPiecesBitboard | b.pieceBitboard[WHITE_KING];
                    allPiecesAfterMove.clearBit(sq);
                    allPiecesAfterMove.clearBit(captureSquare);
                    allPiecesAfterMove.setBit(destLeft);

                    if(pinned) {
                        int32_t pinDir = pinDirections[sq];
                        if(pinDir == SOUTH_WEST || pinDir == NORTH_EAST)
                            moves.push_back(Move(sq, destLeft, MOVE_EN_PASSANT));
                    } else {
                        if(SQ2R(kingSquare) != RANK_4 ||
                           !(straightAttackBitboard(kingSquare, allPiecesAfterMove) & enemyRooksAndQueens))
                            moves.push_back(Move(sq, destLeft, MOVE_EN_PASSANT));
                    }

                } else if(b.enPassantSquare == destRight && file != FILE_H) {
                    Bitboard enemyRooksAndQueens = b.pieceBitboard[WHITE_ROOK] | b.pieceBitboard[WHITE_QUEEN];
                    Bitboard allPiecesAfterMove = b.allPiecesBitboard | b.pieceBitboard[WHITE_KING];
                    allPiecesAfterMove.clearBit(sq);
                    allPiecesAfterMove.clearBit(captureSquare);
                    allPiecesAfterMove.setBit(destRight);

                    if(pinned) {
                        int32_t pinDir = pinDirections[sq];
                        if(pinDir == SOUTH_EAST || pinDir == NORTH_WEST)
                            moves.push_back(Move(sq, destRight, MOVE_EN_PASSANT));
                    } else {
                        if(SQ2R(kingSquare) != RANK_4 ||
                           !(straightAttackBitboard(kingSquare, allPiecesAfterMove) & enemyRooksAndQueens))
                            moves.push_back(Move(sq, destRight, MOVE_EN_PASSANT));
                    }
                }
            }

            // Überprüfe, ob der Bauer horizontal oder diagonal gefesselt ist
            // Wenn ja, kann er sich nur bewegen, wenn er den Angreifer schlägt(diagonal)
            if(pinned) {
                int32_t pinDir = pinDirections[sq];
                
                if(pinDir == NORTH_EAST || pinDir == SOUTH_WEST) {
                    if(b.pieces[destLeft] != EMPTY && (b.pieces[destLeft] & COLOR_MASK) == WHITE) {
                        // Promotion
                        if(rank == RANK_2) {
                            moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                            moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                            moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                            moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                        } else
                            moves.push_back(Move(sq, destLeft, MOVE_CAPTURE));
                    }
                } else if(pinDir == NORTH_WEST || pinDir == SOUTH_EAST) {
                    if(b.pieces[destRight] != EMPTY && (b.pieces[destRight] & COLOR_MASK) == WHITE) {
                        // Promotion
                        if(rank == RANK_2) {
                            moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                            moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                            moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                            moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                        } else
                            moves.push_back(Move(sq, destRight, MOVE_CAPTURE));
                    }
                }
                
                if(!(pinDir == NORTH || pinDir == SOUTH))
                    continue;
            }

            // Wenn der Bauer nicht horizontal oder diagonal gefesselt ist, kann er sich nach vorne bewegen
            if(b.pieces[destForw] == EMPTY) {
                // Promotion
                if(rank == RANK_2) {
                    moves.push_back(Move(sq, destForw, MOVE_PROMOTION_QUEEN));
                    moves.push_back(Move(sq, destForw, MOVE_PROMOTION_ROOK));
                    moves.push_back(Move(sq, destForw, MOVE_PROMOTION_BISHOP));
                    moves.push_back(Move(sq, destForw, MOVE_PROMOTION_KNIGHT));
                } else
                    moves.push_back(Move(sq, destForw, MOVE_QUIET));
            }

            if(rank == RANK_7 && b.pieces[destForw2] == EMPTY && b.pieces[destForw] == EMPTY)
                moves.push_back(Move(sq, destForw2, MOVE_DOUBLE_PAWN));
            
            // Schlagzüge
            // sind nur möglich, wenn der Bauer nicht gefesselt ist
            if(!pinned) {
                Bitboard pawnAttacks = pawnAttackBitboard(sq, BLACK) & b.whitePiecesBitboard;

                while(pawnAttacks) {
                    int32_t dest = pawnAttacks.getFirstSetBit();
                    // Promotion
                    if(rank == RANK_2) {
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                    } else
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE));

                    pawnAttacks.clearBit(dest);
                }
            }
        }
    }
}

void Movegen::generateWhiteKnightMoves(Array<Move, 256>& moves, const Board& b,
                                       int32_t numAttackers, Bitboard attackingRays,
                                       Bitboard pinnedPieces) {
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Springerzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Springer den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[WHITE_KNIGHT]) {
            // Wenn der Springer gefesselt ist, kann er sich nicht bewegen
            if(pinnedPieces.getBit(sq))
                continue;

            Bitboard knightAttacks = knightAttackBitboard(sq) & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];

            knightAttacks &= attackingRays;
            while(knightAttacks) {
                int32_t dest = knightAttacks.getFirstSetBit();
                if(b.pieces[dest] == EMPTY)
                    moves.push_back(Move(sq, dest, MOVE_QUIET));
                else
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE));

                knightAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[WHITE_KNIGHT]) {
            // Wenn der Springer gefesselt ist, kann er sich nicht bewegen
            if(pinnedPieces.getBit(sq))
                continue;

            // Ansonsten darf er sich frei bewegen
            Bitboard knightAttacks = knightAttackBitboard(sq) & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];
            while(knightAttacks) {
                int32_t dest = knightAttacks.getFirstSetBit();
                if(b.pieces[dest] == EMPTY)
                    moves.push_back(Move(sq, dest, MOVE_QUIET));
                else
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE));

                knightAttacks.clearBit(dest);
            }
        }
    }
}

void Movegen::generateBlackKnightMoves(Array<Move, 256>& moves, const Board& b,
                                       int32_t numAttackers, Bitboard attackingRays,
                                       Bitboard pinnedPieces) {
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Springerzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Springer den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[BLACK_KNIGHT]) {
            // Wenn der Springer gefesselt ist, kann er sich nicht bewegen
            if(pinnedPieces.getBit(sq))
                continue;

            Bitboard knightAttacks = knightAttackBitboard(sq) & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];

            knightAttacks &= attackingRays;
            while(knightAttacks) {
                int32_t dest = knightAttacks.getFirstSetBit();
                if(b.pieces[dest] == EMPTY)
                    moves.push_back(Move(sq, dest, MOVE_QUIET));
                else
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE));

                knightAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[BLACK_KNIGHT]) {
            // Wenn der Springer gefesselt ist, kann er sich nicht bewegen
            if(pinnedPieces.getBit(sq))
                continue;

            // Ansonsten darf er sich frei bewegen
            Bitboard knightAttacks = knightAttackBitboard(sq) & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];
            while(knightAttacks) {
                int32_t dest = knightAttacks.getFirstSetBit();
                if(b.pieces[dest] == EMPTY)
                    moves.push_back(Move(sq, dest, MOVE_QUIET));
                else
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE));

                knightAttacks.clearBit(dest);
            }
        }
    }
}

void Movegen::generateWhiteBishopMoves(Array<Move, 256>& moves, const Board& b,
                                       int32_t numAttackers, Bitboard attackingRays,
                                       Bitboard pinnedPiecesBitboard, int32_t* pinDirections) {
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Läuferzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Läufer den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[WHITE_BISHOP]) {
            // Wenn der Läufer gefesselt ist, kann er sich nicht bewegen
            if(pinnedPiecesBitboard.getBit(sq))
                continue;

            Bitboard bishopAttacks = diagonalAttackBitboard(sq,
                                    b.allPiecesBitboard | b.pieceBitboard[WHITE_KING])
                                    & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];

            bishopAttacks &= attackingRays;

            while(bishopAttacks) {
                int32_t dest = bishopAttacks.getFirstSetBit();
                if(b.pieces[dest] == EMPTY)
                    moves.push_back(Move(sq, dest, MOVE_QUIET));
                else
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE));

                bishopAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[WHITE_BISHOP]) {
            // Wenn der Läufer gefesselt ist, muss er sich in oder gegen die Richtung bewegen, in die er gefesselt ist
            if(pinnedPiecesBitboard.getBit(sq)) {
                int32_t pinDirection = pinDirections[sq];

                // Diagonale von links unten nach rechts oben
                if(pinDirection == SOUTH_WEST || pinDirection == NORTH_EAST) {
                    int n_sq = sq + SOUTH_WEST;

                    // Eine Abbruchbedingung ist hier nicht nötig, da in beide Fesselrichtungen eine Figur stehen muss
                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += SOUTH_WEST;
                    }

                    n_sq = sq + NORTH_EAST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += NORTH_EAST;
                    }
                } else if(pinDirection == SOUTH_EAST || pinDirection == NORTH_WEST) {
                    // Diagonale von rechts unten nach links oben
                    int n_sq = sq + SOUTH_EAST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += SOUTH_EAST;
                    }

                    n_sq = sq + NORTH_WEST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += NORTH_WEST;
                    }
                }
            } else {
                // Ansonsten darf er sich frei bewegen
                Bitboard bishopAttacks = diagonalAttackBitboard(sq,
                                        b.allPiecesBitboard | b.pieceBitboard[WHITE_KING])
                                        & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];

                while(bishopAttacks) {
                    int32_t dest = bishopAttacks.getFirstSetBit();
                    if(b.pieces[dest] == EMPTY)
                        moves.push_back(Move(sq, dest, MOVE_QUIET));
                    else
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE));

                    bishopAttacks.clearBit(dest);
                }
            }
        }
    }
}

void Movegen::generateBlackBishopMoves(Array<Move, 256>& moves, const Board& b,
                                       int32_t numAttackers, Bitboard attackingRays,
                                       Bitboard pinnedPiecesBitboard, int32_t* pinDirections) {
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Läuferzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Läufer den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[BLACK_BISHOP]) {
            // Wenn der Läufer gefesselt ist, kann er sich nicht bewegen
            if(pinnedPiecesBitboard.getBit(sq))
                continue;

            Bitboard bishopAttacks = diagonalAttackBitboard(sq,
                                    b.allPiecesBitboard | b.pieceBitboard[BLACK_KING])
                                    & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];

            bishopAttacks &= attackingRays;

            while(bishopAttacks) {
                int32_t dest = bishopAttacks.getFirstSetBit();
                if(b.pieces[dest] == EMPTY)
                    moves.push_back(Move(sq, dest, MOVE_QUIET));
                else
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE));

                bishopAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[BLACK_BISHOP]) {
            // Wenn der Läufer gefesselt ist, muss er sich in oder gegen die Richtung bewegen, in die er gefesselt ist
            if(pinnedPiecesBitboard.getBit(sq)) {
                int32_t pinDirection = pinDirections[sq];

                // Diagonale von links unten nach rechts oben
                if(pinDirection == NORTH_EAST || pinDirection == SOUTH_WEST) {
                    int n_sq = sq + NORTH_EAST;

                    // Eine Abbruchbedingung ist hier nicht nötig, da in beide Fesselrichtungen eine Figur stehen muss
                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += NORTH_EAST;
                    }

                    n_sq = sq + SOUTH_WEST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += SOUTH_WEST;
                    }
                } else if(pinDirection == NORTH_WEST || pinDirection == SOUTH_EAST) {
                    // Diagonale von rechts unten nach links oben
                    int n_sq = sq + NORTH_WEST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += NORTH_WEST;
                    }

                    n_sq = sq + SOUTH_EAST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += SOUTH_EAST;
                    }
                }
            } else {
                // Ansonsten darf er sich frei bewegen
                Bitboard bishopAttacks = diagonalAttackBitboard(sq,
                                        b.allPiecesBitboard | b.pieceBitboard[BLACK_KING])
                                        & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];

                while(bishopAttacks) {
                    int32_t dest = bishopAttacks.getFirstSetBit();
                    if(b.pieces[dest] == EMPTY)
                        moves.push_back(Move(sq, dest, MOVE_QUIET));
                    else
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE));

                    bishopAttacks.clearBit(dest);
                }
            }
        }
    }
}

void Movegen::generateWhiteRookMoves(Array<Move, 256>& moves, const Board& b,
                                     int32_t numAttackers, Bitboard attackingRays,
                                     Bitboard pinnedPiecesBitboard, int32_t* pinDirections) {
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Turmzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Turm den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[WHITE_ROOK]) {
            // Wenn der Turm gefesselt ist, kann er sich nicht bewegen
            if(pinnedPiecesBitboard.getBit(sq))
                continue;

            Bitboard rookAttacks = straightAttackBitboard(sq,
                                    b.allPiecesBitboard | b.pieceBitboard[WHITE_KING])
                                    & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];

            rookAttacks &= attackingRays;

            while(rookAttacks) {
                int32_t dest = rookAttacks.getFirstSetBit();
                if(b.pieces[dest] == EMPTY)
                    moves.push_back(Move(sq, dest, MOVE_QUIET));
                else
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE));

                rookAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[WHITE_ROOK]) {
            // Wenn der Turm gefesselt ist, muss er sich in oder gegen die Richtung bewegen, in die er gefesselt ist
            if(pinnedPiecesBitboard.getBit(sq)) {
                int32_t pinDirection = pinDirections[sq];

                // Gerade von oben nach unten
                if(pinDirection == NORTH || pinDirection == SOUTH) {
                    int n_sq = sq + NORTH;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += NORTH;
                    }

                    n_sq = sq + SOUTH;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += SOUTH;
                    }

                } else if(pinDirection == EAST || pinDirection == WEST) {
                    // Gerade von links nach rechts
                    int n_sq = sq + EAST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += EAST;
                    }

                    n_sq = sq + WEST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += WEST;
                    }
                }
            } else {
                // Ansonsten darf er sich frei bewegen
                Bitboard rookAttacks = straightAttackBitboard(sq,
                                        b.allPiecesBitboard | b.pieceBitboard[WHITE_KING])
                                        & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];

                while(rookAttacks) {
                    int32_t dest = rookAttacks.getFirstSetBit();
                    if(b.pieces[dest] == EMPTY)
                        moves.push_back(Move(sq, dest, MOVE_QUIET));
                    else
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE));

                    rookAttacks.clearBit(dest);
                }
            }
        }
    }
}

void Movegen::generateBlackRookMoves(Array<Move, 256>& moves, const Board& b,
                                     int32_t numAttackers, Bitboard attackingRays,
                                     Bitboard pinnedPiecesBitboard, int32_t* pinDirections) {
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Turmzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Turm den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[BLACK_ROOK]) {
            // Wenn der Turm gefesselt ist, kann er sich nicht bewegen
            if(pinnedPiecesBitboard.getBit(sq))
                continue;

            Bitboard rookAttacks = straightAttackBitboard(sq,
                                    b.allPiecesBitboard | b.pieceBitboard[BLACK_KING])
                                    & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];

            rookAttacks &= attackingRays;

            while(rookAttacks) {
                int32_t dest = rookAttacks.getFirstSetBit();
                if(b.pieces[dest] == EMPTY)
                    moves.push_back(Move(sq, dest, MOVE_QUIET));
                else
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE));

                rookAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[BLACK_ROOK]) {
            // Wenn der Turm gefesselt ist, muss er sich in oder gegen die Richtung bewegen, in die er gefesselt ist
            if(pinnedPiecesBitboard.getBit(sq)) {
                int32_t pinDirection = pinDirections[sq];

                // Gerade von oben nach unten
                if(pinDirection == NORTH || pinDirection == SOUTH) {
                    int n_sq = sq + NORTH;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += NORTH;
                    }

                    n_sq = sq + SOUTH;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += SOUTH;
                    }

                } else if(pinDirection == EAST || pinDirection == WEST) {
                    // Gerade von links nach rechts
                    int n_sq = sq + EAST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += EAST;
                    }

                    n_sq = sq + WEST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += WEST;
                    }
                }
            } else {
                // Ansonsten darf er sich frei bewegen
                Bitboard rookAttacks = straightAttackBitboard(sq,
                                        b.allPiecesBitboard | b.pieceBitboard[BLACK_KING])
                                        & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];

                while(rookAttacks) {
                    int32_t dest = rookAttacks.getFirstSetBit();
                    if(b.pieces[dest] == EMPTY)
                        moves.push_back(Move(sq, dest, MOVE_QUIET));
                    else
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE));

                    rookAttacks.clearBit(dest);
                }
            }
        }
    }
}

void Movegen::generateWhiteQueenMoves(Array<Move, 256>& moves, const Board& b,
                                      int32_t numAttackers, Bitboard attackingRays,
                                      Bitboard pinnedPiecesBitboard, int32_t* pinDirections) {
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Damenzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann die Dame den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[WHITE_QUEEN]) {
            // Wenn die Dame gefesselt ist, kann sie sich nicht bewegen
            if(pinnedPiecesBitboard.getBit(sq))
                continue;

            // Diagonal
            Bitboard queenAttacks = diagonalAttackBitboard(sq,
                                    b.allPiecesBitboard | b.pieceBitboard[WHITE_KING])
                                    & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];

            // Gerade
            queenAttacks |= straightAttackBitboard(sq,
                                    b.allPiecesBitboard | b.pieceBitboard[WHITE_KING])
                                    & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];

            queenAttacks &= attackingRays;

            while(queenAttacks) {
                int32_t dest = queenAttacks.getFirstSetBit();
                if(b.pieces[dest] == EMPTY)
                    moves.push_back(Move(sq, dest, MOVE_QUIET));
                else
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE));

                queenAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[WHITE_QUEEN]) {
            // Wenn die Dame gefesselt ist, muss sie sich in oder gegen die Richtung bewegen, in die sie gefesselt ist
            if(pinnedPiecesBitboard.getBit(sq)) {
                int32_t pinDirection = pinDirections[sq];

                // Diagonal von links oben nach rechts unten
                if(pinDirection == NORTH_WEST || pinDirection == SOUTH_EAST) {
                    int n_sq = sq + NORTH_WEST;

                    // Eine Abbruchbedingung ist hier nicht nötig, da in beide Fesselrichtungen eine Figur stehen muss
                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += NORTH_WEST;
                    }

                    n_sq = sq + SOUTH_EAST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += SOUTH_EAST;
                    }
                } else if(pinDirection == NORTH_EAST || pinDirection == SOUTH_WEST) {
                    int n_sq = sq + NORTH_EAST;

                    while(true) {
                        // Diagonal von rechts oben nach links unten
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;

                        n_sq += NORTH_EAST;
                    }

                    n_sq = sq + SOUTH_WEST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;

                        n_sq += SOUTH_WEST;
                    }
                } else if(pinDirection == NORTH || pinDirection == SOUTH) {
                    // Gerade von oben nach unten
                    int n_sq = sq + NORTH;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else
                            break;

                        n_sq += NORTH;
                    }

                    n_sq = sq + SOUTH;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else
                            break;

                        n_sq += SOUTH;
                    }
                } else if(pinDirection == EAST || pinDirection == WEST) {
                    // Gerade von links nach rechts
                    int n_sq = sq + EAST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else
                            break;

                        n_sq += EAST;
                    }

                    n_sq = sq + WEST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else
                            break;

                        n_sq += WEST;
                    }
                }
            } else {
                // Ansonsten darf sie sich frei bewegen

                // Diagonal
                Bitboard queenAttacks = diagonalAttackBitboard(sq,
                                        b.allPiecesBitboard | b.pieceBitboard[WHITE_KING])
                                        & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];

                // Gerade
                queenAttacks |= straightAttackBitboard(sq,
                                        b.allPiecesBitboard | b.pieceBitboard[WHITE_KING])
                                        & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];

                while(queenAttacks) {
                    int32_t dest = queenAttacks.getFirstSetBit();
                    if(b.pieces[dest] == EMPTY)
                        moves.push_back(Move(sq, dest, MOVE_QUIET));
                    else
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE));

                    queenAttacks.clearBit(dest);
                }
            }
        }
    }
}

void Movegen::generateBlackQueenMoves(Array<Move, 256>& moves, const Board& b,
                                      int32_t numAttackers, Bitboard attackingRays,
                                      Bitboard pinnedPiecesBitboard, int32_t* pinDirections) {
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Damenzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann die Dame den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[BLACK_QUEEN]) {
            // Wenn die Dame gefesselt ist, kann sie sich nicht bewegen
            if(pinnedPiecesBitboard.getBit(sq))
                continue;

            // Diagonal
            Bitboard queenAttacks = diagonalAttackBitboard(sq,
                                    b.allPiecesBitboard | b.pieceBitboard[BLACK_KING])
                                    & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];

            // Gerade
            queenAttacks |= straightAttackBitboard(sq,
                                    b.allPiecesBitboard | b.pieceBitboard[BLACK_KING])
                                    & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];

            queenAttacks &= attackingRays;

            while(queenAttacks) {
                int32_t dest = queenAttacks.getFirstSetBit();
                if(b.pieces[dest] == EMPTY)
                    moves.push_back(Move(sq, dest, MOVE_QUIET));
                else
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE));

                queenAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[BLACK_QUEEN]) {
            // Wenn die Dame gefesselt ist, muss sie sich in oder gegen die Richtung bewegen, in die sie gefesselt ist
            if(pinnedPiecesBitboard.getBit(sq)) {
                int32_t pinDirection = pinDirections[sq];

                // Diagonal von links oben nach rechts unten
                if(pinDirection == NORTH_WEST || pinDirection == SOUTH_EAST) {
                    int n_sq = sq + NORTH_WEST;

                    // Eine Abbruchbedingung ist hier nicht nötig, da in beide Fesselrichtungen eine Figur stehen muss
                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += NORTH_WEST;
                    }

                    n_sq = sq + SOUTH_EAST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += SOUTH_EAST;
                    }
                } else if(pinDirection == NORTH_EAST || pinDirection == SOUTH_WEST) {
                    int n_sq = sq + NORTH_EAST;

                    while(true) {
                        // Diagonal von rechts oben nach links unten
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;

                        n_sq += NORTH_EAST;
                    }

                    n_sq = sq + SOUTH_WEST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;

                        n_sq += SOUTH_WEST;
                    }
                } else if(pinDirection == NORTH || pinDirection == SOUTH) {
                    // Gerade von oben nach unten
                    int n_sq = sq + NORTH;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else
                            break;

                        n_sq += NORTH;
                    }

                    n_sq = sq + SOUTH;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else
                            break;

                        n_sq += SOUTH;
                    }
                } else if(pinDirection == EAST || pinDirection == WEST) {
                    // Gerade von links nach rechts
                    int n_sq = sq + EAST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else
                            break;

                        n_sq += EAST;
                    }

                    n_sq = sq + WEST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY)
                            moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else
                            break;

                        n_sq += WEST;
                    }
                }
            } else {
                // Ansonsten darf sie sich frei bewegen

                // Diagonal
                Bitboard queenAttacks = diagonalAttackBitboard(sq,
                                        b.allPiecesBitboard | b.pieceBitboard[BLACK_KING])
                                        & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];

                // Gerade
                queenAttacks |= straightAttackBitboard(sq,
                                        b.allPiecesBitboard | b.pieceBitboard[BLACK_KING])
                                        & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];

                while(queenAttacks) {
                    int32_t dest = queenAttacks.getFirstSetBit();
                    if(b.pieces[dest] == EMPTY)
                        moves.push_back(Move(sq, dest, MOVE_QUIET));
                    else
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE));

                    queenAttacks.clearBit(dest);
                }
            }
        }
    }
}

void Movegen::generateWhiteKingMoves(Array<Move, 256>& moves, const Board& b,
                                     Bitboard attackedSquares) {

    int sq = b.pieceList[WHITE_KING].front();
    
    Bitboard kingAttacks = kingAttackBitboard(sq) & ~b.whitePiecesBitboard & ~b.blackAttackBitboard;

    while(kingAttacks) {
        int32_t dest = kingAttacks.getFirstSetBit();
        if(b.pieces[dest] == EMPTY)
            moves.push_back(Move(sq, dest, MOVE_QUIET));
        else
            moves.push_back(Move(sq, dest, MOVE_CAPTURE));

        kingAttacks.clearBit(dest);
    }

    if(!attackedSquares.getBit(E1)) {
        if(b.castlingPermission & WHITE_KINGSIDE_CASTLE) {
            if(b.pieces[F1] == EMPTY && b.pieces[G1] == EMPTY &&
                !attackedSquares.getBit(F1) && !attackedSquares.getBit(G1)) {
                moves.push_back(Move(E1, G1, MOVE_KINGSIDE_CASTLE));
            }
        }

        if(b.castlingPermission & WHITE_QUEENSIDE_CASTLE) {
            if(b.pieces[D1] == EMPTY && b.pieces[C1] == EMPTY && b.pieces[B1] == EMPTY &&
                !attackedSquares.getBit(D1) && !attackedSquares.getBit(C1)) {
                moves.push_back(Move(E1, C1, MOVE_QUEENSIDE_CASTLE));
            }
        }
    }
}

void Movegen::generateBlackKingMoves(Array<Move, 256>& moves, const Board& b,
                                     Bitboard attackedSquares) {
    int sq = b.pieceList[BLACK_KING].front();
    
    Bitboard kingAttacks = kingAttackBitboard(sq) & ~b.blackPiecesBitboard & ~b.whiteAttackBitboard;

    while(kingAttacks) {
        int32_t dest = kingAttacks.getFirstSetBit();
        if(b.pieces[dest] == EMPTY)
            moves.push_back(Move(sq, dest, MOVE_QUIET));
        else
            moves.push_back(Move(sq, dest, MOVE_CAPTURE));

        kingAttacks.clearBit(dest);
    }

    if(!attackedSquares.getBit(E8)) {
        if(b.castlingPermission & BLACK_KINGSIDE_CASTLE) {
            if(b.pieces[F8] == EMPTY && b.pieces[G8] == EMPTY &&
                !attackedSquares.getBit(F8) && !attackedSquares.getBit(G8)) {
                moves.push_back(Move(E8, G8, MOVE_KINGSIDE_CASTLE));
            }
        }

        if(b.castlingPermission & BLACK_QUEENSIDE_CASTLE) {
            if(b.pieces[D8] == EMPTY && b.pieces[C8] == EMPTY && b.pieces[B8] == EMPTY &&
                !attackedSquares.getBit(D8) && !attackedSquares.getBit(C8)) {
                moves.push_back(Move(E8, C8, MOVE_QUEENSIDE_CASTLE));
            }
        }
    }
}

void Movegen::generateWhitePawnCaptures(Array<Move, 256>& moves, const Board& b,
                                     int32_t numAttackers, Bitboard attackingRays,
                                     Bitboard pinnedPiecesBitboard, int32_t* pinDirections) {
    
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Bauernzug kann legal sein
    if(numAttackers > 1)
        return;

    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Bauer den Angreifer schlagen oder den König schützen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[WHITE_PAWN]) {
            int32_t rank = SQ2R(sq);

            int32_t destLeft = sq + NORTH_WEST;
            int32_t destRight = sq + NORTH_EAST;

            int32_t file = SQ2F(sq);

            bool pinned = pinnedPiecesBitboard.getBit(sq);

            // Wenn der Bauer gefesselt ist, kann er sich nicht bewegen
            if(pinned)
                continue;

            // En-Passant Züge befreien den König nur aus dem Schach, wenn der geschlagene Bauer der Angreifer ist
            if(b.enPassantSquare != NO_SQ) {
                int32_t captureSquare = b.enPassantSquare + SOUTH;

                if(b.enPassantSquare == destLeft && file != FILE_A) {
                    if(attackingRays.getBit(captureSquare))
                        moves.push_back(Move(sq, destLeft, MOVE_EN_PASSANT));
                } else if(b.enPassantSquare == destRight && file != FILE_H) {
                    if(attackingRays.getBit(captureSquare))
                        moves.push_back(Move(sq, destRight, MOVE_EN_PASSANT));
                }
            }
            
            // Wenn der Bauer den Angreifer schlagen kann, ist der Zug legal
            Bitboard pawnAttacks = pawnAttackBitboard(sq, WHITE) & attackingRays & b.blackPiecesBitboard;

            while(pawnAttacks) {
                int32_t dest = pawnAttacks.getFirstSetBit();
                // Promotion
                if(rank == RANK_7) {
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                } else
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE));

                pawnAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[WHITE_PAWN]) {
            int32_t rank = SQ2R(sq);

            int32_t destLeft = sq + NORTH_WEST;
            int32_t destRight = sq + NORTH_EAST;

            int32_t file = SQ2F(sq);

            bool pinned = pinnedPiecesBitboard.getBit(sq);

            // En-Passant Züge entfernen 2 Figuren aus einer Reihe, daher muss immer überprüft werden, ob der König nach dem Zug im Schach stände
            // -> Ein En-Passant Zug mit einem ungefesselten Bauer kann illegal sein
            if(b.enPassantSquare != NO_SQ) {
                int32_t captureSquare = b.enPassantSquare + SOUTH;
                int32_t kingSquare = b.pieceList[WHITE_KING].front();

                if(b.enPassantSquare == destLeft && file != FILE_A) {
                    Bitboard enemyRooksAndQueens = b.pieceBitboard[BLACK_ROOK] | b.pieceBitboard[BLACK_QUEEN];
                    Bitboard allPiecesAfterMove = b.allPiecesBitboard | b.pieceBitboard[BLACK_KING];
                    allPiecesAfterMove.clearBit(sq);
                    allPiecesAfterMove.clearBit(captureSquare);
                    allPiecesAfterMove.setBit(destLeft);

                    if(pinned) {
                        int32_t pinDir = pinDirections[sq];
                        if(pinDir == NORTH_WEST || pinDir == SOUTH_EAST)
                            moves.push_back(Move(sq, destLeft, MOVE_EN_PASSANT));
                    } else {
                        if(SQ2R(kingSquare) != RANK_5 ||
                           !(straightAttackBitboard(kingSquare, allPiecesAfterMove) & enemyRooksAndQueens))
                            moves.push_back(Move(sq, destLeft, MOVE_EN_PASSANT));
                    }

                } else if(b.enPassantSquare == destRight && file != FILE_H) {
                    Bitboard enemyRooksAndQueens = b.pieceBitboard[BLACK_ROOK] | b.pieceBitboard[BLACK_QUEEN];
                    Bitboard allPiecesAfterMove = b.allPiecesBitboard | b.pieceBitboard[BLACK_KING];
                    allPiecesAfterMove.clearBit(sq);
                    allPiecesAfterMove.clearBit(captureSquare);
                    allPiecesAfterMove.setBit(destRight);

                    if(pinned) {
                        int32_t pinDir = pinDirections[sq];
                        if(pinDir == NORTH_EAST || pinDir == SOUTH_WEST)
                            moves.push_back(Move(sq, destRight, MOVE_EN_PASSANT));
                    } else {
                        if(SQ2R(kingSquare) != RANK_5 ||
                           !(straightAttackBitboard(kingSquare, allPiecesAfterMove) & enemyRooksAndQueens))
                            moves.push_back(Move(sq, destRight, MOVE_EN_PASSANT));
                    }
                }
            }

            // Überprüfe, ob der Bauer horizontal oder diagonal gefesselt ist
            // Wenn ja, kann er sich nur bewegen, wenn er den Angreifer schlägt(diagonal)
            if(pinned) {
                int32_t pinDir = pinDirections[sq];
                if(pinDir == SOUTH_EAST || pinDir == NORTH_WEST) {
                    if(b.pieces[destLeft] != EMPTY && (b.pieces[destLeft] & COLOR_MASK) == BLACK) {
                        // Promotion
                        if(rank == RANK_7) {
                                moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                                moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                                moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                                moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                        } else
                            moves.push_back(Move(sq, destLeft, MOVE_CAPTURE));
                    }
                } else if(pinDir == SOUTH_WEST || pinDir == NORTH_EAST) {
                    if(b.pieces[destRight] != EMPTY && (b.pieces[destRight] & COLOR_MASK) == BLACK) {
                        // Promotion
                        if(rank == RANK_7) {
                                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                        } else
                            moves.push_back(Move(sq, destRight, MOVE_CAPTURE));
                    }
                }
                
                if(!(pinDir == NORTH || pinDir == SOUTH))
                    continue;
            }
            
            // Schlagzüge
            // sind nur möglich, wenn der Bauer nicht gefesselt ist
            if(!pinned) {
                Bitboard pawnAttacks = pawnAttackBitboard(sq, WHITE) & b.blackPiecesBitboard;

                while(pawnAttacks) {
                    int32_t dest = pawnAttacks.getFirstSetBit();
                    // Promotion
                    if(rank == RANK_7) {
                            moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                            moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                            moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                            moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                    } else
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE));

                    pawnAttacks.clearBit(dest);
                }
            }
        }
    }
}

void Movegen::generateBlackPawnCaptures(Array<Move, 256>& moves, const Board& b,
                                     int32_t numAttackers, Bitboard attackingRays,
                                     Bitboard pinnedPiecesBitboard, int32_t* pinDirections) {
    
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Bauerzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Bauer den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[BLACK_PAWN]) {
            int32_t rank = SQ2R(sq);

            int32_t destLeft = sq + SOUTH_WEST;
            int32_t destRight = sq + SOUTH_EAST;

            int32_t file = SQ2F(sq);

            bool pinned = pinnedPiecesBitboard.getBit(sq);

            // Wenn der Bauer gefesselt ist, kann er sich nicht bewegen
            if(pinned)
                continue;
            
            // En-Passant Züge befreien den König nur aus dem Schach, wenn der geschlagene Bauer der Angreifer ist
            if(b.enPassantSquare != NO_SQ) {
                int32_t captureSquare = b.enPassantSquare + NORTH;

                if(b.enPassantSquare == destLeft && file != FILE_A) {
                    if(attackingRays.getBit(captureSquare))
                        moves.push_back(Move(sq, destLeft, MOVE_EN_PASSANT));
                } else if(b.enPassantSquare == destRight && file != FILE_H) {
                    if(attackingRays.getBit(captureSquare))
                        moves.push_back(Move(sq, destRight, MOVE_EN_PASSANT));
                }
            }
            
            // Wenn der Bauer den Angreifer schlagen kann, ist der Zug legal
            Bitboard pawnAttacks = pawnAttackBitboard(sq, BLACK) & attackingRays & b.whitePiecesBitboard;

            while(pawnAttacks) {
                int32_t dest = pawnAttacks.getFirstSetBit();
                // Promotion
                if(rank == RANK_2) {
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                } else
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE));

                pawnAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[BLACK_PAWN]) {
            int32_t rank = SQ2R(sq);

            int32_t destLeft = sq + SOUTH_WEST;
            int32_t destRight = sq + SOUTH_EAST;

            int32_t file = SQ2F(sq);

            bool pinned = pinnedPiecesBitboard.getBit(sq);

            // En-Passant Züge entfernen 2 Figuren aus einer Reihe, daher muss immer überprüft werden, ob der König nach dem Zug im Schach stände
            // -> Ein En-Passant Zug mit einem ungefesselten Bauer kann illegal sein
            if(b.enPassantSquare != NO_SQ) {
                int32_t captureSquare = b.enPassantSquare + NORTH;
                int32_t kingSquare = b.pieceList[BLACK_KING].front();

                if(b.enPassantSquare == destLeft && file != FILE_A) {
                    Bitboard enemyRooksAndQueens = b.pieceBitboard[WHITE_ROOK] | b.pieceBitboard[WHITE_QUEEN];
                    Bitboard allPiecesAfterMove = b.allPiecesBitboard | b.pieceBitboard[WHITE_KING];
                    allPiecesAfterMove.clearBit(sq);
                    allPiecesAfterMove.clearBit(captureSquare);
                    allPiecesAfterMove.setBit(destLeft);

                    if(pinned) {
                        int32_t pinDir = pinDirections[sq];
                        if(pinDir == SOUTH_WEST || pinDir == NORTH_EAST)
                            moves.push_back(Move(sq, destLeft, MOVE_EN_PASSANT));
                    } else {
                        if(SQ2R(kingSquare) != RANK_4 ||
                           !(straightAttackBitboard(kingSquare, allPiecesAfterMove) & enemyRooksAndQueens))
                            moves.push_back(Move(sq, destLeft, MOVE_EN_PASSANT));
                    }

                } else if(b.enPassantSquare == destRight && file != FILE_H) {
                    Bitboard enemyRooksAndQueens = b.pieceBitboard[WHITE_ROOK] | b.pieceBitboard[WHITE_QUEEN];
                    Bitboard allPiecesAfterMove = b.allPiecesBitboard | b.pieceBitboard[WHITE_KING];
                    allPiecesAfterMove.clearBit(sq);
                    allPiecesAfterMove.clearBit(captureSquare);
                    allPiecesAfterMove.setBit(destRight);

                    if(pinned) {
                        int32_t pinDir = pinDirections[sq];
                        if(pinDir == SOUTH_EAST || pinDir == NORTH_WEST)
                            moves.push_back(Move(sq, destRight, MOVE_EN_PASSANT));
                    } else {
                        if(SQ2R(kingSquare) != RANK_4 ||
                           !(straightAttackBitboard(kingSquare, allPiecesAfterMove) & enemyRooksAndQueens))
                            moves.push_back(Move(sq, destRight, MOVE_EN_PASSANT));
                    }
                }
            }

            // Überprüfe, ob der Bauer horizontal oder diagonal gefesselt ist
            // Wenn ja, kann er sich nur bewegen, wenn er den Angreifer schlägt(diagonal)
            if(pinned) {
                int32_t pinDir = pinDirections[sq];
                
                if(pinDir == NORTH_EAST || pinDir == SOUTH_WEST) {
                    if(b.pieces[destLeft] != EMPTY && (b.pieces[destLeft] & COLOR_MASK) == WHITE) {
                        // Promotion
                        if(rank == RANK_2) {
                            moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                            moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                            moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                            moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                        } else
                            moves.push_back(Move(sq, destLeft, MOVE_CAPTURE));
                    }
                } else if(pinDir == NORTH_WEST || pinDir == SOUTH_EAST) {
                    if(b.pieces[destRight] != EMPTY && (b.pieces[destRight] & COLOR_MASK) == WHITE) {
                        // Promotion
                        if(rank == RANK_2) {
                            moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                            moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                            moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                            moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                        } else
                            moves.push_back(Move(sq, destRight, MOVE_CAPTURE));
                    }
                }
                
                if(!(pinDir == NORTH || pinDir == SOUTH))
                    continue;
            }
            
            // Schlagzüge
            // sind nur möglich, wenn der Bauer nicht gefesselt ist
            if(!pinned) {
                Bitboard pawnAttacks = pawnAttackBitboard(sq, BLACK) & b.whitePiecesBitboard;

                while(pawnAttacks) {
                    int32_t dest = pawnAttacks.getFirstSetBit();
                    // Promotion
                    if(rank == RANK_2) {
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                    } else
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE));

                    pawnAttacks.clearBit(dest);
                }
            }
        }
    }
}

void Movegen::generateWhiteKnightCaptures(Array<Move, 256>& moves, const Board& b,
                                       int32_t numAttackers, Bitboard attackingRays,
                                       Bitboard pinnedPieces) {

    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Springerzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Springer den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[WHITE_KNIGHT]) {
            // Wenn der Springer gefesselt ist, kann er sich nicht bewegen
            if(pinnedPieces.getBit(sq))
                continue;

            Bitboard knightAttacks = knightAttackBitboard(sq) & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING]
                                        & attackingRays & b.blackPiecesBitboard;
            while(knightAttacks) {
                int32_t dest = knightAttacks.getFirstSetBit();
                moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                knightAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[WHITE_KNIGHT]) {
            // Wenn der Springer gefesselt ist, kann er sich nicht bewegen
            if(pinnedPieces.getBit(sq))
                continue;

            // Ansonsten darf er sich frei bewegen
            Bitboard knightAttacks = knightAttackBitboard(sq) & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING]
                                        & b.blackPiecesBitboard;
            while(knightAttacks) {
                int32_t dest = knightAttacks.getFirstSetBit();
                moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                knightAttacks.clearBit(dest);
            }
        }
    }
}

void Movegen::generateBlackKnightCaptures(Array<Move, 256>& moves, const Board& b,
                                       int32_t numAttackers, Bitboard attackingRays,
                                       Bitboard pinnedPieces) {

    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Springerzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Springer den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[BLACK_KNIGHT]) {
            // Wenn der Springer gefesselt ist, kann er sich nicht bewegen
            if(pinnedPieces.getBit(sq))
                continue;

            Bitboard knightAttacks = knightAttackBitboard(sq) & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING]
                                        & attackingRays & b.whitePiecesBitboard;
            while(knightAttacks) {
                int32_t dest = knightAttacks.getFirstSetBit();
                moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                knightAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[BLACK_KNIGHT]) {
            // Wenn der Springer gefesselt ist, kann er sich nicht bewegen
            if(pinnedPieces.getBit(sq))
                continue;

            // Ansonsten darf er sich frei bewegen
            Bitboard knightAttacks = knightAttackBitboard(sq) & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING]
                                        & b.whitePiecesBitboard;
            while(knightAttacks) {
                int32_t dest = knightAttacks.getFirstSetBit();
                moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                knightAttacks.clearBit(dest);
            }
        }
    }
}

void Movegen::generateWhiteBishopCaptures(Array<Move, 256>& moves, const Board& b,
                                       int32_t numAttackers, Bitboard attackingRays,
                                       Bitboard pinnedPiecesBitboard, int32_t* pinDirections) {
    
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Läuferzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Läufer den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[WHITE_BISHOP]) {
            // Wenn der Läufer gefesselt ist, kann er sich nicht bewegen
            if(pinnedPiecesBitboard.getBit(sq))
                continue;

            Bitboard bishopAttacks = diagonalAttackBitboard(sq,
                                    b.allPiecesBitboard | b.pieceBitboard[WHITE_KING])
                                    & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];

            bishopAttacks &= attackingRays & b.blackPiecesBitboard;

            while(bishopAttacks) {
                int32_t dest = bishopAttacks.getFirstSetBit();
                moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                bishopAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[WHITE_BISHOP]) {
            // Wenn der Läufer gefesselt ist, muss er sich in oder gegen die Richtung bewegen, in die er gefesselt ist
            if(pinnedPiecesBitboard.getBit(sq)) {
                int32_t pinDirection = pinDirections[sq];

                // Diagonale von links unten nach rechts oben
                if(pinDirection == SOUTH_WEST || pinDirection == NORTH_EAST) {
                    int n_sq = sq + SOUTH_WEST;

                    // Eine Abbruchbedingung ist hier nicht nötig, da in beide Fesselrichtungen eine Figur stehen muss
                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += SOUTH_WEST;
                    }

                    n_sq = sq + NORTH_EAST;

                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += NORTH_EAST;
                    }
                } else if(pinDirection == SOUTH_EAST || pinDirection == NORTH_WEST) {
                    // Diagonale von rechts unten nach links oben
                    int n_sq = sq + SOUTH_EAST;

                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += SOUTH_EAST;
                    }

                    n_sq = sq + NORTH_WEST;

                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += NORTH_WEST;
                    }
                }
            } else {
                // Ansonsten darf er sich frei bewegen
                Bitboard bishopAttacks = diagonalAttackBitboard(sq,
                                        b.allPiecesBitboard | b.pieceBitboard[WHITE_KING])
                                        & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING]
                                        & b.blackPiecesBitboard;

                while(bishopAttacks) {
                    int32_t dest = bishopAttacks.getFirstSetBit();
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                    bishopAttacks.clearBit(dest);
                }
            }
        }
    }
}

void Movegen::generateBlackBishopCaptures(Array<Move, 256>& moves, const Board& b,
                                       int32_t numAttackers, Bitboard attackingRays,
                                       Bitboard pinnedPiecesBitboard, int32_t* pinDirections) {
    
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Läuferzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Läufer den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[BLACK_BISHOP]) {
            // Wenn der Läufer gefesselt ist, kann er sich nicht bewegen
            if(pinnedPiecesBitboard.getBit(sq))
                continue;

            Bitboard bishopAttacks = diagonalAttackBitboard(sq,
                                    b.allPiecesBitboard | b.pieceBitboard[BLACK_KING])
                                    & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];

            bishopAttacks &= attackingRays & b.whitePiecesBitboard;

            while(bishopAttacks) {
                int32_t dest = bishopAttacks.getFirstSetBit();
                moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                bishopAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[BLACK_BISHOP]) {
            // Wenn der Läufer gefesselt ist, muss er sich in oder gegen die Richtung bewegen, in die er gefesselt ist
            if(pinnedPiecesBitboard.getBit(sq)) {
                int32_t pinDirection = pinDirections[sq];

                // Diagonale von links unten nach rechts oben
                if(pinDirection == NORTH_EAST || pinDirection == SOUTH_WEST) {
                    int n_sq = sq + NORTH_EAST;

                    // Eine Abbruchbedingung ist hier nicht nötig, da in beide Fesselrichtungen eine Figur stehen muss
                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += NORTH_EAST;
                    }

                    n_sq = sq + SOUTH_WEST;

                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += SOUTH_WEST;
                    }
                } else if(pinDirection == NORTH_WEST || pinDirection == SOUTH_EAST) {
                    // Diagonale von rechts unten nach links oben
                    int n_sq = sq + NORTH_WEST;

                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += NORTH_WEST;
                    }

                    n_sq = sq + SOUTH_EAST;

                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += SOUTH_EAST;
                    }
                }
            } else {
                // Ansonsten darf er sich frei bewegen
                Bitboard bishopAttacks = diagonalAttackBitboard(sq,
                                        b.allPiecesBitboard | b.pieceBitboard[BLACK_KING])
                                        & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING]
                                        & b.whitePiecesBitboard;

                while(bishopAttacks) {
                    int32_t dest = bishopAttacks.getFirstSetBit();
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                    bishopAttacks.clearBit(dest);
                }
            }
        }
    }
}

void Movegen::generateWhiteRookCaptures(Array<Move, 256>& moves, const Board& b,
                                     int32_t numAttackers, Bitboard attackingRays,
                                     Bitboard pinnedPiecesBitboard, int32_t* pinDirections) {
    
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Turmzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Turm den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[WHITE_ROOK]) {
            // Wenn der Turm gefesselt ist, kann er sich nicht bewegen
            if(pinnedPiecesBitboard.getBit(sq))
                continue;

            Bitboard rookAttacks = straightAttackBitboard(sq,
                                    b.allPiecesBitboard | b.pieceBitboard[WHITE_KING])
                                    & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];

            rookAttacks &= attackingRays & b.blackPiecesBitboard;

            while(rookAttacks) {
                int32_t dest = rookAttacks.getFirstSetBit();
                moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                rookAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[WHITE_ROOK]) {
            // Wenn der Turm gefesselt ist, muss er sich in oder gegen die Richtung bewegen, in die er gefesselt ist
            if(pinnedPiecesBitboard.getBit(sq)) {
                int32_t pinDirection = pinDirections[sq];

                // Gerade von oben nach unten
                if(pinDirection == NORTH || pinDirection == SOUTH) {
                    int n_sq = sq + NORTH;

                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += NORTH;
                    }

                    n_sq = sq + SOUTH;

                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += SOUTH;
                    }

                } else if(pinDirection == EAST || pinDirection == WEST) {
                    // Gerade von links nach rechts
                    int n_sq = sq + EAST;

                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += EAST;
                    }

                    n_sq = sq + WEST;

                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += WEST;
                    }
                }
            } else {
                // Ansonsten darf er sich frei bewegen
                Bitboard rookAttacks = straightAttackBitboard(sq,
                                        b.allPiecesBitboard | b.pieceBitboard[WHITE_KING])
                                        & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING]
                                        & b.blackPiecesBitboard;

                while(rookAttacks) {
                    int32_t dest = rookAttacks.getFirstSetBit();
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                    rookAttacks.clearBit(dest);
                }
            }
        }
    }
}

void Movegen::generateBlackRookCaptures(Array<Move, 256>& moves, const Board& b,
                                     int32_t numAttackers, Bitboard attackingRays,
                                     Bitboard pinnedPiecesBitboard, int32_t* pinDirections) {
    
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Turmzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Turm den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[BLACK_ROOK]) {
            // Wenn der Turm gefesselt ist, kann er sich nicht bewegen
            if(pinnedPiecesBitboard.getBit(sq))
                continue;

            Bitboard rookAttacks = straightAttackBitboard(sq,
                                    b.allPiecesBitboard | b.pieceBitboard[BLACK_KING])
                                    & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];

            rookAttacks &= attackingRays & b.whitePiecesBitboard;

            while(rookAttacks) {
                int32_t dest = rookAttacks.getFirstSetBit();
                moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                rookAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[BLACK_ROOK]) {
            // Wenn der Turm gefesselt ist, muss er sich in oder gegen die Richtung bewegen, in die er gefesselt ist
            if(pinnedPiecesBitboard.getBit(sq)) {
                int32_t pinDirection = pinDirections[sq];

                // Gerade von oben nach unten
                if(pinDirection == NORTH || pinDirection == SOUTH) {
                    int n_sq = sq + NORTH;

                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += NORTH;
                    }

                    n_sq = sq + SOUTH;

                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += SOUTH;
                    }

                } else if(pinDirection == EAST || pinDirection == WEST) {
                    // Gerade von links nach rechts
                    int n_sq = sq + EAST;

                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += EAST;
                    }

                    n_sq = sq + WEST;

                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += WEST;
                    }
                }
            } else {
                // Ansonsten darf er sich frei bewegen
                Bitboard rookAttacks = straightAttackBitboard(sq,
                                        b.allPiecesBitboard | b.pieceBitboard[BLACK_KING])
                                        & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING]
                                        & b.whitePiecesBitboard;

                while(rookAttacks) {
                    int32_t dest = rookAttacks.getFirstSetBit();
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                    rookAttacks.clearBit(dest);
                }
            }
        }
    }
}

void Movegen::generateWhiteQueenCaptures(Array<Move, 256>& moves, const Board& b,
                                     int32_t numAttackers, Bitboard attackingRays,
                                     Bitboard pinnedPiecesBitboard, int32_t* pinDirections) {
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Damenzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann die Dame den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[WHITE_QUEEN]) {
            // Wenn die Dame gefesselt ist, kann sie sich nicht bewegen
            if(pinnedPiecesBitboard.getBit(sq))
                continue;

            // Diagonal
            Bitboard queenAttacks = diagonalAttackBitboard(sq,
                                    b.allPiecesBitboard | b.pieceBitboard[WHITE_KING])
                                    & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];

            // Gerade
            queenAttacks |= straightAttackBitboard(sq,
                                    b.allPiecesBitboard | b.pieceBitboard[WHITE_KING])
                                    & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];

            queenAttacks &= attackingRays & b.blackPiecesBitboard;

            while(queenAttacks) {
                int32_t dest = queenAttacks.getFirstSetBit();
                moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                queenAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[WHITE_QUEEN]) {
            // Wenn die Dame gefesselt ist, muss sie sich in oder gegen die Richtung bewegen, in die sie gefesselt ist
            if(pinnedPiecesBitboard.getBit(sq)) {
                int32_t pinDirection = pinDirections[sq];

                // Diagonal von links oben nach rechts unten
                if(pinDirection == NORTH_WEST || pinDirection == SOUTH_EAST) {
                    int n_sq = sq + NORTH_WEST;

                    // Eine Abbruchbedingung ist hier nicht nötig, da in beide Fesselrichtungen eine Figur stehen muss
                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += NORTH_WEST;
                    }

                    n_sq = sq + SOUTH_EAST;

                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += SOUTH_EAST;
                    }
                } else if(pinDirection == NORTH_EAST || pinDirection == SOUTH_WEST) {
                    int n_sq = sq + NORTH_EAST;

                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += NORTH_EAST;
                    }

                    n_sq = sq + SOUTH_WEST;

                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += SOUTH_WEST;
                    }
                } else if(pinDirection == NORTH || pinDirection == SOUTH) {
                    // Gerade von oben nach unten
                    int n_sq = sq + NORTH;

                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += NORTH;
                    }

                    n_sq = sq + SOUTH;

                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += SOUTH;
                    }
                } else if(pinDirection == EAST || pinDirection == WEST) {
                    // Gerade von links nach rechts
                    int n_sq = sq + EAST;

                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += EAST;
                    }

                    n_sq = sq + WEST;

                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += WEST;
                    }
                }
            } else {
                // Ansonsten darf sie sich frei bewegen

                // Diagonal
                Bitboard queenAttacks = diagonalAttackBitboard(sq,
                                        b.allPiecesBitboard | b.pieceBitboard[WHITE_KING])
                                        & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];

                // Gerade
                queenAttacks |= straightAttackBitboard(sq,
                                        b.allPiecesBitboard | b.pieceBitboard[WHITE_KING])
                                        & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];

                queenAttacks &= b.blackPiecesBitboard;

                while(queenAttacks) {
                    int32_t dest = queenAttacks.getFirstSetBit();
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                    queenAttacks.clearBit(dest);
                }
            }
        }
    }
}

void Movegen::generateBlackQueenCaptures(Array<Move, 256>& moves, const Board& b,
                                     int32_t numAttackers, Bitboard attackingRays,
                                     Bitboard pinnedPiecesBitboard, int32_t* pinDirections) {
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Damenzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann die Dame den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[BLACK_QUEEN]) {
            // Wenn die Dame gefesselt ist, kann sie sich nicht bewegen
            if(pinnedPiecesBitboard.getBit(sq))
                continue;

            // Diagonal
            Bitboard queenAttacks = diagonalAttackBitboard(sq,
                                    b.allPiecesBitboard | b.pieceBitboard[BLACK_KING])
                                    & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];

            // Gerade
            queenAttacks |= straightAttackBitboard(sq,
                                    b.allPiecesBitboard | b.pieceBitboard[BLACK_KING])
                                    & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];

            queenAttacks &= attackingRays & b.whitePiecesBitboard;

            while(queenAttacks) {
                int32_t dest = queenAttacks.getFirstSetBit();
                moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                queenAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[BLACK_QUEEN]) {
            // Wenn die Dame gefesselt ist, muss sie sich in oder gegen die Richtung bewegen, in die sie gefesselt ist
            if(pinnedPiecesBitboard.getBit(sq)) {
                int32_t pinDirection = pinDirections[sq];

                // Diagonal von links oben nach rechts unten
                if(pinDirection == NORTH_WEST || pinDirection == SOUTH_EAST) {
                    int n_sq = sq + NORTH_WEST;

                    // Eine Abbruchbedingung ist hier nicht nötig, da in beide Fesselrichtungen eine Figur stehen muss
                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += NORTH_WEST;
                    }

                    n_sq = sq + SOUTH_EAST;

                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += SOUTH_EAST;
                    }
                } else if(pinDirection == NORTH_EAST || pinDirection == SOUTH_WEST) {
                    int n_sq = sq + NORTH_EAST;

                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += NORTH_EAST;
                    }

                    n_sq = sq + SOUTH_WEST;

                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += SOUTH_WEST;
                    }
                } else if(pinDirection == NORTH || pinDirection == SOUTH) {
                    // Gerade von oben nach unten
                    int n_sq = sq + NORTH;

                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += NORTH;
                    }

                    n_sq = sq + SOUTH;

                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += SOUTH;
                    }
                } else if(pinDirection == EAST || pinDirection == WEST) {
                    // Gerade von links nach rechts
                    int n_sq = sq + EAST;

                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += EAST;
                    }

                    n_sq = sq + WEST;

                    while(true) {
                        if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else if(b.pieces[n_sq] != EMPTY)
                            break;

                        n_sq += WEST;
                    }
                }
            } else {
                // Ansonsten darf sie sich frei bewegen

                // Diagonal
                Bitboard queenAttacks = diagonalAttackBitboard(sq,
                                        b.allPiecesBitboard | b.pieceBitboard[BLACK_KING])
                                        & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];

                // Gerade
                queenAttacks |= straightAttackBitboard(sq,
                                        b.allPiecesBitboard | b.pieceBitboard[BLACK_KING])
                                        & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];

                queenAttacks &= b.whitePiecesBitboard;

                while(queenAttacks) {
                    int32_t dest = queenAttacks.getFirstSetBit();
                    moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                    queenAttacks.clearBit(dest);
                }
            }
        }
    }
}

void Movegen::generateWhiteKingCaptures(Array<Move, 256>& moves, const Board& b) {
    
    int sq = b.pieceList[WHITE_KING].front();
    
    Bitboard kingAttacks = kingAttackBitboard(sq) & ~b.whitePiecesBitboard & ~b.blackAttackBitboard
                            & b.blackPiecesBitboard;

    while(kingAttacks) {
        int32_t dest = kingAttacks.getFirstSetBit();
        moves.push_back(Move(sq, dest, MOVE_CAPTURE));
        kingAttacks.clearBit(dest);
    }
}

void Movegen::generateBlackKingCaptures(Array<Move, 256>& moves, const Board& b) {
    
    int sq = b.pieceList[BLACK_KING].front();
    
    Bitboard kingAttacks = kingAttackBitboard(sq) & ~b.blackPiecesBitboard & ~b.whiteAttackBitboard
                            & b.whitePiecesBitboard;

    while(kingAttacks) {
        int32_t dest = kingAttacks.getFirstSetBit();
        moves.push_back(Move(sq, dest, MOVE_CAPTURE));
        kingAttacks.clearBit(dest);
    }
}