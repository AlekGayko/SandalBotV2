#include "Piece.h"

#include <cctype>

namespace SandalBot {

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

}