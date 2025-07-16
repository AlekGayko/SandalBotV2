#include "Types.h"

#include <cctype>

namespace SandalBot {

    // Returns corresponding integer piece from given character symbol
    Piece symbolToPiece(char symbol) {
        Color color = BLACK;
        PieceType pieceType = NO_PIECE_TYPE;

        if (std::isupper(symbol)) { // Uppercase symbols are white
            color = WHITE;
        }

        // Get piece type
        switch (std::tolower(symbol)) { // Convert to lower for white and black
        case 'k':
            pieceType = KING;
            break;
        case 'q':
            pieceType = QUEEN;
            break;
        case 'r':
            pieceType = ROOK;
            break;
        case 'b':
            pieceType = BISHOP;
            break;
        case 'n':
            pieceType = KNIGHT;
            break;
        case 'p':
            pieceType = PAWN;
            break;
        default:
            return NO_PIECE;
        }
        return makePiece(pieceType, color);
    }

    // Returns corresponding character symbol from integer piece
    char pieceToSymbol(Piece piece) {
        char symbol = '_';

        switch (piece & pieceMask) { // Get piece type
        case KING:
            symbol = 'k';
            break;
        case QUEEN:
            symbol = 'q';
            break;
        case ROOK:
            symbol = 'r';
            break;
        case BISHOP:
            symbol = 'b';
            break;
        case KNIGHT:
            symbol = 'n';
            break;
        case PAWN:
            symbol = 'p';
            break;
        default:
            return symbol;
        }
        // If piece is white, convert to uppercase character
        if (isColor(piece, WHITE)) {
            symbol = std::toupper(symbol);
        }

        return symbol;
    }

}