#include "core/chess/Board.h"
#include "core/chess/Referee.h"
#include "core/chess/ZobristDefinitions.h"
#include "core/chess/movegen/NewMovegen.h"
#include "core/utils/MoveNotations.h"

#include <stdio.h>
#include <stdexcept>

Board::Board() {
    hashValue = generateHashValue();
    moveHistory.reserve(256);
}

Board::Board(std::string fen) {
    // Trimme die FEN-Zeichenkette
    fen.erase(0, fen.find_first_not_of(' '));
    fen.erase(fen.find_last_not_of(' ') + 1);

    // Initialisiere die Bitboards
    for(int i = 0; i < 15; i++)
        pieceBitboard[i] = Bitboard(0ULL);
    
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
            int square = Square::fromFileRank(file, rank);
            pieces[square] = piece;

            pieceBitboard[piece].setBit(square);
            file++;
        }
    }

    // Überprüfe, zwei Könige vorhanden sind und keine Bauern auf der 1. oder 8. Reihe stehen
    if(pieceBitboard[WHITE_KING].popcount() != 1 || 
       pieceBitboard[BLACK_KING].popcount() != 1)
        throw std::invalid_argument("Invalid FEN string: There must be exactly one white and one black king");

    Bitboard whitePawns = pieceBitboard[WHITE_PAWN];
    while(whitePawns) {
        int square = whitePawns.popFSB();
        if(SQ2R(square) == RANK_1 || SQ2R(square) == RANK_8)
            throw std::invalid_argument("Invalid FEN string: There must not be any white pawns on the first or eighth rank");
    }
    
    Bitboard blackPawns = pieceBitboard[BLACK_PAWN];
    while(blackPawns) {
        int square = blackPawns.popFSB();
        if(SQ2R(square) == RANK_1 || SQ2R(square) == RANK_8)
            throw std::invalid_argument("Invalid FEN string: There must not be any black pawns on the first or eighth rank");
    }
    
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
        age = side == WHITE ? 0 : 1;
    else {
        // Anzahl der bereits gespielten Züge auslesen
        int plyAdd = side == WHITE ? 0 : 1;
        int plyFen = std::stoi(fen);
        if(plyFen < 1)
            throw std::invalid_argument("Invalid FEN string: Number of played moves is zero or negative");

        age = (plyFen - 1) * 2 + plyAdd;
    }

    generateSpecialBitboards();

    // Überprüfe, ob der König des Spielers, der nicht am Zug ist, im Schach steht
    if(squareAttacked(pieceBitboard[(side ^ COLOR_MASK) | KING].getFSB(), side))
        throw std::invalid_argument("Invalid FEN string: King of player not moving is in check");

    hashValue = generateHashValue();
}

bool Board::operator==(const Board& b) const {
    for(size_t i = 0; i < 15; i++)
        if(pieceBitboard[i] != b.pieceBitboard[i])
            return false;

    if(side != b.side)
        return false;

    if(enPassantSquare != b.enPassantSquare)
        return false;

    if(fiftyMoveRule != b.fiftyMoveRule)
        return false;

    if(castlingPermission != b.castlingPermission)
        return false;

    if(age != b.age)
        return false;

    return true;
}

void trim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));

    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());

    s.shrink_to_fit();
}

std::tuple<std::string, std::string> readMetadataLine(std::istream& ss) {
    std::string key;
    std::string value;

    // Lese das [ - Zeichen
    ss.get();
    // Lese den Schlüssel
    ss >> key;
    // Lese das Leerzeichen
    ss >> std::ws;
    // Lese den Wert
    std::getline(ss, value);
    // Lese das Zeilenende
    ss >> std::ws;

    trim(value);

    // Entferne das vordereste " im Wert (wenn vorhanden)
    if(value[0] == '"')
        value.erase(0, 1);
    
    // Entferne das hinterste ] im Wert
    value.erase(value.length() - 1, 1);

    // Entferne das hinterste " im Wert (wenn vorhanden)
    if(value[value.length() - 1] == '"')
        value.erase(value.length() - 1, 1);

    return std::make_tuple(key, value);
}

std::string readToken(std::istream& ss) {
    // Lese bis zum nächsten Leerzeichen, Tabulator,
    // Zeilenumbruch oder einem { ; oder % - Zeichen

    // Entferne alle Leerzeichen, Tabulatoren und Zeilenumbrüche
    ss >> std::ws;

    std::stringstream move;
    char c;
    while(ss.get(c)) {
        if(c == '{' || c == ';' || c == '%') {
            // Hier beginnt ein Kommentar
            ss.unget();
            break;
        }

        // Überprüfe, ob das Zeichen leer ist
        if(std::isspace(c))
            break;

        move << c;

        if(c == '.') {
            // Zugnummer
            ss >> std::ws;
            break;
        }
    }

    return move.str();
}

std::string readComment(std::istream& ss) {
    // Blockkommentar: Lese bis zum nächsten }-Zeichen
    // Ersetze dabei alle Zeilenumbrüche
    // durch Leerzeichen
    // Zeilenkommentar: Lese bis zum nächsten Zeilenumbruch

    char commentType;
    ss >> commentType;

    std::stringstream comment;

    if(commentType == '{') {
        // Blockkommentar

        char c;
        while(ss.get(c)) {
            if(c == '}')
                break;
            if(!std::isspace(c) || c == '\t')
                comment << c;
            else if(c == '\n')
                comment << ' ';
        }
    } else {
        // Zeilenkommentar

        std::string line;
        std::getline(ss, line);
        trim(line);
        comment << line;
    }

    return comment.str();
}

std::tuple<Board, PGNData> Board::fromPGN(std::istream& ss, size_t numMoves) {
    PGNData data;

    std::vector<std::string> moves;
    std::string initialFen = "";

    bool behindMoveNumber = false;

    // Lese Leerzeichen, Tabulatoren und Zeilenumbrüche
    ss >> std::ws;

    std::string token;
    while(ss.good()) {
        // Überprüfe, was als nächstes gelesen werden soll
        char c = ss.peek();
        if(c == '[') {
            // Metadaten
            data.metadata.push_back(readMetadataLine(ss));

            if(std::get<0>(data.metadata.back()) == "FEN")
                initialFen = std::get<1>(data.metadata.back());

        } else if(c == '{' || c == ';' || c == '%') // Kommentar
            data.comments.push_back(std::make_tuple(readComment(ss), moves.size() + behindMoveNumber));
        else {
            token = readToken(ss);
            if(token != "") {
                if((token[0] >= '0' && token[0] <= '9') || token[0] == '.' || token[0] == '*') {
                    // Zugnummer oder Ergebnis
                    if(token == "1-0")
                        data.result = PGNData::WHITE_WINS;
                    else if(token == "0-1")
                        data.result = PGNData::BLACK_WINS;
                    else if(token == "1/2-1/2" || token == "1/2")
                        data.result = PGNData::DRAW;
                    else if(token == "*")
                        data.result = PGNData::ONGOING;
                    else
                        behindMoveNumber = true;

                    if(!behindMoveNumber)
                        break; // Wenn der Token keine Zugnummer ist, wurde ein Ergebnis gelesen und der PGN-String ist zu Ende
                } else {
                    // Zug

                    // Entferne alle =, (, ) und / Zeichen (für Promotionen)
                    token.erase(std::remove(token.begin(), token.end(), '='), token.end());
                    token.erase(std::remove(token.begin(), token.end(), '('), token.end());
                    token.erase(std::remove(token.begin(), token.end(), ')'), token.end());
                    token.erase(std::remove(token.begin(), token.end(), '/'), token.end());

                    moves.push_back(token);
                    behindMoveNumber = false;
                }
            }
        }

        // Lese Leerzeichen, Tabulatoren und Zeilenumbrüche
        ss >> std::ws;
    }

    Board board;
    size_t numMovesParsed = 0;

    if(initialFen != "") {
        board = Board(initialFen);
        numMovesParsed = board.getAge();
    }

    for(const std::string& s : moves) {
        if(numMovesParsed >= numMoves)
            break;

        Move move;

        for(Move m : board.generateLegalMoves()) {
            if(toStandardAlgebraicNotation(m, board) == s) {
                move = m;
                break;
            }
        }

        if(!board.isMoveLegal(move)) {
            std::stringstream error;
            error << "Illegal move in PGN: " << s << " at halfmove " << numMovesParsed + 1;

            throw std::runtime_error(error.str());
        }

        board.makeMove(move);
        numMovesParsed++;
    }

    return std::make_tuple(board, data);
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
        int file = Square::fileOf(enPassantSquare);
        hash ^= Zobrist::zobristEnPassantKeys[file];
    }

    return hash;
}


void Board::generateSpecialBitboards() {
    // alle weißen Figuren
    pieceBitboard[WHITE] = pieceBitboard[WHITE_PAWN] |
                  pieceBitboard[WHITE_KNIGHT] |
                  pieceBitboard[WHITE_BISHOP] |
                  pieceBitboard[WHITE_ROOK] |
                  pieceBitboard[WHITE_QUEEN];

    // alle schwarzen Figuren
    pieceBitboard[BLACK] = pieceBitboard[BLACK_PAWN] |
                  pieceBitboard[BLACK_KNIGHT] |
                  pieceBitboard[BLACK_BISHOP] |
                  pieceBitboard[BLACK_ROOK] |
                  pieceBitboard[BLACK_QUEEN];
    
    // alle Figuren
    pieceBitboard[ALL_PIECES] = pieceBitboard[WHITE] | pieceBitboard[BLACK];

    // Angriffsbitboards
    updateAttackBitboards(WHITE);
    updateAttackBitboards(BLACK);
}

bool Board::isMoveLegal(Move move) {
    // Nullzüge sind immer illegal
    if(move.isNullMove())
        return false;

    int side = this->side;
    int otherSide = side ^ COLOR_MASK;

    int origin = move.getOrigin();
    int destination = move.getDestination();

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
            int destForw = side == WHITE ? NORTH : SOUTH;
            int destLeft = destForw + WEST;
            int destRight = destForw + EAST;

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
            int sqDiff = destination - origin;
            bool valid = false;

            for(int i = 0; i < 8; i++) {
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
            if(!(diagonalAttackBitboard(origin, pieceBitboard[ALL_PIECES] | pieceBitboard[side | KING]).getBit(destination)))
                return false;
            break;
        }
        case WHITE_ROOK: {
        case BLACK_ROOK:
            if(!(horizontalAttackBitboard(origin, pieceBitboard[ALL_PIECES] | pieceBitboard[side | KING]).getBit(destination)))
                return false;
            break;
        }
        case WHITE_QUEEN: {
        case BLACK_QUEEN:
            if(!(diagonalAttackBitboard(origin, pieceBitboard[ALL_PIECES] | pieceBitboard[side | KING]).getBit(destination)
                || horizontalAttackBitboard(origin, pieceBitboard[ALL_PIECES] | pieceBitboard[side | KING]).getBit(destination)))
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

    bool isCheck = squareAttacked(pieceBitboard[side | KING].getFSB(), otherSide);

    undoMove();

    return !isCheck;
}

bool Board::isCheck() const {
    return squareAttacked(pieceBitboard[side | KING].getFSB(), side ^ COLOR_MASK);
}

void Board::makeMove(Move m) {
    int origin = m.getOrigin();
    int destination = m.getDestination();
    int pieceType = pieces[origin];
    int capturedPieceType = pieces[destination];
    int enPassantCaptureSq = enPassantSquare + (side == WHITE ? SOUTH : NORTH);

    if(m.isEnPassant())
        capturedPieceType = pieces[enPassantCaptureSq];

    // Speichere den Zustand des Spielfeldes, um den Zug später wieder rückgängig machen zu können
    MoveHistoryEntry entry(m);
    entry.capturedPiece = capturedPieceType;
    entry.castlingPermission = castlingPermission;
    entry.enPassantSquare = enPassantSquare;
    entry.fiftyMoveRule = fiftyMoveRule;
    entry.hashValue = hashValue;

    memcpy(entry.pieceBitboard, pieceBitboard, sizeof(Bitboard) * 15);
    memcpy(entry.attackBitboard, attackBitboard, sizeof(Bitboard) * 15);

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
        age++;

        return;
    }

    int otherSide = side ^ COLOR_MASK;

    // Spezialfall: En Passant
    if(m.isEnPassant()) {
        // Entferne den geschlagenen Bauern
        pieces[enPassantCaptureSq] = EMPTY;
        pieceBitboard[capturedPieceType].clearBit(enPassantCaptureSq);
        pieceBitboard[ALL_PIECES].clearBit(enPassantCaptureSq);
        pieceBitboard[otherSide].clearBit(enPassantCaptureSq);
        
        hashValue ^= Zobrist::zobristPieceKeys[capturedPieceType][enPassantCaptureSq];
    }

    // Spezialfall: Rochade
    if(m.isCastle()) {
        if(m.isKingsideCastle()) {
            // Turm auf Königsseite bewegen
            pieces[origin + 3] = EMPTY;
            pieces[origin + 1] = side | ROOK;
            pieceBitboard[side | ROOK].clearBit(origin + 3);
            pieceBitboard[side | ROOK].setBit(origin + 1);
            pieceBitboard[ALL_PIECES].clearBit(origin + 3);
            pieceBitboard[ALL_PIECES].setBit(origin + 1);
            pieceBitboard[side].clearBit(origin + 3);
            pieceBitboard[side].setBit(origin + 1);

            hashValue ^= Zobrist::zobristPieceKeys[side | ROOK][origin + 3];
            hashValue ^= Zobrist::zobristPieceKeys[side | ROOK][origin + 1];
        } else {
            // Turm auf Damenseite bewegen
            pieces[origin - 4] = EMPTY;
            pieces[origin - 1] = side | ROOK;
            pieceBitboard[side | ROOK].clearBit(origin - 4);
            pieceBitboard[side | ROOK].setBit(origin - 1);
            pieceBitboard[ALL_PIECES].clearBit(origin - 4);
            pieceBitboard[ALL_PIECES].setBit(origin - 1);
            pieceBitboard[side].clearBit(origin - 4);
            pieceBitboard[side].setBit(origin - 1);

            hashValue ^= Zobrist::zobristPieceKeys[side | ROOK][origin - 4];
            hashValue ^= Zobrist::zobristPieceKeys[side | ROOK][origin - 1];
        }
    }

    // Bewege die Figur
    pieces[origin] = EMPTY;
    pieces[destination] = pieceType;
    pieceBitboard[pieceType].clearBit(origin);
    pieceBitboard[pieceType].setBit(destination);

    hashValue ^= Zobrist::zobristPieceKeys[pieceType][origin];
    hashValue ^= Zobrist::zobristPieceKeys[pieceType][destination];
    // Könige sind in den allgemeinen Bitboards nicht enthalten
    if(TYPEOF(pieceType) != KING) {
        pieceBitboard[ALL_PIECES].clearBit(origin);
        pieceBitboard[ALL_PIECES].setBit(destination);
        pieceBitboard[side].clearBit(origin);
        pieceBitboard[side].setBit(destination);
    }

    // Spezialfall: Schlagen
    if(capturedPieceType != EMPTY && !m.isEnPassant()) {
        pieceBitboard[capturedPieceType].clearBit(destination);
        pieceBitboard[otherSide].clearBit(destination);
        
        hashValue ^= Zobrist::zobristPieceKeys[capturedPieceType][destination];
        
        // Wenn die schlagende Figur ein König ist, muss das Feld aus dem allgemeinen Bitboard entfernt werden
        if(TYPEOF(pieceType) == KING)
            pieceBitboard[ALL_PIECES].clearBit(destination);
    }

    // Spezialfall: Bauernumwandlung
    if(m.isPromotion()) {
        int promotedPieceType = EMPTY;
        if(m.isPromotionQueen())
            promotedPieceType = QUEEN;
        else if(m.isPromotionRook())
            promotedPieceType = ROOK;
        else if(m.isPromotionBishop())
            promotedPieceType = BISHOP;
        else if(m.isPromotionKnight())
            promotedPieceType = KNIGHT;

        // Entferne den Bauern, die allgemeinen Bitboards müssen nicht angepasst werden
        pieceBitboard[side | PAWN].clearBit(destination);

        hashValue ^= Zobrist::zobristPieceKeys[side | PAWN][destination];

        // Füge die neue Figur hinzu
        pieces[destination] = side | promotedPieceType;
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

    updateAttackBitboards(WHITE, updatedSquares, capturedPieceType, m.isPromotion() && side == WHITE);
    updateAttackBitboards(BLACK, updatedSquares, capturedPieceType, m.isPromotion() && side == BLACK);

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

    side = otherSide;
    hashValue ^= Zobrist::zobristBlackToMove;
    age++;
}

void Board::undoMove() {
    MoveHistoryEntry& moveEntry = moveHistory.back();
    Move move = moveEntry.move;

    int origin = move.getOrigin();
    int destination = move.getDestination();
    int pieceType = pieces[destination];
    int capturedPieceType = moveEntry.capturedPiece;

    // Mache den Spielzustand rückgängig
    age--;
    side ^= COLOR_MASK;
    castlingPermission = moveEntry.castlingPermission;
    enPassantSquare = moveEntry.enPassantSquare;
    fiftyMoveRule = moveEntry.fiftyMoveRule;
    hashValue = moveEntry.hashValue;

    memcpy(pieceBitboard, moveEntry.pieceBitboard, sizeof(Bitboard) * 15);
    memcpy(attackBitboard, moveEntry.attackBitboard, sizeof(Bitboard) * 15);
    
    // Spezialfall: Nullzug
    if(move.isNullMove()) {
        moveHistory.pop_back();
        return;
    }

    int enPassantCaptureSq = enPassantSquare + (side == WHITE ? SOUTH : NORTH);

    // Spezialfall: Bauernumwandlung
    if(move.isPromotion())
        pieceType = side | PAWN;

    // Mache den Zug rückgängig
    pieces[origin] = pieceType;
    pieces[destination] = EMPTY;

    // Spezialfall: Schlagen
    if(capturedPieceType != EMPTY && !move.isEnPassant())
        pieces[destination] = capturedPieceType;

    // Spezialfall: Rochade
    if(move.isCastle()) {
        if(move.isKingsideCastle()) {
            // Rochade auf Königsseite
            pieces[origin + 3] = side | ROOK;
            pieces[origin + 1] = EMPTY;
        } else {
            // Rochade auf Damenseite
            pieces[origin - 4] = side | ROOK;
            pieces[origin - 1] = EMPTY;
        }
    }

    // Spezialfall: En Passant
    if(move.isEnPassant())
        pieces[enPassantCaptureSq] = capturedPieceType;

    moveHistory.pop_back();
}

bool Board::squareAttacked(int sq, int ownSide) const {
    return attackBitboard[ownSide].getBit(sq);
}

bool Board::squareAttacked(int sq, int ownSide, Bitboard occupied) const {
    int otherSide = ownSide ^ COLOR_MASK;

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

bool Board::squareAttacked(int sq, int ownSide, Bitboard occupied, Bitboard& attackerRays) const {
    return numSquareAttackers(sq, ownSide, occupied, attackerRays) > 0;
}

int Board::numSquareAttackers(int sq, int ownSide, Bitboard occupied) const {
    int otherSide = ownSide ^ COLOR_MASK;
    int numAttackers = 0;

    // Diagonale Angriffe
    Bitboard diagonalAttackers = diagonalAttackBitboard(sq, occupied) & (pieceBitboard[ownSide | BISHOP] | pieceBitboard[ownSide | QUEEN]);
    numAttackers += diagonalAttackers.popcount();

    // Waaagerechte Angriffe
    Bitboard straightAttackers = horizontalAttackBitboard(sq, occupied) & (pieceBitboard[ownSide | ROOK] | pieceBitboard[ownSide | QUEEN]);
    numAttackers += straightAttackers.popcount();

    // Springer Angriffe
    Bitboard knightAttackers = knightAttackBitboard(sq) & pieceBitboard[ownSide | KNIGHT];
    numAttackers += knightAttackers.popcount();

    // Bauer Angriffe
    Bitboard pawnAttackers = pawnAttackBitboard(sq, otherSide) & pieceBitboard[ownSide | PAWN];
    numAttackers += pawnAttackers.popcount();

    // König Angriffe
    Bitboard kingAttackers = kingAttackBitboard(sq) & pieceBitboard[ownSide | KING];
    numAttackers += kingAttackers.popcount();

    return numAttackers;
}

int Board::numSquareAttackers(int sq, int ownSide, Bitboard occupied, Bitboard& attackerRays) const {
    int otherSide = ownSide ^ COLOR_MASK;
    int numAttackers = 0;

    // Diagonale Angriffe
    Bitboard diagonalAttackers = diagonalAttackBitboard(sq, occupied) & (pieceBitboard[ownSide | BISHOP] | pieceBitboard[ownSide | QUEEN]);
    numAttackers += diagonalAttackers.popcount();
    attackerRays |= diagonalAttackUntilBlocked(sq, pieceBitboard[ownSide | BISHOP] | pieceBitboard[ownSide | QUEEN], occupied);

    // Waaagerechte Angriffe
    Bitboard straightAttackers = horizontalAttackBitboard(sq, occupied) & (pieceBitboard[ownSide | ROOK] | pieceBitboard[ownSide | QUEEN]);
    numAttackers += straightAttackers.popcount();
    attackerRays |= horizontalAttackUntilBlocked(sq, pieceBitboard[ownSide | ROOK] | pieceBitboard[ownSide | QUEEN], occupied);

    // Springer Angriffe
    Bitboard knightAttackers = knightAttackBitboard(sq) & pieceBitboard[ownSide | KNIGHT];
    numAttackers += knightAttackers.popcount();
    attackerRays |= knightAttackers;

    // Bauer Angriffe
    Bitboard pawnAttackers = pawnAttackBitboard(sq, otherSide) & pieceBitboard[ownSide | PAWN];
    numAttackers += pawnAttackers.popcount();
    attackerRays |= pawnAttackers;

    // König Angriffe
    Bitboard kingAttackers = kingAttackBitboard(sq) & pieceBitboard[ownSide | KING];
    numAttackers += kingAttackers.popcount();
    attackerRays |= kingAttackers;

    return numAttackers;
}

void Board::updateAttackBitboards(int side) {
    Bitboard piecesPlusOwnKing = pieceBitboard[ALL_PIECES] | pieceBitboard[side | KING];

    for(int i = side; i <= (side | KING); i++)
        attackBitboard[i] = 0;
    
    // Diagonale Angriffe
    Bitboard bishopBitboard = pieceBitboard[side | BISHOP];
    while(bishopBitboard) {
        int sq = bishopBitboard.popFSB();
        attackBitboard[side | BISHOP] |= diagonalAttackBitboard(sq, piecesPlusOwnKing);
        attackBitboard[side] |= attackBitboard[side | BISHOP];
    }

    Bitboard queenBitboard = pieceBitboard[side | QUEEN];
    while(queenBitboard) {
        int sq = queenBitboard.popFSB();
        attackBitboard[side | QUEEN] |= diagonalAttackBitboard(sq, piecesPlusOwnKing);
        attackBitboard[side] |= attackBitboard[side | QUEEN];
    }

    // Waagerechte Angriffe
    Bitboard rookBitboard = pieceBitboard[side | ROOK];
    while(rookBitboard) {
        int sq = rookBitboard.popFSB();
        attackBitboard[side | ROOK] |= horizontalAttackBitboard(sq, piecesPlusOwnKing);
        attackBitboard[side] |= attackBitboard[side | ROOK];
    }

    queenBitboard = pieceBitboard[side | QUEEN];
    while(queenBitboard) {
        int sq = queenBitboard.popFSB();
        attackBitboard[side | QUEEN] |= horizontalAttackBitboard(sq, piecesPlusOwnKing);
        attackBitboard[side] |= attackBitboard[side | QUEEN];
    }

    // Springer Angriffe
    Bitboard knightBitboard = pieceBitboard[side | KNIGHT];
    while(knightBitboard) {
        int sq = knightBitboard.popFSB();
        attackBitboard[side | KNIGHT] |= knightAttackBitboard(sq);
        attackBitboard[side] |= attackBitboard[side | KNIGHT];
    }

    // Bauer Angriffe
    Bitboard pawnBitboard = pieceBitboard[side | PAWN];
    while(pawnBitboard) {
        int sq = pawnBitboard.popFSB();
        attackBitboard[side | PAWN] |= pawnAttackBitboard(sq, side);
        attackBitboard[side] |= attackBitboard[side | PAWN];
    }

    // König Angriffe
    Bitboard kingBitboard = pieceBitboard[side | KING];
    while(kingBitboard) {
        int sq = kingBitboard.popFSB();
        attackBitboard[side | KING] |= kingAttackBitboard(sq);
        attackBitboard[side] |= attackBitboard[side | KING];
    }

    attackBitboard[ALL_PIECES] = attackBitboard[WHITE] | attackBitboard[BLACK];
}

void Board::updateAttackBitboards(int side, Bitboard updatedSquares, int capturedPiece, bool wasPromotion) {
    Bitboard piecesPlusOwnKing = pieceBitboard[ALL_PIECES] | pieceBitboard[side | KING];
    
    // Diagonale Angriffe
    if(capturedPiece == (side | BISHOP) ||
       updatedSquares & (attackBitboard[side | BISHOP] | pieceBitboard[side | BISHOP])) {
        Bitboard bishopAttackBitboard;
        Bitboard bishopBitboard = pieceBitboard[side | BISHOP];

        while(bishopBitboard) {
            int sq = bishopBitboard.popFSB();
            bishopAttackBitboard |= diagonalAttackBitboard(sq, piecesPlusOwnKing);
        }

        attackBitboard[side | BISHOP] = bishopAttackBitboard;
    }

    // Waagerechte Angriffe
    if(capturedPiece == (side | ROOK) ||
       updatedSquares & (attackBitboard[side | ROOK] | pieceBitboard[side | ROOK])) {
        Bitboard rookAttackBitboard;
        Bitboard rookBitboard = pieceBitboard[side | ROOK];

        while(rookBitboard) {
            int sq = rookBitboard.popFSB();
            rookAttackBitboard |= horizontalAttackBitboard(sq, piecesPlusOwnKing);
        }

        attackBitboard[side | ROOK] = rookAttackBitboard;
    }

    // Damen-Angriffe
    if(capturedPiece == (side | QUEEN) ||
       updatedSquares & (attackBitboard[side | QUEEN] | pieceBitboard[side | QUEEN])) {
        Bitboard queenAttackBitboard;
        Bitboard queenBitboard = pieceBitboard[side | QUEEN];

        while(queenBitboard) {
            int sq = queenBitboard.popFSB();
            queenAttackBitboard |= diagonalAttackBitboard(sq, piecesPlusOwnKing);
            queenAttackBitboard |= horizontalAttackBitboard(sq, piecesPlusOwnKing);
        }

        attackBitboard[side | QUEEN] = queenAttackBitboard;
    }

    // Springer-Angriffe
    if(capturedPiece == (side | KNIGHT) ||
       updatedSquares & pieceBitboard[side | KNIGHT]) {
        Bitboard knightAttacks;
        Bitboard knightBitboard = pieceBitboard[side | KNIGHT];

        while(knightBitboard) {
            int sq = knightBitboard.popFSB();
            knightAttacks |= knightAttackBitboard(sq);
        }

        attackBitboard[side | KNIGHT] = knightAttacks;
    }

    // Bauer-Angriffe
    if(capturedPiece == (side | PAWN) || wasPromotion ||
       updatedSquares & pieceBitboard[side | PAWN]) {
        Bitboard pawnAttacks;
        Bitboard pawnBitboard = pieceBitboard[side | PAWN];

        while(pawnBitboard) {
            int sq = pawnBitboard.popFSB();
            pawnAttacks |= pawnAttackBitboard(sq, side);
        }

        attackBitboard[side | PAWN] = pawnAttacks;
    }

    // König-Angriffe
    if(updatedSquares & pieceBitboard[side | KING]) {
        Bitboard kingAttacks;
        Bitboard kingBitboard = pieceBitboard[side | KING];

        while(kingBitboard) {
            int sq = kingBitboard.popFSB();
            kingAttacks |= kingAttackBitboard(sq);
        }

        attackBitboard[side | KING] = kingAttacks;
    }

    // Erstelle ein zentrales Angriffsbitboard
    attackBitboard[side] = 0;
    for(int i = (side | PAWN); i <= (side | KING); i++)
        attackBitboard[side] |= attackBitboard[i];

    attackBitboard[ALL_PIECES] = attackBitboard[WHITE] | attackBitboard[BLACK];
}

void Board::generatePinnedPiecesBitboards(int side, Bitboard& pinnedPiecesBitboard,
                                          int* pinnedPiecesDirection) const {

    int kingSquare = pieceBitboard[side | KING].getFSB();
    int otherSide = side ^ COLOR_MASK;
    Bitboard enemyPiecesPlusKing = pieceBitboard[otherSide] | pieceBitboard[otherSide | KING];

    Bitboard ownPieces = pieceBitboard[side];

    // Diagonale Angriffe
    int diagonalPins[4];
    int diagonalPinDirections[4];

    int numDiagonalPins = getDiagonallyPinnedToSquare(kingSquare,
                              ownPieces,
                              pieceBitboard[otherSide | QUEEN] | pieceBitboard[otherSide | BISHOP],
                              enemyPiecesPlusKing,
                              diagonalPins,
                              diagonalPinDirections);
    
    // Waaagerechte Angriffe
    int straightPins[4];
    int straightPinDirections[4];

    int numStraightPins = getHorizontallyPinnedToSquare(kingSquare,
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

Array<Move, 256> Board::generateLegalMoves() const noexcept {
    Array<Move, 256> legalMoves;
    generateLegalMoves(legalMoves);
    return legalMoves;
}

void Board::generateLegalMoves(Array<Move, 256>& legalMoves) const noexcept {
    Bitboard attackingRays, pinnedPieces;
    int numAttackers = 0;
    Array<int, 64> pinDirections;
    pinDirections.resize(64);

    if(side == WHITE) {
        if(isCheck())
            numAttackers = numSquareAttackers(pieceBitboard[WHITE_KING].getFSB(), BLACK, pieceBitboard[ALL_PIECES] | pieceBitboard[BLACK_KING], attackingRays);
        
        generatePinnedPiecesBitboards(WHITE, pinnedPieces, pinDirections);

        Movegen::generateLegalMoves<WHITE>(*this, legalMoves, numAttackers, attackingRays, pinnedPieces, pinDirections);
    } else {
        if(isCheck())
            numAttackers = numSquareAttackers(pieceBitboard[BLACK_KING].getFSB(), WHITE, pieceBitboard[ALL_PIECES] | pieceBitboard[WHITE_KING], attackingRays);
        
        generatePinnedPiecesBitboards(BLACK, pinnedPieces, pinDirections);

        Movegen::generateLegalMoves<BLACK>(*this, legalMoves, numAttackers, attackingRays, pinnedPieces, pinDirections);
    }
}

Array<Move, 256> Board::generateLegalCaptures() const noexcept {
    Array<Move, 256> legalCaptures;
    generateLegalCaptures(legalCaptures);
    return legalCaptures;
}

void Board::generateLegalCaptures(Array<Move, 256>& legalCaptures) const noexcept {
    Bitboard attackingRays, pinnedPieces;
    int numAttackers = 0;
    Array<int, 64> pinDirections;
    pinDirections.resize(64);

    if(side == WHITE) {
        if(isCheck())
            numAttackers = numSquareAttackers(pieceBitboard[WHITE_KING].getFSB(), BLACK, pieceBitboard[ALL_PIECES] | pieceBitboard[BLACK_KING], attackingRays);
        
        generatePinnedPiecesBitboards(WHITE, pinnedPieces, pinDirections);

        Movegen::generateLegalCaptures<WHITE>(*this, legalCaptures, numAttackers, attackingRays, pinnedPieces, pinDirections);
    } else {
        if(isCheck())
            numAttackers = numSquareAttackers(pieceBitboard[BLACK_KING].getFSB(), WHITE, pieceBitboard[ALL_PIECES] | pieceBitboard[WHITE_KING], attackingRays);
        
        generatePinnedPiecesBitboards(BLACK, pinnedPieces, pinDirections);

        Movegen::generateLegalCaptures<BLACK>(*this, legalCaptures, numAttackers, attackingRays, pinnedPieces, pinDirections);
    }
}

std::string Board::toFEN() const {
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

    int fullMoves = (age / 2) + 1;

    fen += std::to_string(fullMoves);

    return fen;
}

std::string Board::toPGN(const PGNData& data) const {
    std::stringstream pgn;
    std::string fenOverwrite = "";

    Board temp = *this;
    std::vector<Move> moves;

    for(size_t i = 0; i < moveHistory.size(); i++)
        temp.undoMove();

    // Wenn das Spiel nicht im Startzustand begann,
    // muss die FEN-Notation im PGN gespeichert werden
    if(temp != Board())
        fenOverwrite = temp.toFEN();

    for(auto& [key, value] : data.metadata) {
        if(fenOverwrite == "" || (key != "FEN" && key != "SetUp"))
            pgn << "[" << key << " \"" << value << "\"]\n";
    }

    if(fenOverwrite != "")
        pgn << "[SetUp \"1\"]\n[FEN \"" << fenOverwrite << "\"]\n";

    pgn << "\n";

    // Sortiere die Kommentare nach Zugnummer
    std::vector<std::tuple<std::string, size_t>> comments = data.comments;
    std::sort(comments.begin(), comments.end(), [](const std::tuple<std::string, size_t>& a, const std::tuple<std::string, size_t>& b) {
        return std::get<1>(a) < std::get<1>(b);
    });

    unsigned int halfMoves = temp.age;
    bool white = halfMoves % 2 == 0;
    size_t commentIndex = 0, moveIndex = 1;

    while(commentIndex < comments.size() && std::get<1>(comments[commentIndex]) == 0) {
        pgn << "{" << std::get<0>(comments[commentIndex]) << "} ";
        commentIndex++;
    }

    if(!white)
        pgn << std::to_string(halfMoves / 2 + 1) << ". ... ";

    for(MoveHistoryEntry entry : moveHistory) {
        if(white)
            pgn << std::to_string(halfMoves / 2 + 1) << ". ";

        pgn << toStandardAlgebraicNotation(entry.move, temp) << " ";

        while(commentIndex < comments.size() && std::get<1>(comments[commentIndex]) == moveIndex) {
            pgn << "{" << std::get<0>(comments[commentIndex]) << "} ";
            commentIndex++;
        }

        temp.makeMove(entry.move);

        white = !white;
        halfMoves++;
        moveIndex++;
    }

    if(Referee::isCheckmate(temp)) {
        if(white)
            pgn << "0-1";
        else
            pgn << "1-0";
    } else if(Referee::isDraw(temp))
        pgn << "1/2-1/2";
    else {
        switch(data.result) {
            case PGNData::Result::WHITE_WINS:
                pgn << "1-0";
                break;
            case PGNData::Result::BLACK_WINS:
                pgn << "0-1";
                break;
            case PGNData::Result::DRAW:
                pgn << "1/2-1/2";
                break;
            case PGNData::Result::ONGOING:
                pgn << "*";
                break;
        }
    }

    return pgn.str();
}

unsigned int Board::repetitionCount() const {
    unsigned int count = 1;

    for(int i = moveHistory.size() - 2; i >= std::max((int)moveHistory.size() - fiftyMoveRule, 0); i -= 2) {
        if(moveHistory[i].hashValue == hashValue) {
            if(++count == 3)
                return count;
        }
    }

    return count;
}