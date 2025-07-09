#ifndef PIECE_H
#define PIECE_H

#include <bitset>

namespace SandalBot {

	// Piece is a utility class used for pieces in the Board class and beyond.
	// Defines the numerical identifiers of pieces and abstractions for those
	// pieces through piece variables
	class Piece {
	public:
		// Pieces
		static constexpr int king{ 0b0110 }; // 6
		static constexpr int queen{ 0b0101 }; // 5
		static constexpr int rook{ 0b0100 }; // 4
		static constexpr int bishop{ 0b0011 }; // 3
		static constexpr int knight{ 0b0010 }; // 2
		static constexpr int pawn{ 0b0001 }; // 1
		static constexpr int empty{ 0b0000 }; // 0

		// Colors
		static constexpr int black{ 0b0000 }; // 0
		static constexpr int white{ 0b1000 }; // 8

		// Black pieces
		static constexpr int blackKing{ king | black }; // 6
		static constexpr int blackQueen{ queen | black }; // 5
		static constexpr int blackRook{ rook | black }; // 4
		static constexpr int blackBishop{ bishop | black }; // 3
		static constexpr int blackKnight{ knight | black }; // 2
		static constexpr int blackPawn{ pawn | black }; // 1

		// White pieces
		static constexpr int whiteKing{ king | white }; // 14
		static constexpr int whiteQueen{ queen | white }; // 13
		static constexpr int whiteRook{ rook | white }; // 12
		static constexpr int whiteBishop{ bishop | white }; // 11
		static constexpr int whiteKnight{ knight | white }; // 10
		static constexpr int whitePawn{ pawn | white }; // 9

		// Masks for identifying piece information
		static constexpr int pieceMask{ 0b0111 };
		static constexpr int colorMask{ 0b1000 };

		// Easy iterator access to pieces
		static constexpr int pieces[12] {
			blackKing, blackQueen, blackRook, blackBishop, blackKnight, blackPawn,
			whiteKing, whiteQueen, whiteRook, whiteBishop, whiteKnight, whitePawn
		};

		// Constructs piece
		static constexpr int makePiece(int pieceType, int color) { return pieceType | color; }
		// Constructs piece
		static constexpr int makePiece(int pieceType, bool isWhite) { return makePiece(pieceType, isWhite ? white : black); }
		// Returns the color of piece
		static constexpr int color(int piece) { return piece & colorMask; }
		// Returns the type of a piece
		static constexpr int type(int piece) { return piece & pieceMask; }
		// Returns true if piece has same color as parameter color
		static constexpr bool isColor(int piece, int color) { return (piece & colorMask) == color && piece != empty; }
		// Returns true if piece has same type as parameter type
		static constexpr bool isType(int piece, int pieceType) { return (piece & pieceMask) == pieceType; }
		// Returns true if both pieces have same type and color
		static constexpr bool isPiece(int piece1, int piece2) { return piece1 == piece2; }
		// Returns true if piece can move diagonally and/or orthogonally
		static constexpr bool isSlidingPiece(int piece) {
			int pieceType = piece & pieceMask;
			return pieceType == queen || pieceType == rook || pieceType == bishop;
		}
		// Returns true if piece moves orthogonally
		static constexpr bool isOrthogonal(int piece) {
			int pieceType = piece & pieceMask;
			return pieceType == queen || pieceType == rook;
		}
		// Returns true if piece moves diagonally
		static constexpr bool isDiagonal(int piece) {
			int pieceType = piece & pieceMask;
			return pieceType == queen || pieceType == bishop;
		}

		static int symbolToPiece(char symbol);
		static char pieceToSymbol(int piece);
	};

}

#endif