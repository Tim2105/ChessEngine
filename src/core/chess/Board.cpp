#include "core/chess/Board.h"
#include "core/chess/ZobristDefinitions.h"
#include "core/chess/movegen/Movegen.h"
#include "core/utils/MoveNotations.h"

#include <stdio.h>

Board::Board() {
    side = WHITE;
    enPassantSquare = NO_SQ;
    fiftyMoveRule = 0;
    castlingPermission = WHITE_KINGSIDE_CASTLE |
                       WHITE_QUEENSIDE_CASTLE |
                       BLACK_KINGSIDE_CASTLE |
                       BLACK_QUEENSIDE_CASTLE;
    ply = 0;

    moveHistory.reserve(256);

    generateBitboards();

    hashValue = generateHashValue();

    repetitionTable.increment(hashValue);
}

Board::Board(const Board& b) {
    for(int i = 0; i < 15; i++)
        pieceList[i] = b.pieceList[i];
    
    for(int i = 0; i < 64; i++)
        pieces[i] = b.pieces[i];

    side = b.side;
    enPassantSquare = b.enPassantSquare;
    fiftyMoveRule = b.fiftyMoveRule;
    castlingPermission = b.castlingPermission;
    ply = b.ply;

    moveHistory = b.moveHistory;
    repetitionTable = b.repetitionTable;

    hashValue = b.hashValue;

    generateBitboards();
}

Board& Board::operator=(const Board& b) {
    for(int i = 0; i < 15; i++)
        pieceList[i] = b.pieceList[i];
    
    for(int i = 0; i < 64; i++)
        pieces[i] = b.pieces[i];

    side = b.side;
    enPassantSquare = b.enPassantSquare;
    fiftyMoveRule = b.fiftyMoveRule;
    castlingPermission = b.castlingPermission;
    ply = b.ply;

    moveHistory = b.moveHistory;
    repetitionTable = b.repetitionTable;

    hashValue = b.hashValue;

    generateBitboards();

    return *this;
}

Board::Board(std::string fen) {
    // Trimme die FEN-Zeichenkette
    fen.erase(0, fen.find_first_not_of(' '));
    fen.erase(fen.find_last_not_of(' ') + 1);

    // Spielfeld-Array und Figurlisten leeren
    for(int i = 0; i < 15; i++)
        pieceList[i].clear();
    
    for(int i = 0; i < 64; i++)
        pieces[i] = EMPTY;

    int file = FILE_A;
    int rank = RANK_8;
    size_t indexNextSection = fen.find(' ');

    if(indexNextSection == std::string::npos)
        throw std::invalid_argument("Invalid FEN string: Side character missing");

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
                default: throw std::invalid_argument("Invalid FEN string: " + std::to_string(c) + " is not a valid piece character");
            }
            int square = FR2SQ(file, rank);
            pieces[square] = piece;

            if(pieceList[piece].size() == 9)
                throw std::invalid_argument("Invalid FEN string: There are more than 9 " + std::to_string(c) + "s on the board");

            pieceList[piece].push_back(square);
            file++;
        }
    }

    // Überprüfe, zwei Könige vorhanden sind und keine Bauern auf der 1. oder 8. Reihe stehen
    if(pieceList[WHITE_KING].size() != 1 || pieceList[BLACK_KING].size() != 1)
        throw std::invalid_argument("Invalid FEN string: There must be exactly one white and one black king");
    
    for(int32_t square : pieceList[WHITE_PAWN])
        if(SQ2R(square) == RANK_1 || SQ2R(square) == RANK_8)
            throw std::invalid_argument("Invalid FEN string: There must not be any white pawns on the first or eighth rank");
    
    for(int32_t square : pieceList[BLACK_PAWN])
        if(SQ2R(square) == RANK_1 || SQ2R(square) == RANK_8)
            throw std::invalid_argument("Invalid FEN string: There must not be any black pawns on the first or eighth rank");
    
    fen = fen.substr(indexNextSection + 1);
    indexNextSection = fen.find(' ');

    if(indexNextSection == std::string::npos)
        throw std::invalid_argument("Invalid FEN string: Castling rights missing");

    if(fen[0] != 'w' && fen[0] != 'b')
        throw std::invalid_argument("Invalid FEN string: " + std::to_string(fen[0]) + " is not a valid side character");

    // Zugfarbe auslesen
    side = fen[0] == 'w' ? WHITE : BLACK;

    fen = fen.substr(indexNextSection + 1);
    indexNextSection = fen.find(' ');

    // Wenn das En Passant Feld fehlt, wird es standardmäßig auf NO_SQ gesetzt
    if(indexNextSection == std::string::npos)
        indexNextSection = fen.length();

    // Noch mögliche Rochaden auslesen
    std::string fenCastling = fen.substr(0, indexNextSection);
    castlingPermission = 0;
    for(char c : fenCastling) {
        switch(c) {
            case 'K': castlingPermission |= WHITE_KINGSIDE_CASTLE; break;
            case 'Q': castlingPermission |= WHITE_QUEENSIDE_CASTLE; break;
            case 'k': castlingPermission |= BLACK_KINGSIDE_CASTLE; break;
            case 'q': castlingPermission |= BLACK_QUEENSIDE_CASTLE; break;
            case '-': break;
            default: throw std::invalid_argument("Invalid FEN string: " + std::to_string(c) + " is not a valid castling character");
        }
    }
    fen = fen.substr(std::min(indexNextSection + 1, fen.length()));
    indexNextSection = fen.find(' ');

    // Wenn die 50-Züge-Regel fehlt, wird sie standardmäßig auf 0 gesetzt
    if(indexNextSection == std::string::npos)
        indexNextSection = fen.length();

    // En Passant Feld auslesen
    std::string fenEnPassant = fen.substr(0, indexNextSection);
    if(fenEnPassant.length() == 2) {
        int file = fenEnPassant[0] - 'a';
        int rank = fenEnPassant[1] - '1';
        enPassantSquare = FR2SQ(file, rank);

        if(enPassantSquare < A1 || enPassantSquare > H8)
            throw std::invalid_argument("Invalid FEN string: En Passant square is not on the board");

        if(side == WHITE && rank != RANK_6)
            throw std::invalid_argument("Invalid FEN string: En Passant square is not on rank 6");
        else if(side == BLACK && rank != RANK_3)
            throw std::invalid_argument("Invalid FEN string: En Passant square is not on rank 3");
    }
    else {
        enPassantSquare = NO_SQ;
    }
    fen = fen.substr(std::min(indexNextSection + 1, fen.length()));
    indexNextSection = fen.find(' ');

    // Wenn die Anzahl der bereits gespielten Züge fehlt, wird sie standardmäßig auf 0 gesetzt
    if(indexNextSection == std::string::npos)
        indexNextSection = fen.length();

    // Status der 50-Züge-Regel auslesen
    if(indexNextSection == 0)
        fiftyMoveRule = 0;
    else {
        fiftyMoveRule = std::stoi(fen.substr(0, indexNextSection));
        if(fiftyMoveRule < 0)
            throw std::invalid_argument("Invalid FEN string: Fifty move rule is negative");
    }

    fen = fen.substr(std::min(indexNextSection + 1, fen.length()));

    if(fen.length() == 0)
        ply = side == WHITE ? 0 : 1;
    else {
        // Anzahl der bereits gespielten Züge auslesen
        int plyAdd = side == WHITE ? 0 : 1;
        int plyFen = std::stoi(fen);
        if(plyFen < 1)
            throw std::invalid_argument("Invalid FEN string: Number of played moves is zero or negative");

        ply = (plyFen - 1) * 2 + plyAdd;
    }

    generateBitboards();

    // Überprüfe, ob der König des Spielers, der nicht am Zug ist, im Schach steht
    if(squareAttacked(pieceList[(side ^ COLOR_MASK) | KING].front(), side))
        throw std::invalid_argument("Invalid FEN string: King of player not moving is in check");

    hashValue = generateHashValue();

    repetitionTable.increment(hashValue);
}


Board Board::fromPGN(std::string pgn) {
    Board board;

    std::vector<std::string> sections;

    std::string section;
    for(char c : pgn) {
        if(c == ' ') {
            if(section == "")
                continue;
            
            sections.push_back(section);
            section = "";
        }
        else if(c == '.')
            section = "";
        else
            section += c;
    }

    std::vector<std::string> stringHistory;

    for(std::string s : sections) {  
        Move move;

        for(Move m : board.generateLegalMoves()) {
            if(toStandardAlgebraicNotation(m, board) == s) {
                move = m;
                break;
            }
        }

        if(!board.isMoveLegal(move)) {
            std::string error = "Illegal move in PGN: ";
            error += s;
            error += " after ";

            for(std::string s : stringHistory)
                error += s + " ";
            
            throw std::runtime_error(error);
        }

        board.makeMove(move);
        stringHistory.push_back(s);
    }

    return board;
}

Board::~Board() {

}

uint64_t Board::generateHashValue() {
    uint64_t hash = 0ULL;

    // Figuren
    for(int i = 0; i < 64; i++) {
        int piece = pieces[i];
        if(piece != EMPTY)
            hash ^= Zobrist::zobristPieceKeys[piece][i];
    }

    // Zugfarbe
    if(side == BLACK)
        hash ^= Zobrist::zobristBlackToMove;
    
    // Rochadenrechte
    hash ^= Zobrist::zobristCastlingKeys[castlingPermission];

    // En Passant
    if(enPassantSquare != NO_SQ) {
        int file = SQ2F(enPassantSquare);
        hash ^= Zobrist::zobristEnPassantKeys[file];
    }

    return hash;
}


void Board::generateBitboards() {
    // weißer Bauer
    uint64_t res = 0ULL;
    for(int i : pieceList[WHITE_PAWN])
        res |= (1ULL << i);
    
    pieceBitboard[WHITE_PAWN] = Bitboard(res);

    // weißer Springer
    res = 0ULL;
    for(int i : pieceList[WHITE_KNIGHT])
        res |= (1ULL << i);

    pieceBitboard[WHITE_KNIGHT] = Bitboard(res);

    // weißer Läufer
    res = 0ULL;
    for(int i : pieceList[WHITE_BISHOP])
        res |= (1ULL << i);

    pieceBitboard[WHITE_BISHOP] = Bitboard(res);

    // weißer Turm
    res = 0ULL;
    for(int i : pieceList[WHITE_ROOK])
        res |= (1ULL << i);

    pieceBitboard[WHITE_ROOK] = Bitboard(res);

    // weiße Dame
    res = 0ULL;
    for(int i : pieceList[WHITE_QUEEN])
        res |= (1ULL << i);

    pieceBitboard[WHITE_QUEEN] = Bitboard(res);

    // weißer König
    res = 0ULL;
    for(int i : pieceList[WHITE_KING])
        res |= (1ULL << i);

    pieceBitboard[WHITE_KING] = Bitboard(res);

    // schwarzer Bauer
    res = 0ULL;
    for(int i : pieceList[BLACK_PAWN])
        res |= (1ULL << i);

    pieceBitboard[BLACK_PAWN] = Bitboard(res);

    // schwarzer Springer
    res = 0ULL;
    for(int i : pieceList[BLACK_KNIGHT])
        res |= (1ULL << i);

    pieceBitboard[BLACK_KNIGHT] = Bitboard(res);

    // schwarzer Läufer
    res = 0ULL;
    for(int i : pieceList[BLACK_BISHOP])
        res |= (1ULL << i);

    pieceBitboard[BLACK_BISHOP] = Bitboard(res);

    // schwarzer Turm
    res = 0ULL;
    for(int i : pieceList[BLACK_ROOK])
        res |= (1ULL << i);

    pieceBitboard[BLACK_ROOK] = Bitboard(res);

    // schwarze Dame
    res = 0ULL;
    for(int i : pieceList[BLACK_QUEEN])
        res |= (1ULL << i);

    pieceBitboard[BLACK_QUEEN] = Bitboard(res);

    // schwarzer König
    res = 0ULL;
    for(int i : pieceList[BLACK_KING])
        res |= (1ULL << i);

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

    // Angriffsbitboards
    whiteAttackBitboard = generateAttackBitboard(WHITE);
    blackAttackBitboard = generateAttackBitboard(BLACK);
}

bool Board::isMoveLegal(Move move) {
    // Nullzüge sind immer illegal
    if(move.isNullMove())
        return false;

    int32_t side = this->side;
    int32_t otherSide = side ^ COLOR_MASK;

    int32_t origin = move.getOrigin();
    int32_t destination = move.getDestination();

    // Überprüfe, ob der Zug eine eigene Figur ist
    if(pieces[origin] == EMPTY || (pieces[origin] & COLOR_MASK) != side)
        return false; // Zug ist keine eigene Figur
    
    // Überprüfe, ob der Zug eine eigene Figur schlägt
    if(pieces[destination] != EMPTY && (pieces[destination] & COLOR_MASK) == side)
        return false; // Zug schlägt eine eigene Figur
    
    // Wenn der Zug eine Rochade ist, überprüfe, ob die Rochade erlaubt ist
    if(move.isCastle()) {
        if(side == WHITE) {
            if(move.isKingsideCastle()) {
                if(!(castlingPermission & WHITE_KINGSIDE_CASTLE))
                    return false;
                
                if(pieces[E1] != WHITE_KING || pieces[H1] != WHITE_ROOK)
                    return false;
                
                if(pieces[F1] != EMPTY || pieces[G1] != EMPTY)
                    return false;
                
                if(squareAttacked(E1, otherSide) || squareAttacked(F1, otherSide) || squareAttacked(G1, otherSide))
                    return false;
            } else {
                if(!(castlingPermission & WHITE_QUEENSIDE_CASTLE))
                    return false;
                
                if(pieces[E1] != WHITE_KING || pieces[A1] != WHITE_ROOK)
                    return false;
                
                if(pieces[D1] != EMPTY || pieces[C1] != EMPTY || pieces[B1] != EMPTY)
                    return false;
                
                if(squareAttacked(E1, otherSide) || squareAttacked(D1, otherSide) || squareAttacked(C1, otherSide))
                    return false;
            }
        } else {
            if(move.isKingsideCastle()) {
                if(!(castlingPermission & BLACK_KINGSIDE_CASTLE))
                    return false;
                
                if(pieces[E8] != BLACK_KING || pieces[H8] != BLACK_ROOK)
                    return false;
                
                if(pieces[F8] != EMPTY || pieces[G8] != EMPTY)
                    return false;
                
                if(squareAttacked(E8, otherSide) || squareAttacked(F8, otherSide) || squareAttacked(G8, otherSide))
                    return false;
            } else {
                if(!(castlingPermission & BLACK_QUEENSIDE_CASTLE))
                    return false;
                
                if(pieces[E8] != BLACK_KING || pieces[A8] != BLACK_ROOK)
                    return false;
                
                if(pieces[D8] != EMPTY || pieces[C8] != EMPTY || pieces[B8] != EMPTY)
                    return false;
                
                if(squareAttacked(E8, otherSide) || squareAttacked(D8, otherSide) || squareAttacked(C8, otherSide))
                    return false;
            }
        }
    }

    // Überprüfe En Passant
    if(move.isEnPassant()) {
        if(enPassantSquare == NO_SQ)
            return false;

        if(pieces[origin] != (side | PAWN))
            return false;
        
        if(destination != enPassantSquare)
            return false;
    }

    // Überprüfe Bauernaufwertungen
    if(move.isPromotion()) {
        if(pieces[origin] != (side | PAWN))
            return false;
        
        if(side == WHITE) {
            if(SQ2R(origin) != RANK_7)
                return false;
        } else {
            if(SQ2R(origin) != RANK_2)
                return false;
        }
    }

    // Überprüfe, ob die Figur sich korrekt bewegt
    switch(pieces[origin]) {
        case WHITE_PAWN:
        case BLACK_PAWN: {
            int32_t destForw = side == WHITE ? NORTH : SOUTH;
            int32_t destLeft = destForw + WEST;
            int32_t destRight = destForw + EAST;

            if(destination == origin + destForw) {
                if(pieces[destination] != EMPTY)
                    return false;
            } else if(move.isDoublePawn() && destination == origin + destForw + destForw) {
                if(pieces[destination] != EMPTY || pieces[origin + destForw] != EMPTY)
                    return false;
            }

            if(!move.isEnPassant()) {
                if(destination == origin + destLeft) {
                    if(pieces[destination] == EMPTY)
                        return false;
                } else if(destination == origin + destRight)
                    if(pieces[destination] == EMPTY)
                        return false;
            }
            break;
        }
        case WHITE_KNIGHT:
        case BLACK_KNIGHT: {
            int32_t sqDiff = destination - origin;
            bool valid = false;

            for(int32_t i = 0; i < 8; i++) {
                if(sqDiff == KNIGHT_ATTACKS[i]) {
                    valid = true;
                    break;
                }
            }

            if(!valid)
                return false;
            break;
        }
        case WHITE_BISHOP: {
        case BLACK_BISHOP:
            if(!(diagonalAttackBitboard(origin, allPiecesBitboard | pieceBitboard[side | KING]).getBit(destination)))
                return false;
            break;
        }
        case WHITE_ROOK: {
        case BLACK_ROOK:
            if(!(horizontalAttackBitboard(origin, allPiecesBitboard | pieceBitboard[side | KING]).getBit(destination)))
                return false;
            break;
        }
        case WHITE_QUEEN: {
        case BLACK_QUEEN:
            if(!(diagonalAttackBitboard(origin, allPiecesBitboard | pieceBitboard[side | KING]).getBit(destination)
                || horizontalAttackBitboard(origin, allPiecesBitboard | pieceBitboard[side | KING]).getBit(destination)))
                return false;
            break;
        }
        case WHITE_KING: {
        case BLACK_KING:
            if(!move.isCastle() &&
               !(kingAttackBitboard(origin).getBit(destination)))
                return false;
            break;
        }
    }

    // Überprüfe, ob der Zug den eigenen König in Schach setzt/lässt
    makeMove(move);

    bool isCheck = squareAttacked(pieceList[side | KING].front(), otherSide);

    undoMove();

    return !isCheck;
}

bool Board::isCheck() const {
    return squareAttacked(pieceList[side | KING].front(), side ^ COLOR_MASK);
}

void Board::makeMove(Move m) {
    int32_t origin = m.getOrigin();
    int32_t destination = m.getDestination();
    int32_t pieceType = pieces[origin];
    int32_t capturedPieceType = pieces[destination];
    int32_t enPassantCaptureSq = enPassantSquare + (side == WHITE ? SOUTH : NORTH);

    if(m.isEnPassant())
        capturedPieceType = pieces[enPassantCaptureSq];

    // Speichere den Zustand des Spielfeldes, um den Zug später wieder rückgängig machen zu können
    MoveHistoryEntry entry(m);
    entry.capturedPiece = capturedPieceType;
    entry.castlingPermission = castlingPermission;
    entry.enPassantSquare = enPassantSquare;
    entry.fiftyMoveRule = fiftyMoveRule;
    entry.hashValue = hashValue;
    entry.whitePiecesBitboard = whitePiecesBitboard;
    entry.blackPiecesBitboard = blackPiecesBitboard;

    for(int i = 0; i < 15; i++)
        entry.pieceBitboard[i] = pieceBitboard[i];

    entry.whiteAttackBitboard = whiteAttackBitboard;
    entry.blackAttackBitboard = blackAttackBitboard;
    
    for(int i = 0; i < 15; i++)
        entry.pieceAttackBitboard[i] = pieceAttackBitboard[i];

    moveHistory.push_back(entry);

    // Zug ausführen

    // Spezialfall: Nullzug
    // Bei einem Nullzug wird nur der Spieler gewechselt
    if(m.isNullMove()) {
        enPassantSquare = NO_SQ;

        if(enPassantSquare != entry.enPassantSquare) {
            if(entry.enPassantSquare != NO_SQ)
                hashValue ^= Zobrist::zobristEnPassantKeys[SQ2F(entry.enPassantSquare)];
            
            if(enPassantSquare != NO_SQ)
                hashValue ^= Zobrist::zobristEnPassantKeys[SQ2F(enPassantSquare)];
        }

        fiftyMoveRule++;

        side = side ^ COLOR_MASK;
        hashValue ^= Zobrist::zobristBlackToMove;
        ply++;

        repetitionTable.increment(hashValue);

        return;
    }

    // Spezialfall: En Passant
    if(m.isEnPassant()) {
        // Entferne den geschlagenen Bauern
        pieces[enPassantCaptureSq] = EMPTY;
        pieceList[capturedPieceType].remove(enPassantCaptureSq);
        pieceBitboard[capturedPieceType].clearBit(enPassantCaptureSq);
        allPiecesBitboard.clearBit(enPassantCaptureSq);
        if(side == WHITE)
            blackPiecesBitboard.clearBit(enPassantCaptureSq);
        else
            whitePiecesBitboard.clearBit(enPassantCaptureSq);
        
        hashValue ^= Zobrist::zobristPieceKeys[capturedPieceType][enPassantCaptureSq];
    }

    // Spezialfall: Rochade
    if(m.isCastle()) {
        if(m.isKingsideCastle()) {
            // Turm auf Königsseite bewegen
            pieces[origin + 3] = EMPTY;
            pieces[origin + 1] = side | ROOK;
            pieceList[side | ROOK].remove(origin + 3);
            pieceList[side | ROOK].push_back(origin + 1);
            pieceBitboard[side | ROOK].clearBit(origin + 3);
            pieceBitboard[side | ROOK].setBit(origin + 1);
            allPiecesBitboard.clearBit(origin + 3);
            allPiecesBitboard.setBit(origin + 1);
            if(side == WHITE) {
                whitePiecesBitboard.clearBit(origin + 3);
                whitePiecesBitboard.setBit(origin + 1);
            }
            else {
                blackPiecesBitboard.clearBit(origin + 3);
                blackPiecesBitboard.setBit(origin + 1);
            }

            hashValue ^= Zobrist::zobristPieceKeys[side | ROOK][origin + 3];
            hashValue ^= Zobrist::zobristPieceKeys[side | ROOK][origin + 1];
        } else {
            // Turm auf Damenseite bewegen
            pieces[origin - 4] = EMPTY;
            pieces[origin - 1] = side | ROOK;
            pieceList[side | ROOK].remove(origin - 4);
            pieceList[side | ROOK].push_back(origin - 1);
            pieceBitboard[side | ROOK].clearBit(origin - 4);
            pieceBitboard[side | ROOK].setBit(origin - 1);
            allPiecesBitboard.clearBit(origin - 4);
            allPiecesBitboard.setBit(origin - 1);
            if(side == WHITE) {
                whitePiecesBitboard.clearBit(origin - 4);
                whitePiecesBitboard.setBit(origin - 1);
            }
            else {
                blackPiecesBitboard.clearBit(origin - 4);
                blackPiecesBitboard.setBit(origin - 1);
            }

            hashValue ^= Zobrist::zobristPieceKeys[side | ROOK][origin - 4];
            hashValue ^= Zobrist::zobristPieceKeys[side | ROOK][origin - 1];
        }
    }

    // Bewege die Figur
    pieces[origin] = EMPTY;
    pieces[destination] = pieceType;
    pieceList[pieceType].remove(origin);
    pieceList[pieceType].push_back(destination);
    pieceBitboard[pieceType].clearBit(origin);
    pieceBitboard[pieceType].setBit(destination);

    hashValue ^= Zobrist::zobristPieceKeys[pieceType][origin];
    hashValue ^= Zobrist::zobristPieceKeys[pieceType][destination];
    // Könige sind in den allgemeinen Bitboards nicht enthalten
    if(TYPEOF(pieceType) != KING) {
        allPiecesBitboard.clearBit(origin);
        allPiecesBitboard.setBit(destination);
        if(side == WHITE) {
            whitePiecesBitboard.clearBit(origin);
            whitePiecesBitboard.setBit(destination);
        } else {
            blackPiecesBitboard.clearBit(origin);
            blackPiecesBitboard.setBit(destination);
        }
    }

    // Spezialfall: Schlagen
    if(capturedPieceType != EMPTY && !m.isEnPassant()) {
        pieceList[capturedPieceType].remove(destination);
        pieceBitboard[capturedPieceType].clearBit(destination);
        if(side == WHITE)
            blackPiecesBitboard.clearBit(destination);
        else
            whitePiecesBitboard.clearBit(destination);
        
        hashValue ^= Zobrist::zobristPieceKeys[capturedPieceType][destination];
        
        // Wenn die schlagende Figur ein König ist, muss das Feld aus dem allgemeinen Bitboard entfernt werden
        if(TYPEOF(pieceType) == KING)
            allPiecesBitboard.clearBit(destination);
    }

    // Spezialfall: Bauernumwandlung
    if(m.isPromotion()) {
        int32_t promotedPieceType = EMPTY;
        if(m.isPromotionQueen())
            promotedPieceType = QUEEN;
        else if(m.isPromotionRook())
            promotedPieceType = ROOK;
        else if(m.isPromotionBishop())
            promotedPieceType = BISHOP;
        else if(m.isPromotionKnight())
            promotedPieceType = KNIGHT;

        // Entferne den Bauern, die allgemeinen Bitboards müssen nicht angepasst werden
        pieceList[side | PAWN].remove(destination);
        pieceBitboard[side | PAWN].clearBit(destination);

        hashValue ^= Zobrist::zobristPieceKeys[side | PAWN][destination];

        // Füge die neue Figur hinzu
        pieces[destination] = side | promotedPieceType;
        pieceList[side | promotedPieceType].push_back(destination);
        pieceBitboard[side | promotedPieceType].setBit(destination);

        hashValue ^= Zobrist::zobristPieceKeys[side | promotedPieceType][destination];
    }

    // Aktualisiere Angriffsbitboards
    Bitboard updatedSquares;
    updatedSquares.setBit(origin);
    updatedSquares.setBit(destination);

    if(m.isEnPassant())
        updatedSquares.setBit(enPassantCaptureSq);

    if(m.isCastle()) {
        if(m.isKingsideCastle()) {
            updatedSquares.setBit(origin + 3);
            updatedSquares.setBit(origin + 1);
        } else {
            updatedSquares.setBit(origin - 4);
            updatedSquares.setBit(origin - 1);
        }
    }

    whiteAttackBitboard = generateAttackBitboard(WHITE, updatedSquares, capturedPieceType, m.isPromotion() && side == WHITE);
    blackAttackBitboard = generateAttackBitboard(BLACK, updatedSquares, capturedPieceType, m.isPromotion() && side == BLACK);

    // Aktualisiere Rochandenrechte und En Passant
    enPassantSquare = NO_SQ;

    if(TYPEOF(pieceType) == KING || TYPEOF(pieceType) == ROOK) {
        if(pieceType == WHITE_KING)
            castlingPermission &= ~(WHITE_KINGSIDE_CASTLE | WHITE_QUEENSIDE_CASTLE);
        else if(pieceType == BLACK_KING)
            castlingPermission &= ~(BLACK_KINGSIDE_CASTLE | BLACK_QUEENSIDE_CASTLE);
        else if(pieceType == WHITE_ROOK && origin == A1)
            castlingPermission &= ~WHITE_QUEENSIDE_CASTLE;
        else if(pieceType == WHITE_ROOK && origin == H1)
            castlingPermission &= ~WHITE_KINGSIDE_CASTLE;
        else if(pieceType == BLACK_ROOK && origin == A8)
            castlingPermission &= ~BLACK_QUEENSIDE_CASTLE;
        else if(pieceType == BLACK_ROOK && origin == H8)
            castlingPermission &= ~BLACK_KINGSIDE_CASTLE;
    }

    if(TYPEOF(capturedPieceType) == ROOK) {
        if(capturedPieceType == WHITE_ROOK && destination == A1)
            castlingPermission &= ~WHITE_QUEENSIDE_CASTLE;
        else if(capturedPieceType == WHITE_ROOK && destination == H1)
            castlingPermission &= ~WHITE_KINGSIDE_CASTLE;
        else if(capturedPieceType == BLACK_ROOK && destination == A8)
            castlingPermission &= ~BLACK_QUEENSIDE_CASTLE;
        else if(capturedPieceType == BLACK_ROOK && destination == H8)
            castlingPermission &= ~BLACK_KINGSIDE_CASTLE;
    }

    if(castlingPermission != entry.castlingPermission) {
        hashValue ^= Zobrist::zobristCastlingKeys[entry.castlingPermission];
        hashValue ^= Zobrist::zobristCastlingKeys[castlingPermission];
    }

    if(m.isDoublePawn()) {
        if(side == WHITE)
            enPassantSquare = destination + SOUTH;
        else
            enPassantSquare = destination + NORTH;
    }

    if(enPassantSquare != entry.enPassantSquare) {
        if(entry.enPassantSquare != NO_SQ)
            hashValue ^= Zobrist::zobristEnPassantKeys[SQ2F(entry.enPassantSquare)];
        
        if(enPassantSquare != NO_SQ)
            hashValue ^= Zobrist::zobristEnPassantKeys[SQ2F(enPassantSquare)];
    }

    // Aktualisiere den Spielzustand
    if(capturedPieceType != EMPTY || TYPEOF(pieceType) == PAWN)
        fiftyMoveRule = 0;
    else
        fiftyMoveRule++;

    side = side ^ COLOR_MASK;
    hashValue ^= Zobrist::zobristBlackToMove;
    ply++;

    // Aktualisiere die Wiederholungstabelle
    repetitionTable.increment(hashValue);
}

void Board::undoMove() {
    MoveHistoryEntry moveEntry = moveHistory.back();
    Move move = moveEntry.move;
    moveHistory.pop_back();

    int32_t origin = move.getOrigin();
    int32_t destination = move.getDestination();
    int32_t pieceType = pieces[destination];
    int32_t capturedPieceType = moveEntry.capturedPiece;

    // Aktualisiere die Wiederholungstabelle
    repetitionTable.decrement(hashValue);

    // Mache den Spielzustand rückgängig
    ply--;
    side = side ^ COLOR_MASK;
    fiftyMoveRule = moveEntry.fiftyMoveRule;
    enPassantSquare = moveEntry.enPassantSquare;
    castlingPermission = moveEntry.castlingPermission;
    hashValue = moveEntry.hashValue;
    whitePiecesBitboard = moveEntry.whitePiecesBitboard;
    blackPiecesBitboard = moveEntry.blackPiecesBitboard;
    for(int i = 0; i < 15; i++)
        pieceBitboard[i] = moveEntry.pieceBitboard[i];

    allPiecesBitboard = whitePiecesBitboard | blackPiecesBitboard;

    whiteAttackBitboard = moveEntry.whiteAttackBitboard;
    blackAttackBitboard = moveEntry.blackAttackBitboard;
    for(int i = 0; i < 15; i++)
        pieceAttackBitboard[i] = moveEntry.pieceAttackBitboard[i];
    
    // Spezialfall: Nullzug
    if(move.isNullMove())
        return;

    int32_t enPassantCaptureSq = enPassantSquare + (side == WHITE ? SOUTH : NORTH);

    // Spezialfall: Bauernumwandlung
    if(move.isPromotion()) {
        int32_t promotedPieceType = EMPTY;
        if(move.isPromotionQueen())
            promotedPieceType = QUEEN;
        else if(move.isPromotionRook())
            promotedPieceType = ROOK;
        else if(move.isPromotionBishop())
            promotedPieceType = BISHOP;
        else if(move.isPromotionKnight())
            promotedPieceType = KNIGHT;
        
        pieceType = side | PAWN;

        // Entferne die neue Figur, die allgemeinen Bitboards müssen nicht angepasst werden
        pieceList[side | promotedPieceType].remove(destination);

        // Füge den Bauern hinzu
        pieces[destination] = side | PAWN;
        pieceList[side | PAWN].push_back(destination);
    }

    // Mache den Zug rückgängig
    pieces[origin] = pieceType;
    pieces[destination] = EMPTY;
    pieceList[pieceType].remove(destination);
    pieceList[pieceType].push_back(origin);

    // Spezialfall: Schlagen
    if(capturedPieceType != EMPTY && !move.isEnPassant()) {
        pieces[destination] = capturedPieceType;
        pieceList[capturedPieceType].push_back(destination);
    }

    // Spezialfall: Rochade
    if(move.isCastle()) {
        if(move.isKingsideCastle()) {
            // Rochade auf Königsseite
            pieces[origin + 3] = side | ROOK;
            pieces[origin + 1] = EMPTY;
            pieceList[side | ROOK].remove(origin + 1);
            pieceList[side | ROOK].push_back(origin + 3);
        } else {
            // Rochade auf Damenseite
            pieces[origin - 4] = side | ROOK;
            pieces[origin - 1] = EMPTY;
            pieceList[side | ROOK].remove(origin - 1);
            pieceList[side | ROOK].push_back(origin - 4);
        }
    }

    // Spezialfall: En Passant
    if(move.isEnPassant()) {
        pieces[enPassantCaptureSq] = capturedPieceType;
        pieceList[capturedPieceType].push_back(enPassantCaptureSq);
    }
}

bool Board::squareAttacked(int32_t sq, int32_t ownSide) const {
    if(ownSide == WHITE)
        return whiteAttackBitboard.getBit(sq);
    else
        return blackAttackBitboard.getBit(sq);
}

bool Board::squareAttacked(int32_t sq, int32_t ownSide, Bitboard occupied) const {
    int32_t otherSide = (ownSide == WHITE) ? BLACK : WHITE;

    // Diagonale Angriffe
    if(diagonalAttackBitboard(sq, occupied) & (pieceBitboard[ownSide | BISHOP] | pieceBitboard[ownSide | QUEEN]))
        return true;

    // Waaagerechte Angriffe
    if(horizontalAttackBitboard(sq, occupied) & (pieceBitboard[ownSide | ROOK] | pieceBitboard[ownSide | QUEEN]))
        return true;

    // Springer Angriffe
    if(knightAttackBitboard(sq) & pieceBitboard[ownSide | KNIGHT])
        return true;
    
    // Bauer Angriffe
    if(pawnAttackBitboard(sq, otherSide) & pieceBitboard[ownSide | PAWN])
        return true;
    
    // König Angriffe
    if(kingAttackBitboard(sq) & pieceBitboard[ownSide | KING])
        return true;
    
    return false;
}

bool Board::squareAttacked(int32_t sq, int32_t ownSide, Bitboard occupied, Bitboard& attackerRays) const {
    return numSquareAttackers(sq, ownSide, occupied, attackerRays) > 0;
}

int32_t Board::numSquareAttackers(int32_t sq, int32_t ownSide, Bitboard occupied) const {
    int32_t otherSide = (ownSide == WHITE) ? BLACK : WHITE;
    int32_t numAttackers = 0;

    // Diagonale Angriffe
    Bitboard diagonalAttackers = diagonalAttackBitboard(sq, occupied) & (pieceBitboard[ownSide | BISHOP] | pieceBitboard[ownSide | QUEEN]);
    numAttackers += diagonalAttackers.getNumberOfSetBits();

    // Waaagerechte Angriffe
    Bitboard straightAttackers = horizontalAttackBitboard(sq, occupied) & (pieceBitboard[ownSide | ROOK] | pieceBitboard[ownSide | QUEEN]);
    numAttackers += straightAttackers.getNumberOfSetBits();

    // Springer Angriffe
    Bitboard knightAttackers = knightAttackBitboard(sq) & pieceBitboard[ownSide | KNIGHT];
    numAttackers += knightAttackers.getNumberOfSetBits();

    // Bauer Angriffe
    Bitboard pawnAttackers = pawnAttackBitboard(sq, otherSide) & pieceBitboard[ownSide | PAWN];
    numAttackers += pawnAttackers.getNumberOfSetBits();

    // König Angriffe
    Bitboard kingAttackers = kingAttackBitboard(sq) & pieceBitboard[ownSide | KING];
    numAttackers += kingAttackers.getNumberOfSetBits();

    return numAttackers;
}

int32_t Board::numSquareAttackers(int32_t sq, int32_t ownSide, Bitboard occupied, Bitboard& attackerRays) const {
    int32_t otherSide = (ownSide == WHITE) ? BLACK : WHITE;
    int32_t numAttackers = 0;

    // Diagonale Angriffe
    Bitboard diagonalAttackers = diagonalAttackBitboard(sq, occupied) & (pieceBitboard[ownSide | BISHOP] | pieceBitboard[ownSide | QUEEN]);
    numAttackers += diagonalAttackers.getNumberOfSetBits();
    attackerRays |= diagonalAttackUntilBlocked(sq, pieceBitboard[ownSide | BISHOP] | pieceBitboard[ownSide | QUEEN], occupied);

    // Waaagerechte Angriffe
    Bitboard straightAttackers = horizontalAttackBitboard(sq, occupied) & (pieceBitboard[ownSide | ROOK] | pieceBitboard[ownSide | QUEEN]);
    numAttackers += straightAttackers.getNumberOfSetBits();
    attackerRays |= horizontalAttackUntilBlocked(sq, pieceBitboard[ownSide | ROOK] | pieceBitboard[ownSide | QUEEN], occupied);

    // Springer Angriffe
    Bitboard knightAttackers = knightAttackBitboard(sq) & pieceBitboard[ownSide | KNIGHT];
    numAttackers += knightAttackers.getNumberOfSetBits();
    attackerRays |= knightAttackers;

    // Bauer Angriffe
    Bitboard pawnAttackers = pawnAttackBitboard(sq, otherSide) & pieceBitboard[ownSide | PAWN];
    numAttackers += pawnAttackers.getNumberOfSetBits();
    attackerRays |= pawnAttackers;

    // König Angriffe
    Bitboard kingAttackers = kingAttackBitboard(sq) & pieceBitboard[ownSide | KING];
    numAttackers += kingAttackers.getNumberOfSetBits();
    attackerRays |= kingAttackers;

    return numAttackers;
}

Bitboard Board::generateAttackBitboard(int32_t side) {
    Bitboard attackBitboard;
    Bitboard piecesPlusOwnKing = allPiecesBitboard | pieceBitboard[side | KING];

    for(int i = (side | PAWN); i <= (side | KING); i++)
        pieceAttackBitboard[i] = 0;
    
    // Diagonale Angriffe
    for(int sq : pieceList[side | BISHOP]) {
        pieceAttackBitboard[side | BISHOP] |= diagonalAttackBitboard(sq, piecesPlusOwnKing);
        attackBitboard |= pieceAttackBitboard[side | BISHOP];
    }

    for(int sq : pieceList[side | QUEEN]) {
        pieceAttackBitboard[side | QUEEN] |= diagonalAttackBitboard(sq, piecesPlusOwnKing);
        attackBitboard |= pieceAttackBitboard[side | QUEEN];
    }

    // Waagerechte Angriffe
    for(int sq : pieceList[side | ROOK]) {
        pieceAttackBitboard[side | ROOK] |= horizontalAttackBitboard(sq, piecesPlusOwnKing);
        attackBitboard |= pieceAttackBitboard[side | ROOK];
    }

    for(int sq : pieceList[side | QUEEN]) {
        pieceAttackBitboard[side | QUEEN] |= horizontalAttackBitboard(sq, piecesPlusOwnKing);
        attackBitboard |= pieceAttackBitboard[side | QUEEN];
    }

    // Springer Angriffe
    for(int sq : pieceList[side | KNIGHT]) {
        pieceAttackBitboard[side | KNIGHT] |= knightAttackBitboard(sq);
        attackBitboard |= pieceAttackBitboard[side | KNIGHT];
    }

    // Bauer Angriffe
    for(int sq : pieceList[side | PAWN]) {
        pieceAttackBitboard[side | PAWN] |= pawnAttackBitboard(sq, side);
        attackBitboard |= pieceAttackBitboard[side | PAWN];
    }

    // König Angriffe
    for(int sq : pieceList[side | KING]) {
        pieceAttackBitboard[side | KING] |= kingAttackBitboard(sq);
        attackBitboard |= pieceAttackBitboard[side | KING];
    }

    return attackBitboard;
}

Bitboard Board::generateAttackBitboard(int32_t side, Bitboard updatedSquares, int32_t capturedPiece, bool wasPromotion) {
    Bitboard piecesPlusOwnKing = allPiecesBitboard | pieceBitboard[side | KING];
    
    // Diagonale Angriffe
    if(capturedPiece == (side | BISHOP) ||
       updatedSquares & (pieceAttackBitboard[side | BISHOP] | pieceBitboard[side | BISHOP])) {
        Bitboard bishopAttackBitboard;

        for(int sq : pieceList[side | BISHOP])
            bishopAttackBitboard |= diagonalAttackBitboard(sq, piecesPlusOwnKing);

        pieceAttackBitboard[side | BISHOP] = bishopAttackBitboard;
    }

    // Waagerechte Angriffe
    if(capturedPiece == (side | ROOK) ||
       updatedSquares & (pieceAttackBitboard[side | ROOK] | pieceBitboard[side | ROOK])) {
        Bitboard rookAttackBitboard;

        for(int sq : pieceList[side | ROOK])
            rookAttackBitboard |= horizontalAttackBitboard(sq, piecesPlusOwnKing);

        pieceAttackBitboard[side | ROOK] = rookAttackBitboard;
    }

    // Damen-Angriffe
    if(capturedPiece == (side | QUEEN) ||
       updatedSquares & (pieceAttackBitboard[side | QUEEN] | pieceBitboard[side | QUEEN])) {
        Bitboard queenAttackBitboard;

        for(int sq : pieceList[side | QUEEN])
            queenAttackBitboard |= diagonalAttackBitboard(sq, piecesPlusOwnKing) | horizontalAttackBitboard(sq, piecesPlusOwnKing);

        pieceAttackBitboard[side | QUEEN] = queenAttackBitboard;
    }

    // Springer-Angriffe
    if(capturedPiece == (side | KNIGHT) ||
       updatedSquares & pieceBitboard[side | KNIGHT]) {
        Bitboard knightBitboard;

        for(int sq : pieceList[side | KNIGHT])
            knightBitboard |= knightAttackBitboard(sq);

        pieceAttackBitboard[side | KNIGHT] = knightBitboard;
    }

    // Bauer-Angriffe
    if(capturedPiece == (side | PAWN) || wasPromotion ||
       updatedSquares & pieceBitboard[side | PAWN]) {
        Bitboard pawnBitboard;

        for(int sq : pieceList[side | PAWN])
            pawnBitboard |= pawnAttackBitboard(sq, side);

        pieceAttackBitboard[side | PAWN] = pawnBitboard;
    }

    // König-Angriffe
    if(updatedSquares & pieceBitboard[side | KING]) {
        Bitboard kingBitboard;

        for(int sq : pieceList[side | KING])
            kingBitboard |= kingAttackBitboard(sq);

        pieceAttackBitboard[side | KING] = kingBitboard;
    }

    // Erstelle ein zentrales Angriffsbitboard
    Bitboard attackBitboard;
    for(int i = (side | PAWN); i <= (side | KING); i++)
        attackBitboard |= pieceAttackBitboard[i];

    return attackBitboard;
}

void Board::generatePinnedPiecesBitboards(int32_t side, Bitboard& pinnedPiecesBitboard,
                                          int32_t* pinnedPiecesDirection) const {

    int32_t kingSquare = pieceList[side | KING].front();
    int32_t otherSide = side ^ COLOR_MASK;
    Bitboard enemyPiecesPlusKing;
    if(side == WHITE)
        enemyPiecesPlusKing = blackPiecesBitboard | pieceBitboard[BLACK | KING];
    else
        enemyPiecesPlusKing = whitePiecesBitboard | pieceBitboard[WHITE | KING];

    Bitboard ownPieces = side == WHITE ? whitePiecesBitboard : blackPiecesBitboard;

    // Diagonale Angriffe
    int diagonalPins[4];
    int diagonalPinDirections[4];

    int32_t numDiagonalPins = getDiagonallyPinnedToSquare(kingSquare,
                                ownPieces,
                                pieceBitboard[otherSide | QUEEN] | pieceBitboard[otherSide | BISHOP],
                                enemyPiecesPlusKing,
                                diagonalPins,
                                diagonalPinDirections);
    
    // Waaagerechte Angriffe
    int straightPins[4];
    int straightPinDirections[4];

    int32_t numStraightPins = getHorizontallyPinnedToSquare(kingSquare,
                              ownPieces,
                              pieceBitboard[otherSide | QUEEN] | pieceBitboard[otherSide | ROOK],
                              enemyPiecesPlusKing,
                              straightPins,
                              straightPinDirections);
    
    // Pins zusammenfassen
    for(int i = 0; i < numDiagonalPins; i++) {
        pinnedPiecesBitboard.setBit(diagonalPins[i]);
        pinnedPiecesDirection[diagonalPins[i]] = diagonalPinDirections[i];
    }

    for(int i = 0; i < numStraightPins; i++) {
        pinnedPiecesBitboard.setBit(straightPins[i]);
        pinnedPiecesDirection[straightPins[i]] = straightPinDirections[i];
    }
}

Array<Move, 256> Board::generatePseudoLegalMoves() const {
    Array<Move, 256> moves;

    if(side == WHITE) {
        Movegen::generatePseudoLegalWhitePawnMoves(moves, *this);
        Movegen::generatePseudoLegalWhiteKnightMoves(moves, *this);
        Movegen::generatePseudoLegalWhiteBishopMoves(moves, *this);
        Movegen::generatePseudoLegalWhiteRookMoves(moves, *this);
        Movegen::generatePseudoLegalWhiteQueenMoves(moves, *this);
        Movegen::generatePseudoLegalWhiteKingMoves(moves, *this);
    } else {
        Movegen::generatePseudoLegalBlackPawnMoves(moves, *this);
        Movegen::generatePseudoLegalBlackKnightMoves(moves, *this);
        Movegen::generatePseudoLegalBlackBishopMoves(moves, *this);
        Movegen::generatePseudoLegalBlackRookMoves(moves, *this);
        Movegen::generatePseudoLegalBlackQueenMoves(moves, *this);
        Movegen::generatePseudoLegalBlackKingMoves(moves, *this);
    }

    return moves;
}

Array<Move, 256> Board::generateLegalMoves() const {
    Array<Move, 256> legalMoves;

    if(side == WHITE) {
        Bitboard attackedSquares = blackAttackBitboard;

        Bitboard attackingRays;
        int32_t numAttackers = 0;
        if(isCheck())
            numAttackers = numSquareAttackers(pieceList[WHITE_KING].front(), BLACK, allPiecesBitboard | pieceBitboard[BLACK_KING], attackingRays);

        Bitboard pinnedPieces;
        int pinnedDirections[64];

        generatePinnedPiecesBitboards(WHITE, pinnedPieces, pinnedDirections);

        Movegen::generateWhitePawnMoves(legalMoves, *this, numAttackers, attackingRays, pinnedPieces, pinnedDirections);
        Movegen::generateWhiteKnightMoves(legalMoves, *this, numAttackers, attackingRays, pinnedPieces);
        Movegen::generateWhiteBishopMoves(legalMoves, *this, numAttackers, attackingRays, pinnedPieces, pinnedDirections);
        Movegen::generateWhiteRookMoves(legalMoves, *this, numAttackers, attackingRays, pinnedPieces, pinnedDirections);
        Movegen::generateWhiteQueenMoves(legalMoves, *this, numAttackers, attackingRays, pinnedPieces, pinnedDirections);
        Movegen::generateWhiteKingMoves(legalMoves, *this, attackedSquares);
    } else {
        Bitboard attackedSquares = whiteAttackBitboard;

        Bitboard attackingRays;
        int32_t numAttackers = 0;
        if(isCheck())
            numAttackers = numSquareAttackers(pieceList[BLACK_KING].front(), WHITE, allPiecesBitboard | pieceBitboard[WHITE_KING], attackingRays);

        Bitboard pinnedPieces;
        int pinnedDirections[64];

        generatePinnedPiecesBitboards(BLACK, pinnedPieces, pinnedDirections);

        Movegen::generateBlackPawnMoves(legalMoves, *this, numAttackers, attackingRays, pinnedPieces, pinnedDirections);
        Movegen::generateBlackKnightMoves(legalMoves, *this, numAttackers, attackingRays, pinnedPieces);
        Movegen::generateBlackBishopMoves(legalMoves, *this, numAttackers, attackingRays, pinnedPieces, pinnedDirections);
        Movegen::generateBlackRookMoves(legalMoves, *this, numAttackers, attackingRays, pinnedPieces, pinnedDirections);
        Movegen::generateBlackQueenMoves(legalMoves, *this, numAttackers, attackingRays, pinnedPieces, pinnedDirections);
        Movegen::generateBlackKingMoves(legalMoves, *this, attackedSquares);
    }

    return legalMoves;
}

Array<Move, 256> Board::generateLegalCaptures() const {
    Array<Move, 256> legalCaptures;

    if(side == WHITE) {
        Bitboard attackingRays;
        int32_t numAttackers = 0;
        if(isCheck())
            numAttackers = numSquareAttackers(pieceList[WHITE_KING].front(), BLACK, allPiecesBitboard | pieceBitboard[BLACK_KING], attackingRays);

        Bitboard pinnedPieces;
        int pinnedDirections[64];

        generatePinnedPiecesBitboards(WHITE, pinnedPieces, pinnedDirections);

        Movegen::generateWhitePawnCaptures(legalCaptures, *this, numAttackers, attackingRays, pinnedPieces, pinnedDirections);
        Movegen::generateWhiteKnightCaptures(legalCaptures, *this, numAttackers, attackingRays, pinnedPieces);
        Movegen::generateWhiteBishopCaptures(legalCaptures, *this, numAttackers, attackingRays, pinnedPieces, pinnedDirections);
        Movegen::generateWhiteRookCaptures(legalCaptures, *this, numAttackers, attackingRays, pinnedPieces, pinnedDirections);
        Movegen::generateWhiteQueenCaptures(legalCaptures, *this, numAttackers, attackingRays, pinnedPieces, pinnedDirections);
        Movegen::generateWhiteKingCaptures(legalCaptures, *this);
    } else {
        Bitboard attackingRays;
        int32_t numAttackers = 0;
        if(isCheck())
            numAttackers = numSquareAttackers(pieceList[BLACK_KING].front(), WHITE, allPiecesBitboard | pieceBitboard[WHITE_KING], attackingRays);

        Bitboard pinnedPieces;
        int pinnedDirections[64];

        generatePinnedPiecesBitboards(BLACK, pinnedPieces, pinnedDirections);

        Movegen::generateBlackPawnCaptures(legalCaptures, *this, numAttackers, attackingRays, pinnedPieces, pinnedDirections);
        Movegen::generateBlackKnightCaptures(legalCaptures, *this, numAttackers, attackingRays, pinnedPieces);
        Movegen::generateBlackBishopCaptures(legalCaptures, *this, numAttackers, attackingRays, pinnedPieces, pinnedDirections);
        Movegen::generateBlackRookCaptures(legalCaptures, *this, numAttackers, attackingRays, pinnedPieces, pinnedDirections);
        Movegen::generateBlackQueenCaptures(legalCaptures, *this, numAttackers, attackingRays, pinnedPieces, pinnedDirections);
        Movegen::generateBlackKingCaptures(legalCaptures, *this);
    }

    return legalCaptures;
}

std::string Board::toFen() const {
    std::string fen = "";

    int emptySquares = 0;

    for(int r = RANK_8; r >= RANK_1; r--) {
        for(int f = FILE_A; f <= FILE_H; f++) {
            int square = FR2SQ(f, r);

            if(pieces[square] == EMPTY) {
                emptySquares++;
            } else {
                if(emptySquares > 0) {
                    fen += std::to_string(emptySquares);
                    emptySquares = 0;
                }

                char pieceChar = ' ';

                switch(pieces[square]) {
                    case WHITE_PAWN:
                        pieceChar = 'P';
                        break;
                    case WHITE_KNIGHT:
                        pieceChar = 'N';
                        break;
                    case WHITE_BISHOP:
                        pieceChar = 'B';
                        break;
                    case WHITE_ROOK:
                        pieceChar = 'R';
                        break;
                    case WHITE_QUEEN:
                        pieceChar = 'Q';
                        break;
                    case WHITE_KING:
                        pieceChar = 'K';
                        break;
                    case BLACK_PAWN:
                        pieceChar = 'p';
                        break;
                    case BLACK_KNIGHT:
                        pieceChar = 'n';
                        break;
                    case BLACK_BISHOP:
                        pieceChar = 'b';
                        break;
                    case BLACK_ROOK:
                        pieceChar = 'r';
                        break;
                    case BLACK_QUEEN:
                        pieceChar = 'q';
                        break;
                    case BLACK_KING:
                        pieceChar = 'k';
                        break;
                }

                fen += pieceChar;
            }
        }

        if(emptySquares > 0) {
            fen += std::to_string(emptySquares);
            emptySquares = 0;
        }

        if(r > RANK_1)
            fen += "/";
    }

    fen += " ";

    if(side == WHITE)
        fen += "w";
    else
        fen += "b";
    
    fen += " ";

    if(castlingPermission == 0)
        fen += "-";
    else {
        if(castlingPermission & WHITE_KINGSIDE_CASTLE)
            fen += "K";
        if(castlingPermission & WHITE_QUEENSIDE_CASTLE)
            fen += "Q";
        if(castlingPermission & BLACK_KINGSIDE_CASTLE)
            fen += "k";
        if(castlingPermission & BLACK_QUEENSIDE_CASTLE)
            fen += "q";
    }

    fen += " ";

    if(enPassantSquare == NO_SQ)
        fen += "-";
    else {
        char file = 'a' + SQ2F(enPassantSquare);
        char rank = '1' + SQ2R(enPassantSquare);

        fen += file;
        fen += rank;
    }

    fen += " ";

    fen += std::to_string(fiftyMoveRule);

    fen += " ";

    int fullMoves = (ply / 2) + 1;

    fen += std::to_string(fullMoves);

    return fen;
}

std::string Board::toPgn() const {
    std::string pgn = "";

    Board temp;

    if(moveHistory.size() == 0)
        return pgn;

    int32_t startPly = ply - moveHistory.size();

    if(startPly != 0)
        throw std::runtime_error("The game hasn't started here. Can't generate PGN string.");

    int32_t fullMoves = 1;
    bool white = true;

    for(MoveHistoryEntry entry : moveHistory) {
        if(white) {
            pgn += std::to_string(fullMoves);
            pgn += ". ";
        }

        pgn += toStandardAlgebraicNotation(entry.move, temp);
        pgn += " ";
        temp.makeMove(entry.move);

        white = !white;

        if(white)
            fullMoves++;
    }

    return pgn;
}

uint8_t Board::repetitionCount() const {
    return repetitionTable.get(hashValue);
}