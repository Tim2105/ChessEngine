#include "Board.h"
#include <iostream>
#include <stdio.h>
#include "OrderedMoveList.h"

Board::Board() {
    initMailbox();

    side = WHITE;
    enPassantSquare = NO_SQ;
    fiftyMoveRule = 0;
    castlingPermission = WHITE_KINGSIDE_CASTLE |
                       WHITE_QUEENSIDE_CASTLE |
                       BLACK_KINGSIDE_CASTLE |
                       BLACK_QUEENSIDE_CASTLE;
    ply = 0;

    generateBitboards();
}

Board::Board(std::string fen) {
    initMailbox();

    // Spielfeld-Array und Figurlisten leeren
    for(int i = 0; i < 15; i++)
        pieceList[i].clear();
    
    for(int i = 0; i < 120; i++)
        pieces[i] = EMPTY;

    int file = FILE_A;
    int rank = RANK_8;
    int indexNextSection = fen.find(' ');

    // Figuren auslesen und einfügen
    std::string fenPieces = fen.substr(0, indexNextSection);
    for(char c : fenPieces) {
        if(c == '/') {
            file = FILE_A;
            rank--;
        }
        else if(c >= '1' && c <= '8') {
            file += c - '0';
        }
        else {
            int piece = EMPTY;
            switch(c) {
                case 'p': piece = BLACK_PAWN; break;
                case 'n': piece = BLACK_KNIGHT; break;
                case 'b': piece = BLACK_BISHOP; break;
                case 'r': piece = BLACK_ROOK; break;
                case 'q': piece = BLACK_QUEEN; break;
                case 'k': piece = BLACK_KING; break;
                case 'P': piece = WHITE_PAWN; break;
                case 'N': piece = WHITE_KNIGHT; break;
                case 'B': piece = WHITE_BISHOP; break;
                case 'R': piece = WHITE_ROOK; break;
                case 'Q': piece = WHITE_QUEEN; break;
                case 'K': piece = WHITE_KING; break;
            }
            int square = FR2SQ(file, rank);
            pieces[square] = piece;
            pieceList[piece].push_back(square);
            file++;
        }
    }
    
    fen = fen.substr(indexNextSection + 1);
    indexNextSection = fen.find(' ');

    // Zugfarbe auslesen
    side = fen[0] == 'w' ? WHITE : BLACK;
    fen = fen.substr(indexNextSection + 1);
    indexNextSection = fen.find(' ');

    // Noch mögliche Rochaden auslesen
    std::string fenCastling = fen.substr(0, indexNextSection);
    castlingPermission = 0;
    for(char c : fenCastling) {
        switch(c) {
            case 'K': castlingPermission |= WHITE_KINGSIDE_CASTLE; break;
            case 'Q': castlingPermission |= WHITE_QUEENSIDE_CASTLE; break;
            case 'k': castlingPermission |= BLACK_KINGSIDE_CASTLE; break;
            case 'q': castlingPermission |= BLACK_QUEENSIDE_CASTLE; break;
        }
    }
    fen = fen.substr(indexNextSection + 1);
    indexNextSection = fen.find(' ');

    // En Passant Feld auslesen
    std::string fenEnPassant = fen.substr(0, indexNextSection);
    if(fenEnPassant != "-") {
        int file = fenEnPassant[0] - 'a';
        int rank = fenEnPassant[1] - '1';
        enPassantSquare = FR2SQ(file, rank);
    }
    else {
        enPassantSquare = NO_SQ;
    }
    fen = fen.substr(indexNextSection + 1);
    indexNextSection = fen.find(' ');

    // Status der 50-Züge-Regel auslesen
    fiftyMoveRule = std::stoi(fen.substr(0, indexNextSection));
    fen = fen.substr(indexNextSection + 1);

    // Anzahl der bereits gespielten Halbzüge auslesen
    ply = std::stoi(fen) - 1;

    generateBitboards();
}

Board::~Board() {

}

void Board::initMailbox() {
    for(int i = 0; i < 120; i++)
        mailbox[i] = NO_SQ;
    
    int index = 0;
    for(int rank = RANK_1; rank <= RANK_8; rank++) {
        for(int file = FILE_A; file <= FILE_H; file++) {
            int square = FR2SQ(file, rank);
            mailbox[square] = index;
            mailbox64[index] = square;
            index++;
        }
    }    
}

void Board::generateBitboards() {
    // weißer Bauer
    uint64_t res = 0ULL;
    for(int i : pieceList[WHITE_PAWN])
        res |= (1ULL << mailbox[i]);
    
    pieceBitboard[WHITE_PAWN] = Bitboard(res);

    // weißer Springer
    res = 0ULL;
    for(int i : pieceList[WHITE_KNIGHT])
        res |= (1ULL << mailbox[i]);

    pieceBitboard[WHITE_KNIGHT] = Bitboard(res);

    // weißer Läufer
    res = 0ULL;
    for(int i : pieceList[WHITE_BISHOP])
        res |= (1ULL << mailbox[i]);

    pieceBitboard[WHITE_BISHOP] = Bitboard(res);

    // weißer Turm
    res = 0ULL;
    for(int i : pieceList[WHITE_ROOK])
        res |= (1ULL << mailbox[i]);

    pieceBitboard[WHITE_ROOK] = Bitboard(res);

    // weiße Dame
    res = 0ULL;
    for(int i : pieceList[WHITE_QUEEN])
        res |= (1ULL << mailbox[i]);

    pieceBitboard[WHITE_QUEEN] = Bitboard(res);

    // weißer König
    res = 0ULL;
    for(int i : pieceList[WHITE_KING])
        res |= (1ULL << mailbox[i]);

    pieceBitboard[WHITE_KING] = Bitboard(res);

    // schwarzer Bauer
    res = 0ULL;
    for(int i : pieceList[BLACK_PAWN])
        res |= (1ULL << mailbox[i]);

    pieceBitboard[BLACK_PAWN] = Bitboard(res);

    // schwarzer Springer
    res = 0ULL;
    for(int i : pieceList[BLACK_KNIGHT])
        res |= (1ULL << mailbox[i]);

    pieceBitboard[BLACK_KNIGHT] = Bitboard(res);

    // schwarzer Läufer
    res = 0ULL;
    for(int i : pieceList[BLACK_BISHOP])
        res |= (1ULL << mailbox[i]);

    pieceBitboard[BLACK_BISHOP] = Bitboard(res);

    // schwarzer Turm
    res = 0ULL;
    for(int i : pieceList[BLACK_ROOK])
        res |= (1ULL << mailbox[i]);

    pieceBitboard[BLACK_ROOK] = Bitboard(res);

    // schwarze Dame
    res = 0ULL;
    for(int i : pieceList[BLACK_QUEEN])
        res |= (1ULL << mailbox[i]);

    pieceBitboard[BLACK_QUEEN] = Bitboard(res);

    // schwarzer König
    res = 0ULL;
    for(int i : pieceList[BLACK_KING])
        res |= (1ULL << mailbox[i]);

    pieceBitboard[BLACK_KING] = Bitboard(res);

    // alle weißen Figuren
    whitePiecesBitboard = pieceBitboard[WHITE_PAWN] |
                  pieceBitboard[WHITE_KNIGHT] |
                  pieceBitboard[WHITE_BISHOP] |
                  pieceBitboard[WHITE_ROOK] |
                  pieceBitboard[WHITE_QUEEN];

    // alle schwarzen Figuren
    blackPiecesBitboard = pieceBitboard[BLACK_PAWN] |
                  pieceBitboard[BLACK_KNIGHT] |
                  pieceBitboard[BLACK_BISHOP] |
                  pieceBitboard[BLACK_ROOK] |
                  pieceBitboard[BLACK_QUEEN];
    
    // alle Figuren
    allPiecesBitboard = whitePiecesBitboard | blackPiecesBitboard;
}

bool Board::isLegal(Move move) {
    int32_t ownSide = side;
    int32_t enemySide = (side == WHITE) ? BLACK : WHITE;
    int32_t origin = move.getOrigin();
    int32_t destination = move.getDestination();
    int32_t pieceType = TYPEOF(pieces[origin]);

    // Spezialfall: En Passant
    if(move.isEnPassant()) {
        ASSERT(enPassantSquare != NO_SQ);

        // Das occupied-Bitboard muss so angepasst werden,
        // dass der En Passant geschlagene Bauer nicht mehr existiert
        Bitboard modifiedBitboard = allPiecesBitboard;
        modifiedBitboard.clearBit(mailbox[origin]);
        modifiedBitboard.clearBit(mailbox[enPassantSquare]);
        modifiedBitboard.setBit(mailbox[destination]);

        // Überprüfe, ob der König im Schach steht
        int32_t kingSquare = pieceList[ownSide | KING][0];
        return !squareAttacked(kingSquare, enemySide, modifiedBitboard);
    }

    int32_t kingSquare = pieceList[ownSide | KING][0];

    // Spezialfall: Rochade
    if(move.isCastle()) {
        // Der König, sein Ausgangsfeld und das Feld, dass er überspringt dürfen nicht im Schach stehen
        if(move.isKingsideCastle())
            return !squareAttacked(kingSquare, enemySide, allPiecesBitboard) &&
                   !squareAttacked(kingSquare + 1, enemySide, allPiecesBitboard) &&
                   !squareAttacked(kingSquare + 2, enemySide, allPiecesBitboard);
        else
            return !squareAttacked(kingSquare, enemySide, allPiecesBitboard) &&
                   !squareAttacked(kingSquare - 1, enemySide, allPiecesBitboard) &&
                   !squareAttacked(kingSquare - 2, enemySide, allPiecesBitboard);
    }

    // Spezialfall: Der König wird bewegt
    if(pieceType == KING) {
        // Der König darf sich nicht ins Schach bewegen
        return !squareAttacked(destination, enemySide, allPiecesBitboard);
    }

    // Überprüfe, ob die bewegte Figur den König beschützt
    Bitboard modifiedBitboard = allPiecesBitboard;
    modifiedBitboard.clearBit(mailbox[origin]);
    modifiedBitboard.setBit(mailbox[destination]);

    return !squareAttacked(kingSquare, enemySide, modifiedBitboard);
}

bool Board::squareAttacked(int32_t sq120, int32_t ownSide, Bitboard occupied) {
    int32_t sq64 = mailbox[sq120];
    int32_t otherSide = (ownSide == WHITE) ? BLACK : WHITE;

    // Diagonale Angriffe
    if(diagonalAttackBitboard(sq64, occupied) & (pieceBitboard[ownSide | BISHOP] | pieceBitboard[ownSide | QUEEN]))
        return true;

    // Waaagerechte Angriffe
    if(straightAttackBitboard(sq64, occupied) & (pieceBitboard[ownSide | ROOK] | pieceBitboard[ownSide | QUEEN]))
        return true;

    // Springer Angriffe
    if(knightAttackBitboard(sq64) & pieceBitboard[ownSide | KNIGHT])
        return true;
    
    // Bauer Angriffe
    if(pawnAttackBitboard(sq64, otherSide) & pieceBitboard[ownSide | PAWN])
        return true;
    
    // König Angriffe
    if(kingAttackBitboard(sq64) & pieceBitboard[ownSide | KING])
        return true;
    
    return false;
}

std::vector<Move> Board::generatePseudoLegalMoves() {
    std::vector<Move> moves;
    moves.reserve(256);

    if(side == WHITE) {
        generateWhitePawnMoves(moves);
        generateWhiteKnightMoves(moves);
        generateWhiteBishopMoves(moves);
        generateWhiteRookMoves(moves);
        generateWhiteQueenMoves(moves);
        generateWhiteKingMoves(moves);
    }
    else {
        generateBlackPawnMoves(moves);
        generateBlackKnightMoves(moves);
        generateBlackBishopMoves(moves);
        generateBlackRookMoves(moves);
        generateBlackQueenMoves(moves);
        generateBlackKingMoves(moves);
    }

    return moves;
}

std::list<Move> Board::generateLegalMoves() {
    std::vector<Move> moves = generatePseudoLegalMoves();
    OrderedMoveList legalMoves;

    for(Move move : moves) {
        if(isLegal(move))
            legalMoves.addMove(move);
    }

    return legalMoves.getMoves();
}

void Board::generateWhitePawnMoves(std::vector<Move>& moves) {
    for(int sq : pieceList[WHITE_PAWN]) {
        ASSERT(pieces[sq] == WHITE_PAWN);
        ASSERT(mailbox[sq] != NO_SQ);

        int32_t file = SQ2F(sq);
        int32_t rank = SQ2R(sq);

        int32_t destForw = FR2SQ(file, rank + 1);

        if(pieces[destForw] == EMPTY) {
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
            if(pieces[destForw] == EMPTY && pieces[destForw2] == EMPTY)
                moves.push_back(Move(sq, destForw2, MOVE_DOUBLE_PAWN));
        }

        int32_t destLeft = FR2SQ(file - 1, rank + 1);
        if(mailbox[destLeft] != NO_SQ && pieces[destLeft] != EMPTY && (pieces[destLeft] & COLOR_MASK) == BLACK) {
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
        if(mailbox[destRight] != NO_SQ && pieces[destRight] != EMPTY && (pieces[destRight] & COLOR_MASK) == BLACK) {
            if(rank == RANK_7) {
                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
            }
            else
                moves.push_back(Move(sq, destRight, MOVE_CAPTURE));
        }

        if(enPassantSquare != NO_SQ) {
            if(enPassantSquare == destLeft)
                moves.push_back(Move(sq, destLeft, MOVE_EN_PASSANT));
            else if(enPassantSquare == destRight)
                moves.push_back(Move(sq, destRight, MOVE_EN_PASSANT));
        }
    }
}

void Board::generateBlackPawnMoves(std::vector<Move>& moves) {
    for(int sq : pieceList[BLACK_PAWN]) {
        ASSERT(pieces[sq] == BLACK_PAWN);
        ASSERT(mailbox[sq] != NO_SQ);

        int32_t file = SQ2F(sq);
        int32_t rank = SQ2R(sq);

        int32_t destForw = FR2SQ(file, rank - 1);

        if(pieces[destForw] == EMPTY) {
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
            if(pieces[destForw] == EMPTY && pieces[destForw2] == EMPTY)
                moves.push_back(Move(sq, destForw2, MOVE_DOUBLE_PAWN));
        }

        int32_t destLeft = FR2SQ(file - 1, rank - 1);
        if(mailbox[destLeft] != NO_SQ && pieces[destLeft] != EMPTY && (pieces[destLeft] & COLOR_MASK) == WHITE) {
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
        if(mailbox[destRight] != NO_SQ && pieces[destRight] != EMPTY && (pieces[destRight] & COLOR_MASK) == WHITE) {
            if(rank == RANK_2) {
                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
            }
            else
                moves.push_back(Move(sq, destRight, MOVE_CAPTURE));
        }

        if(enPassantSquare != NO_SQ) {
            if(enPassantSquare == destLeft)
                moves.push_back(Move(sq, destLeft, MOVE_EN_PASSANT));
            else if(enPassantSquare == destRight)
                moves.push_back(Move(sq, destRight, MOVE_EN_PASSANT));
        }
    }
}

void Board::generateWhiteKnightMoves(std::vector<Move>& moves) {
    for(int sq : pieceList[WHITE_KNIGHT]) {
        ASSERT(pieces[sq] == WHITE_KNIGHT);
        ASSERT(mailbox[sq] != NO_SQ);

        for(int i = 0; i < 8; i++) {
            int n_sq = sq + KNIGHT_ATTACKS[i];
            if(mailbox[n_sq] != NO_SQ) {
                if(pieces[n_sq] == EMPTY)
                    moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                else if((pieces[n_sq] & COLOR_MASK) == BLACK)
                    moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
            }
        }
    }
}

void Board::generateBlackKnightMoves(std::vector<Move>& moves) {
    for(int sq : pieceList[BLACK_KNIGHT]) {
        ASSERT(pieces[sq] == BLACK_KNIGHT);
        ASSERT(mailbox[sq] != NO_SQ);

        for(int i = 0; i < 8; i++) {
            int n_sq = sq + KNIGHT_ATTACKS[i];
            if(mailbox[n_sq] != NO_SQ) {
                if(pieces[n_sq] == EMPTY)
                    moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                else if((pieces[n_sq] & COLOR_MASK) == WHITE)
                    moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
            }
        }
    }
}

void Board::generateWhiteRookMoves(std::vector<Move>& moves) {
    for(int sq : pieceList[WHITE_BISHOP]) {
        ASSERT(pieces[sq] == WHITE_BISHOP);
        ASSERT(mailbox[sq] != NO_SQ);

        for(int i = 0; i < 4; i++) {
            int n_sq = sq + DIAGONAL_ATTACKS[i];
            while(mailbox[n_sq] != NO_SQ) {
                if(pieces[n_sq] == EMPTY)
                    moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                else if((pieces[n_sq] & COLOR_MASK) == BLACK) {
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

void Board::generateBlackRookMoves(std::vector<Move>& moves) {
    for(int sq : pieceList[BLACK_BISHOP]) {
        ASSERT(pieces[sq] == BLACK_BISHOP);
        ASSERT(mailbox[sq] != NO_SQ);

        for(int i = 0; i < 4; i++) {
            int n_sq = sq + DIAGONAL_ATTACKS[i];
            while(mailbox[n_sq] != NO_SQ) {
                if(pieces[n_sq] == EMPTY)
                    moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                else if((pieces[n_sq] & COLOR_MASK) == WHITE) {
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

void Board::generateWhiteBishopMoves(std::vector<Move>& moves) {
    for(int sq : pieceList[WHITE_ROOK]) {
        ASSERT(pieces[sq] == WHITE_ROOK);
        ASSERT(mailbox[sq] != NO_SQ);

        for(int i = 0; i < 4; i++) {
            int n_sq = sq + STRAIGHT_ATTACKS[i];
            while(mailbox[n_sq] != NO_SQ) {
                if(pieces[n_sq] == EMPTY)
                    moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                else if((pieces[n_sq] & COLOR_MASK) == BLACK) {
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

void Board::generateBlackBishopMoves(std::vector<Move>& moves) {
    for(int sq : pieceList[BLACK_ROOK]) {
        ASSERT(pieces[sq] == BLACK_ROOK);
        ASSERT(mailbox[sq] != NO_SQ);

        for(int i = 0; i < 4; i++) {
            int n_sq = sq + STRAIGHT_ATTACKS[i];
            while(mailbox[n_sq] != NO_SQ) {
                if(pieces[n_sq] == EMPTY)
                    moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                else if((pieces[n_sq] & COLOR_MASK) == WHITE) {
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

void Board::generateWhiteQueenMoves(std::vector<Move>& moves) {
    int32_t QUEEN_ATTACKS[8] = {
        DIAGONAL_ATTACKS[0], DIAGONAL_ATTACKS[1], DIAGONAL_ATTACKS[2], DIAGONAL_ATTACKS[3],
        STRAIGHT_ATTACKS[0], STRAIGHT_ATTACKS[1], STRAIGHT_ATTACKS[2], STRAIGHT_ATTACKS[3]
    };

    for(int sq : pieceList[WHITE_QUEEN]) {
        ASSERT(pieces[sq] == WHITE_QUEEN);
        ASSERT(mailbox[sq] != NO_SQ);

        for(int i = 0; i < 8; i++) {
            int n_sq = sq + QUEEN_ATTACKS[i];
            while(mailbox[n_sq] != NO_SQ) {
                if(pieces[n_sq] == EMPTY)
                    moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                else if((pieces[n_sq] & COLOR_MASK) == BLACK) {
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

void Board::generateBlackQueenMoves(std::vector<Move>& moves) {
    int32_t QUEEN_ATTACKS[8] = {
        DIAGONAL_ATTACKS[0], DIAGONAL_ATTACKS[1], DIAGONAL_ATTACKS[2], DIAGONAL_ATTACKS[3],
        STRAIGHT_ATTACKS[0], STRAIGHT_ATTACKS[1], STRAIGHT_ATTACKS[2], STRAIGHT_ATTACKS[3]
    };

    for(int sq : pieceList[BLACK_QUEEN]) {
        ASSERT(pieces[sq] == BLACK_QUEEN);
        ASSERT(mailbox[sq] != NO_SQ);

        for(int i = 0; i < 8; i++) {
            int n_sq = sq + QUEEN_ATTACKS[i];
            while(mailbox[n_sq] != NO_SQ) {
                if(pieces[n_sq] == EMPTY)
                    moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                else if((pieces[n_sq] & COLOR_MASK) == WHITE) {
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

void Board::generateWhiteKingMoves(std::vector<Move>& moves) {
    int32_t KING_ATTACKS[8] = {
        DIAGONAL_ATTACKS[0], DIAGONAL_ATTACKS[1], DIAGONAL_ATTACKS[2], DIAGONAL_ATTACKS[3],
        STRAIGHT_ATTACKS[0], STRAIGHT_ATTACKS[1], STRAIGHT_ATTACKS[2], STRAIGHT_ATTACKS[3]
    };

    for(int sq : pieceList[WHITE_KING]) {
        ASSERT(pieces[sq] == WHITE_KING);
        ASSERT(mailbox[sq] != NO_SQ);

        for(int i = 0; i < 8; i++) {
            int n_sq = sq + KING_ATTACKS[i];
            if(mailbox[n_sq] != NO_SQ) {
                if(pieces[n_sq] == EMPTY)
                    moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                else if((pieces[n_sq] & COLOR_MASK) == BLACK)
                    moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
            }
        }
    }

    if(castlingPermission & WHITE_KINGSIDE_CASTLE) {
        if(pieces[F1] == EMPTY && pieces[G1] == EMPTY) {
            moves.push_back(Move(E1, G1, MOVE_KINGSIDE_CASTLE));
        }
    }

    if(castlingPermission & WHITE_QUEENSIDE_CASTLE) {
        if(pieces[D1] == EMPTY && pieces[C1] == EMPTY && pieces[B1] == EMPTY) {
            moves.push_back(Move(E1, C1, MOVE_QUEENSIDE_CASTLE));
        }
    }
}

void Board::generateBlackKingMoves(std::vector<Move>& moves) {
    int32_t KING_ATTACKS[8] = {
        DIAGONAL_ATTACKS[0], DIAGONAL_ATTACKS[1], DIAGONAL_ATTACKS[2], DIAGONAL_ATTACKS[3],
        STRAIGHT_ATTACKS[0], STRAIGHT_ATTACKS[1], STRAIGHT_ATTACKS[2], STRAIGHT_ATTACKS[3]
    };

    for(int sq : pieceList[BLACK_KING]) {
        ASSERT(pieces[sq] == BLACK_KING);
        ASSERT(mailbox[sq] != NO_SQ);

        for(int i = 0; i < 8; i++) {
            int n_sq = sq + KING_ATTACKS[i];
            if(mailbox[n_sq] != NO_SQ) {
                if(pieces[n_sq] == EMPTY)
                    moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                else if((pieces[n_sq] & COLOR_MASK) == WHITE)
                    moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
            }
        }
    }

    if(castlingPermission & BLACK_KINGSIDE_CASTLE) {
        if(pieces[F8] == EMPTY && pieces[G8] == EMPTY) {
            moves.push_back(Move(E8, G8, MOVE_KINGSIDE_CASTLE));
        }
    }

    if(castlingPermission & BLACK_QUEENSIDE_CASTLE) {
        if(pieces[D8] == EMPTY && pieces[C8] == EMPTY && pieces[B8] == EMPTY) {
            moves.push_back(Move(E8, C8, MOVE_QUEENSIDE_CASTLE));
        }
    }
}