#include "Piece.h"

#include <cctype>

constexpr int Piece::makePiece(int pieceType, int color) {
    return pieceType | color;
}

constexpr int Piece::makePiece(int pieceType, bool isWhite) {
    return makePiece(pieceType, isWhite ? white : black);
}

constexpr bool Piece::isColor(int piece, int color) {
    return (piece & colorMask) == color;
}

constexpr bool Piece::isType(int piece, int pieceType) {
    return (piece & pieceMask) == pieceType;
}

constexpr bool Piece::isPiece(int piece1, int piece2) {
    return piece1 == piece2;
}

constexpr bool Piece::isOrthogonal(int piece) {
    int pieceType = piece & pieceMask;
    return pieceType == queen || pieceType == rook;
}

constexpr bool Piece::isDiagonal(int piece) {
    int pieceType = piece & pieceMask;
    return pieceType == queen || pieceType == bishop;
}

int Piece::symbolToPiece(char symbol) {
    int color = black;
    int pieceType = 0;
    if (std::isupper(symbol)) {
        color = white;
    }

    switch (std::tolower(symbol)) {
    case 'k': 
        pieceType = king;
        break;
    case 'q':
        pieceType = queen;
        break;
    case 'r':
        pieceType = rook;
        break;
    case 'b':
        pieceType = bishop;
        break;
    case 'n':
        pieceType = knight;
        break;
    case 'p':
        pieceType = pawn;
        break;
    default:
        return 0;
    }
    return makePiece(pieceType, color);
}

char Piece::pieceToSymbol(int piece) {
    char symbol = '_';

    switch (piece & pieceMask) {
    case king: 
        symbol = 'k';
        break;
    case queen:
        symbol = 'q';
        break;
    case rook:
        symbol = 'r';
        break;
    case bishop:
        symbol = 'b';
        break;
    case knight:
        symbol = 'n';
        break;
    case pawn:
        symbol = 'p';
        break;
    default:
        return symbol;
    }

    if (isColor(piece, white)) {
        symbol = std::toupper(symbol);
    }

    return symbol;
}
