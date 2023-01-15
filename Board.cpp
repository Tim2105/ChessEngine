#include "Board.h"
#include <iostream>
#include <stdio.h>

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
}

Board::Board(std::string fen) {
    initMailbox();

    // Spielfeld-Array und Figurlisten leeren
    for(int i = 0; i < 14; i++)
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

std::vector<Move> Board::generateWhitePawnMoves() {
    std::vector<Move> moves;
    moves.reserve(16);

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

    return moves;
}

std::vector<Move> Board::generateBlackPawnMoves() {
    std::vector<Move> moves;
    moves.reserve(16);

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

    return moves;
}