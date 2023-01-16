#include "Movegen.h"

void Movegen::generatePseudoLegalWhitePawnMoves(std::vector<Move>& moves, Board& b) {
    for(int sq : b.pieceList[WHITE_PAWN]) {
        ASSERT(b.pieces[sq] == WHITE_PAWN);
        ASSERT(b.mailbox[sq] != NO_SQ);

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

        int32_t destLeft = FR2SQ(file - 1, rank + 1);
        if(b.mailbox[destLeft] != NO_SQ && b.pieces[destLeft] != EMPTY && (b.pieces[destLeft] & COLOR_MASK) == BLACK) {
            if(rank == RANK_7) {
                moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
            }
            else
                moves.push_back(Move(sq, destLeft, MOVE_CAPTURE));
        }

        int32_t destRight = FR2SQ(file + 1, rank + 1);
        if(b.mailbox[destRight] != NO_SQ && b.pieces[destRight] != EMPTY && (b.pieces[destRight] & COLOR_MASK) == BLACK) {
            if(rank == RANK_7) {
                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
            }
            else
                moves.push_back(Move(sq, destRight, MOVE_CAPTURE));
        }

        if(b.enPassantSquare != NO_SQ) {
            if(b.enPassantSquare == destLeft)
                moves.push_back(Move(sq, destLeft, MOVE_EN_PASSANT));
            else if(b.enPassantSquare == destRight)
                moves.push_back(Move(sq, destRight, MOVE_EN_PASSANT));
        }
    }
}

void Movegen::generatePseudoLegalBlackPawnMoves(std::vector<Move>& moves, Board& b) {
    for(int sq : b.pieceList[BLACK_PAWN]) {
        ASSERT(b.pieces[sq] == BLACK_PAWN);
        ASSERT(b.mailbox[sq] != NO_SQ);

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

        int32_t destLeft = FR2SQ(file - 1, rank - 1);
        if(b.mailbox[destLeft] != NO_SQ && b.pieces[destLeft] != EMPTY && (b.pieces[destLeft] & COLOR_MASK) == WHITE) {
            if(rank == RANK_2) {
                moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
            }
            else
                moves.push_back(Move(sq, destLeft, MOVE_CAPTURE));
        }

        int32_t destRight = FR2SQ(file + 1, rank - 1);
        if(b.mailbox[destRight] != NO_SQ && b.pieces[destRight] != EMPTY && (b.pieces[destRight] & COLOR_MASK) == WHITE) {
            if(rank == RANK_2) {
                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
            }
            else
                moves.push_back(Move(sq, destRight, MOVE_CAPTURE));
        }

        if(b.enPassantSquare != NO_SQ) {
            if(b.enPassantSquare == destLeft)
                moves.push_back(Move(sq, destLeft, MOVE_EN_PASSANT));
            else if(b.enPassantSquare == destRight)
                moves.push_back(Move(sq, destRight, MOVE_EN_PASSANT));
        }
    }
}

void Movegen::generatePseudoLegalWhiteKnightMoves(std::vector<Move>& moves, Board& b) {
    for(int sq : b.pieceList[WHITE_KNIGHT]) {
        ASSERT(b.pieces[sq] == WHITE_KNIGHT);
        ASSERT(b.mailbox[sq] != NO_SQ);

        for(int i = 0; i < 8; i++) {
            int n_sq = sq + KNIGHT_ATTACKS[i];
            if(b.mailbox[n_sq] != NO_SQ) {
                if(b.pieces[n_sq] == EMPTY)
                    moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                else if((b.pieces[n_sq] & COLOR_MASK) == BLACK)
                    moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
            }
        }
    }
}

void Movegen:: generatePseudoLegalBlackKnightMoves(std::vector<Move>& moves, Board& b) {
    for(int sq : b.pieceList[BLACK_KNIGHT]) {
        ASSERT(b.pieces[sq] == BLACK_KNIGHT);
        ASSERT(b.mailbox[sq] != NO_SQ);

        for(int i = 0; i < 8; i++) {
            int n_sq = sq + KNIGHT_ATTACKS[i];
            if(b.mailbox[n_sq] != NO_SQ) {
                if(b.pieces[n_sq] == EMPTY)
                    moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                else if((b.pieces[n_sq] & COLOR_MASK) == WHITE)
                    moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
            }
        }
    }
}

void Movegen::generatePseudoLegalWhiteRookMoves(std::vector<Move>& moves, Board& b) {
    for(int sq : b.pieceList[WHITE_BISHOP]) {
        ASSERT(b.pieces[sq] == WHITE_BISHOP);
        ASSERT(b.mailbox[sq] != NO_SQ);

        for(int i = 0; i < 4; i++) {
            int n_sq = sq + DIAGONAL_ATTACKS[i];
            while(b.mailbox[n_sq] != NO_SQ) {
                if(b.pieces[n_sq] == EMPTY)
                    moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                    moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                    break;
                }
                else
                    break;

                n_sq += DIAGONAL_ATTACKS[i];
            }
        }
    }
}

void Movegen::generatePseudoLegalBlackRookMoves(std::vector<Move>& moves, Board& b) {
    for(int sq : b.pieceList[BLACK_BISHOP]) {
        ASSERT(b.pieces[sq] == BLACK_BISHOP);
        ASSERT(b.mailbox[sq] != NO_SQ);

        for(int i = 0; i < 4; i++) {
            int n_sq = sq + DIAGONAL_ATTACKS[i];
            while(b.mailbox[n_sq] != NO_SQ) {
                if(b.pieces[n_sq] == EMPTY)
                    moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                    moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                    break;
                }
                else
                    break;

                n_sq += DIAGONAL_ATTACKS[i];
            }
        }
    }
}

void Movegen::generatePseudoLegalWhiteBishopMoves(std::vector<Move>& moves, Board& b) {
    for(int sq : b.pieceList[WHITE_ROOK]) {
        ASSERT(b.pieces[sq] == WHITE_ROOK);
        ASSERT(b.mailbox[sq] != NO_SQ);

        for(int i = 0; i < 4; i++) {
            int n_sq = sq + STRAIGHT_ATTACKS[i];
            while(b.mailbox[n_sq] != NO_SQ) {
                if(b.pieces[n_sq] == EMPTY)
                    moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                    moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                    break;
                }
                else
                    break;

                n_sq += STRAIGHT_ATTACKS[i];
            }
        }
    }
}

void Movegen::generatePseudoLegalBlackBishopMoves(std::vector<Move>& moves, Board& b) {
    for(int sq : b.pieceList[BLACK_ROOK]) {
        ASSERT(b.pieces[sq] == BLACK_ROOK);
        ASSERT(b.mailbox[sq] != NO_SQ);

        for(int i = 0; i < 4; i++) {
            int n_sq = sq + STRAIGHT_ATTACKS[i];
            while(b.mailbox[n_sq] != NO_SQ) {
                if(b.pieces[n_sq] == EMPTY)
                    moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                    moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                    break;
                }
                else
                    break;

                n_sq += STRAIGHT_ATTACKS[i];
            }
        }
    }
}

void Movegen::generatePseudoLegalWhiteQueenMoves(std::vector<Move>& moves, Board& b) {
    int32_t QUEEN_ATTACKS[8] = {
        DIAGONAL_ATTACKS[0], DIAGONAL_ATTACKS[1], DIAGONAL_ATTACKS[2], DIAGONAL_ATTACKS[3],
        STRAIGHT_ATTACKS[0], STRAIGHT_ATTACKS[1], STRAIGHT_ATTACKS[2], STRAIGHT_ATTACKS[3]
    };

    for(int sq : b.pieceList[WHITE_QUEEN]) {
        ASSERT(b.pieces[sq] == WHITE_QUEEN);
        ASSERT(b.mailbox[sq] != NO_SQ);

        for(int i = 0; i < 8; i++) {
            int n_sq = sq + QUEEN_ATTACKS[i];
            while(b.mailbox[n_sq] != NO_SQ) {
                if(b.pieces[n_sq] == EMPTY)
                    moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                    moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                    break;
                }
                else
                    break;

                n_sq += QUEEN_ATTACKS[i];
            }
        }
    }
}

void Movegen::generatePseudoLegalBlackQueenMoves(std::vector<Move>& moves, Board& b) {
    int32_t QUEEN_ATTACKS[8] = {
        DIAGONAL_ATTACKS[0], DIAGONAL_ATTACKS[1], DIAGONAL_ATTACKS[2], DIAGONAL_ATTACKS[3],
        STRAIGHT_ATTACKS[0], STRAIGHT_ATTACKS[1], STRAIGHT_ATTACKS[2], STRAIGHT_ATTACKS[3]
    };

    for(int sq : b.pieceList[BLACK_QUEEN]) {
        ASSERT(b.pieces[sq] == BLACK_QUEEN);
        ASSERT(b.mailbox[sq] != NO_SQ);

        for(int i = 0; i < 8; i++) {
            int n_sq = sq + QUEEN_ATTACKS[i];
            while(b.mailbox[n_sq] != NO_SQ) {
                if(b.pieces[n_sq] == EMPTY)
                    moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                    moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                    break;
                }
                else
                    break;

                n_sq += QUEEN_ATTACKS[i];
            }
        }
    }
}

void Movegen::generatePseudoLegalWhiteKingMoves(std::vector<Move>& moves, Board& b) {
    int32_t KING_ATTACKS[8] = {
        DIAGONAL_ATTACKS[0], DIAGONAL_ATTACKS[1], DIAGONAL_ATTACKS[2], DIAGONAL_ATTACKS[3],
        STRAIGHT_ATTACKS[0], STRAIGHT_ATTACKS[1], STRAIGHT_ATTACKS[2], STRAIGHT_ATTACKS[3]
    };

    int sq = b.pieceList[WHITE_KING].front();
    ASSERT(b.pieces[sq] == WHITE_KING);
    ASSERT(b.mailbox[sq] != NO_SQ);

    for(int i = 0; i < 8; i++) {
        int n_sq = sq + KING_ATTACKS[i];
        if(b.mailbox[n_sq] != NO_SQ) {
            if(b.pieces[n_sq] == EMPTY)
                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
            else if((b.pieces[n_sq] & COLOR_MASK) == BLACK)
                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
        }
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

void Movegen::generatePseudoLegalBlackKingMoves(std::vector<Move>& moves, Board& b) {
    int32_t KING_ATTACKS[8] = {
        DIAGONAL_ATTACKS[0], DIAGONAL_ATTACKS[1], DIAGONAL_ATTACKS[2], DIAGONAL_ATTACKS[3],
        STRAIGHT_ATTACKS[0], STRAIGHT_ATTACKS[1], STRAIGHT_ATTACKS[2], STRAIGHT_ATTACKS[3]
    };

    int sq = b.pieceList[BLACK_KING].front();
    ASSERT(b.pieces[sq] == BLACK_KING);
    ASSERT(b.mailbox[sq] != NO_SQ);

    for(int i = 0; i < 8; i++) {
        int n_sq = sq + KING_ATTACKS[i];
        if(b.mailbox[n_sq] != NO_SQ) {
            if(b.pieces[n_sq] == EMPTY)
                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
            else if((b.pieces[n_sq] & COLOR_MASK) == WHITE)
                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
        }
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

void Movegen::generateWhitePawnMoves(std::vector<Move>& moves, Board& b, int32_t numAttackers, Bitboard attackingRays, Bitboard pinnedPiecesBitboard, int32_t* pinDirections) {
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Bauernzug kann legal sein
    if(numAttackers > 1)
        return;

    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Bauer den Angreifer schlagen oder den König schützen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[WHITE_PAWN]) {
            ASSERT(b.pieces[sq] == WHITE_PAWN);
            ASSERT(b.mailbox[sq] != NO_SQ);

            int32_t file = SQ2F(sq);
            int32_t rank = SQ2R(sq);

            int32_t destForw = sq + NORTH;
            int32_t destForw2 = sq + 2 * NORTH;
            int32_t destLeft = sq + NORTH_WEST;
            int32_t destRight = sq + NORTH_EAST;

            bool pinned = pinnedPiecesBitboard & Bitboard(1ULL << b.mailbox[sq]);

            // En passant kann den König schützen, auch wenn er gefesselt ist
            // Bei En Passant Zügen gibt es so viele Ausnahmen und Sonderfälle, dass die Züge einfach aufwendig auf Legalität geprüft werden
            // Weil En Passant Züge so selten sind, ist das kein Problem
            if(b.enPassantSquare != NO_SQ) {
                if(b.enPassantSquare == destLeft) {
                    Move m(sq, destLeft, MOVE_EN_PASSANT);
                    if(b.isLegal(m))
                        moves.push_back(m);
                } else if(b.enPassantSquare == destRight) {
                    Move m(sq, destRight, MOVE_EN_PASSANT);
                    if(b.isLegal(m))
                        moves.push_back(m);
                }
            }

            // Wenn der Bauer gefesselt ist, kann er sich nicht bewegen
            if(pinned)
                continue;
            
            // Wenn der Bauer den Angreifer blockiert, ist der Zug legal
            // Ein blockierende Zug kann kein normaler Schlagzug sein, weil der Bauer sonst nicht gefesselt wäre
            if(attackingRays & Bitboard(1ULL << b.mailbox[destForw]) && b.pieces[destForw] == EMPTY) {
                // Promotion
                if(rank == RANK_7) {
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_QUEEN));
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_ROOK));
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_BISHOP));
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_KNIGHT));
                } else
                    moves.push_back(Move(sq, destForw, MOVE_QUIET));
            } else if(rank == RANK_2 && attackingRays & Bitboard(1ULL << b.mailbox[destForw2]) && b.pieces[destForw2] == EMPTY && b.pieces[destForw] == EMPTY)
                moves.push_back(Move(sq, destForw2, MOVE_DOUBLE_PAWN));
        }
    } else {
        for(int sq : b.pieceList[WHITE_PAWN]) {
            ASSERT(b.pieces[sq] == WHITE_PAWN);
            ASSERT(b.mailbox[sq] != NO_SQ);

            int32_t file = SQ2F(sq);
            int32_t rank = SQ2R(sq);

            int32_t destForw = sq + NORTH;
            int32_t destForw2 = sq + 2 * NORTH;
            int32_t destLeft = sq + NORTH_WEST;
            int32_t destRight = sq + NORTH_EAST;

            bool pinned = pinnedPiecesBitboard & Bitboard(1ULL << b.mailbox[sq]);

            // En passant kann den König schützen, auch wenn er gefesselt ist
            // Bei En Passant Zügen gibt es so viele Ausnahmen und Sonderfälle, dass die Züge einfach aufwendig auf Legalität geprüft werden
            // Weil En Passant Züge so selten sind, ist das kein Problem
            if(b.enPassantSquare != NO_SQ) {
                if(b.enPassantSquare == destLeft) {
                    Move m(sq, destLeft, MOVE_EN_PASSANT);
                    if(b.isLegal(m))
                        moves.push_back(m);
                } else if(b.enPassantSquare == destRight) {
                    Move m(sq, destRight, MOVE_EN_PASSANT);
                    if(b.isLegal(m))
                        moves.push_back(m);
                }
            }

            // Überprüfe, ob der Bauer horizontal oder diagonal gefesselt ist
            // Wenn ja, kann er sich nicht bewegen
            if(pinned) {
                int32_t pinDir = pinDirections[b.mailbox[sq]];
                if(!(pinDir == NORTH ||pinDir == SOUTH))
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
            int32_t nwPiece = b.pieces[destLeft];

            if(nwPiece != EMPTY && (nwPiece & COLOR_MASK) == BLACK && !pinned) {
                // Promotion
                if(rank == RANK_7) {
                        moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                        moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                        moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                        moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                } else
                    moves.push_back(Move(sq, destLeft, MOVE_CAPTURE));
            }

            int32_t nePiece = b.pieces[destRight];

            if(nePiece != EMPTY && (nePiece & COLOR_MASK) == BLACK && !pinned) {
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
    }
}

void Movegen::generateBlackPawnMoves(std::vector<Move>& moves, Board& b, int32_t numAttackers, Bitboard attackingRays, Bitboard pinnedPiecesBitboard, int32_t* pinDirections) {
    
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Bauerzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Bauer den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[BLACK_PAWN]) {
            ASSERT(b.pieces[sq] == BLACK_PAWN);
            ASSERT(b.mailbox[sq] != NO_SQ);

            int32_t file = SQ2F(sq);
            int32_t rank = SQ2R(sq);

            int32_t destForw = sq + SOUTH;
            int32_t destForw2 = sq + 2 * SOUTH;
            int32_t destLeft = sq + SOUTH_WEST;
            int32_t destRight = sq + SOUTH_EAST;

            bool pinned = pinnedPiecesBitboard & Bitboard(1ULL << b.mailbox[sq]);

            // En passant kann den König schützen, auch wenn er gefesselt ist
            // Bei En Passant Zügen gibt es so viele Ausnahmen und Sonderfälle, dass die Züge einfach aufwendig auf Legalität geprüft werden
            // Weil En Passant Züge so selten sind, ist das kein Problem
            if(b.enPassantSquare != NO_SQ) {
                if(b.enPassantSquare == destLeft) {
                    Move m(sq, destLeft, MOVE_EN_PASSANT);
                    if(b.isLegal(m))
                        moves.push_back(m);
                } else if(b.enPassantSquare == destRight) {
                    Move m(sq, destRight, MOVE_EN_PASSANT);
                    if(b.isLegal(m))
                        moves.push_back(m);
                }
            }

            // Wenn der Bauer gefesselt ist, kann er sich nicht bewegen
            if(pinned)
                continue;
            
            // Wenn der Bauer den Angreifer blockiert, ist der Zug legal
            // Ein blockierende Zug kann kein normaler Schlagzug sein, weil der Bauer sonst nicht gefesselt wäre
            if(attackingRays & Bitboard(1ULL << b.mailbox[destForw]) && b.pieces[destForw] == EMPTY) {
                // Promotion
                if(rank == RANK_2) {
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_QUEEN));
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_ROOK));
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_BISHOP));
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_KNIGHT));
                } else
                    moves.push_back(Move(sq, destForw, MOVE_QUIET));
            } else if(rank == RANK_7 && attackingRays & Bitboard(1ULL << b.mailbox[destForw2]) && b.pieces[destForw2] == EMPTY && b.pieces[destForw] == EMPTY) {
                moves.push_back(Move(sq, destForw2, MOVE_DOUBLE_PAWN));
            }
        }
    } else {
        for(int sq : b.pieceList[BLACK_PAWN]) {
            ASSERT(b.pieces[sq] == BLACK_PAWN);
            ASSERT(b.mailbox[sq] != NO_SQ);

            int32_t file = SQ2F(sq);
            int32_t rank = SQ2R(sq);

            int32_t destForw = sq + SOUTH;
            int32_t destForw2 = sq + 2 * SOUTH;
            int32_t destLeft = sq + SOUTH_WEST;
            int32_t destRight = sq + SOUTH_EAST;

            bool pinned = pinnedPiecesBitboard & Bitboard(1ULL << b.mailbox[sq]);

            // En passant kann den König schützen, auch wenn er gefesselt ist
            // Bei En Passant Zügen gibt es so viele Ausnahmen und Sonderfälle, dass die Züge einfach aufwendig auf Legalität geprüft werden
            // Weil En Passant Züge so selten sind, ist das kein Problem
            if(b.enPassantSquare != NO_SQ) {
                if(b.enPassantSquare == destLeft) {
                    Move m(sq, destLeft, MOVE_EN_PASSANT);
                    if(b.isLegal(m))
                        moves.push_back(m);
                } else if(b.enPassantSquare == destRight) {
                    Move m(sq, destRight, MOVE_EN_PASSANT);
                    if(b.isLegal(m))
                        moves.push_back(m);
                }
            }

            // Überprüfe, ob der Bauer horizontal oder diagonal gefesselt ist
            // Wenn ja, kann er sich nicht bewegen
            if(pinned) {
                int32_t pinDir = pinDirections[b.mailbox[sq]];
                if(!(pinDir == NORTH ||pinDir == SOUTH))
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
            int32_t swPiece = b.pieces[destLeft];

            if(swPiece != EMPTY && (swPiece & COLOR_MASK) == WHITE && !pinned) {
                // Promotion
                if(rank == RANK_2) {
                    moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                    moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                    moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                    moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                } else
                    moves.push_back(Move(sq, destLeft, MOVE_CAPTURE));
            }

            int32_t sePiece = b.pieces[destRight];

            if(sePiece != EMPTY && (sePiece & COLOR_MASK) == WHITE && !pinned) {
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
    }
}

void Movegen::generateWhiteKnightMoves(std::vector<Move>& moves, Board& b, int32_t numAttackers, Bitboard attackingRays, Bitboard pinnedPieces) {
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Springerzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Springer den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[WHITE_KNIGHT]) {
            ASSERT(b.pieces[sq] == WHITE_KNIGHT);
            ASSERT(b.mailbox[sq] != NO_SQ);

            // Wenn der Springer gefesselt ist, kann er sich nicht bewegen
            if(pinnedPieces & Bitboard(1ULL << b.mailbox[sq]))
                continue;

            for(int i = 0; i < 8; i++) {
                int n_sq = sq + KNIGHT_ATTACKS[i];
                
                // Wenn der Springer den Angreifer schlagen oder sich dazwischen stellen kann, ist der Zug legal
                if(attackingRays & Bitboard(1ULL << b.mailbox[n_sq]))
                    if(b.pieces[n_sq] == EMPTY)
                        moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                    else
                        moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
            }
        }
    } else {
        for(int sq : b.pieceList[WHITE_KNIGHT]) {
            ASSERT(b.pieces[sq] == WHITE_KNIGHT);
            ASSERT(b.mailbox[sq] != NO_SQ);

            // Wenn der Springer gefesselt ist, kann er sich nicht bewegen
            if(pinnedPieces & Bitboard(1ULL << b.mailbox[sq]))
                continue;

            // Ansonsten darf er sich frei bewegen
            for(int i = 0; i < 8; i++) {
                int n_sq = sq + KNIGHT_ATTACKS[i];
                if(b.mailbox[n_sq] != NO_SQ) {
                    if(b.pieces[n_sq] == EMPTY)
                        moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                    else if((b.pieces[n_sq] & COLOR_MASK) == BLACK)
                        moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                }
            }
        }
    }
}

void Movegen::generateBlackKnightMoves(std::vector<Move>& moves, Board& b, int32_t numAttackers, Bitboard attackingRays, Bitboard pinnedPieces) {
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Springerzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Springer den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[BLACK_KNIGHT]) {
            ASSERT(b.pieces[sq] == BLACK_KNIGHT);
            ASSERT(b.mailbox[sq] != NO_SQ);

            // Wenn der Springer gefesselt ist, kann er sich nicht bewegen
            if(pinnedPieces & Bitboard(1ULL << b.mailbox[sq]))
                continue;

            for(int i = 0; i < 8; i++) {
                int n_sq = sq + KNIGHT_ATTACKS[i];
                
                // Wenn der Springer den Angreifer schlagen oder sich dazwischen stellen kann, ist der Zug legal
                if(attackingRays & Bitboard(1ULL << b.mailbox[n_sq]))
                    if(b.pieces[n_sq] == EMPTY)
                        moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                    else
                        moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
            }
        }
    } else {
        for(int sq : b.pieceList[BLACK_KNIGHT]) {
            ASSERT(b.pieces[sq] == BLACK_KNIGHT);
            ASSERT(b.mailbox[sq] != NO_SQ);

            // Wenn der Springer gefesselt ist, kann er sich nicht bewegen
            if(pinnedPieces & Bitboard(1ULL << b.mailbox[sq]))
                continue;

            // Ansonsten darf er sich frei bewegen
            for(int i = 0; i < 8; i++) {
                int n_sq = sq + KNIGHT_ATTACKS[i];
                if(b.mailbox[n_sq] != NO_SQ) {
                    if(b.pieces[n_sq] == EMPTY)
                        moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                    else if((b.pieces[n_sq] & COLOR_MASK) == WHITE)
                        moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                }
            }
        }
    }
}

void Movegen::generateWhiteBishopMoves(std::vector<Move>& moves, Board& b, int32_t numAttackers, Bitboard attackingRays) {

}

void Movegen::generateBlackBishopMoves(std::vector<Move>& moves, Board& b, int32_t numAttackers, Bitboard attackingRays) {

}

void Movegen::generateWhiteRookMoves(std::vector<Move>& moves, Board& b, int32_t numAttackers, Bitboard attackingRays) {

}

void Movegen::generateBlackRookMoves(std::vector<Move>& moves, Board& b, int32_t numAttackers, Bitboard attackingRays) {

}

void Movegen::generateWhiteQueenMoves(std::vector<Move>& moves, Board& b, int32_t numAttackers, Bitboard attackingRays) {

}

void Movegen::generateBlackQueenMoves(std::vector<Move>& moves, Board& b, int32_t numAttackers, Bitboard attackingRays) {

}

void Movegen::generateWhiteKingMoves(std::vector<Move>& moves, Board& b) {
    int32_t KING_ATTACKS[8] = {
        DIAGONAL_ATTACKS[0], DIAGONAL_ATTACKS[1], DIAGONAL_ATTACKS[2], DIAGONAL_ATTACKS[3],
        STRAIGHT_ATTACKS[0], STRAIGHT_ATTACKS[1], STRAIGHT_ATTACKS[2], STRAIGHT_ATTACKS[3]
    };

    int sq = b.pieceList[WHITE_KING].front();
    ASSERT(b.pieces[sq] == WHITE_KING);
    ASSERT(b.mailbox[sq] != NO_SQ);

    Bitboard allPiecesPlusEnemyKing = b.allPiecesBitboard | b.pieceBitboard[BLACK_KING];

    for(int i = 0; i < 8; i++) {
        int n_sq = sq + KING_ATTACKS[i];
        if(b.mailbox[n_sq] != NO_SQ) {
            if(b.pieces[n_sq] == EMPTY && !b.squareAttacked(n_sq, BLACK, allPiecesPlusEnemyKing))
                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
            else if((b.pieces[n_sq] & COLOR_MASK) == BLACK && !b.squareAttacked(n_sq, BLACK, allPiecesPlusEnemyKing))
                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
        }
    }

    if(b.castlingPermission & WHITE_KINGSIDE_CASTLE) {
        if(b.pieces[F1] == EMPTY && b.pieces[G1] == EMPTY &&
            !b.squareAttacked(F1, BLACK, allPiecesPlusEnemyKing) && !b.squareAttacked(G1, BLACK, allPiecesPlusEnemyKing)) {
            moves.push_back(Move(E1, G1, MOVE_KINGSIDE_CASTLE));
        }
    }

    if(b.castlingPermission & WHITE_QUEENSIDE_CASTLE) {
        if(b.pieces[D1] == EMPTY && b.pieces[C1] == EMPTY && b.pieces[B1] == EMPTY &&
            !b.squareAttacked(D1, BLACK, allPiecesPlusEnemyKing) && !b.squareAttacked(C1, BLACK, allPiecesPlusEnemyKing)) {
            moves.push_back(Move(E1, C1, MOVE_QUEENSIDE_CASTLE));
        }
    }
}

void Movegen::generateBlackKingMoves(std::vector<Move>& moves, Board& b) {
    int32_t KING_ATTACKS[8] = {
        DIAGONAL_ATTACKS[0], DIAGONAL_ATTACKS[1], DIAGONAL_ATTACKS[2], DIAGONAL_ATTACKS[3],
        STRAIGHT_ATTACKS[0], STRAIGHT_ATTACKS[1], STRAIGHT_ATTACKS[2], STRAIGHT_ATTACKS[3]
    };

    int sq = b.pieceList[BLACK_KING].front();
    ASSERT(b.pieces[sq] == BLACK_KING);
    ASSERT(b.mailbox[sq] != NO_SQ);

    Bitboard allPiecesPlusEnemyKing = b.allPiecesBitboard | b.pieceBitboard[WHITE_KING];

    for(int i = 0; i < 8; i++) {
        int n_sq = sq + KING_ATTACKS[i];
        if(b.mailbox[n_sq] != NO_SQ) {
            if(b.pieces[n_sq] == EMPTY && !b.squareAttacked(n_sq, WHITE, allPiecesPlusEnemyKing))
                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
            else if((b.pieces[n_sq] & COLOR_MASK) == WHITE && !b.squareAttacked(n_sq, WHITE, allPiecesPlusEnemyKing))
                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
        }
    }

    if(b.castlingPermission & BLACK_KINGSIDE_CASTLE) {
        if(b.pieces[F8] == EMPTY && b.pieces[G8] == EMPTY &&
            !b.squareAttacked(F8, WHITE, allPiecesPlusEnemyKing) && !b.squareAttacked(G8, WHITE, allPiecesPlusEnemyKing)) {
            moves.push_back(Move(E8, G8, MOVE_KINGSIDE_CASTLE));
        }
    }

    if(b.castlingPermission & BLACK_QUEENSIDE_CASTLE) {
        if(b.pieces[D8] == EMPTY && b.pieces[C8] == EMPTY && b.pieces[B8] == EMPTY &&
            !b.squareAttacked(D8, WHITE, allPiecesPlusEnemyKing) && !b.squareAttacked(C8, WHITE, allPiecesPlusEnemyKing)) {
            moves.push_back(Move(E8, C8, MOVE_QUEENSIDE_CASTLE));
        }
    }
}