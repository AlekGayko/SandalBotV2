#include "Piece.h"

#include <cctype>

namespace SandalBot::Piece {

    // Returns corresponding integer piece from given character symbol
    int Piece::symbolToPiece(char symbol) {
        int color = black;
        int pieceType = 0;
        if (std::isupper(symbol)) { // Uppercase symbols are white
            color = white;
        }

        // Get piece type
        switch (std::tolower(symbol)) { // Convert to lower for white and black
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

    // Returns corresponding character symbol from integer piece
    char Piece::pieceToSymbol(int piece) {
        char symbol = '_';

        switch (piece & pieceMask) { // Get piece type
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
        // If piece is white, convert to uppercase character
        if (isColor(piece, white)) {
            symbol = std::toupper(symbol);
        }

        return symbol;
    }

}