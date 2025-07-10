#ifndef PIECE_H
#define PIECE_H

#include <bitset>

// Piece is a utility namesapce used for pieces in the Board class and beyond.
// Defines the numerical identifiers of pieces and abstractions for those
// pieces through piece variables
namespace SandalBot::Piece {

	// Pieces
	constexpr int king{ 0b0110 }; // 6
	constexpr int queen{ 0b0101 }; // 5
	constexpr int rook{ 0b0100 }; // 4
	constexpr int bishop{ 0b0011 }; // 3
	constexpr int knight{ 0b0010 }; // 2
	constexpr int pawn{ 0b0001 }; // 1
	constexpr int empty{ 0b0000 }; // 0

	// Colors
	constexpr int black{ 0b0000 }; // 0
	constexpr int white{ 0b1000 }; // 8

	// Black pieces
	constexpr int blackKing{ king | black }; // 6
	constexpr int blackQueen{ queen | black }; // 5
	constexpr int blackRook{ rook | black }; // 4
	constexpr int blackBishop{ bishop | black }; // 3
	constexpr int blackKnight{ knight | black }; // 2
	constexpr int blackPawn{ pawn | black }; // 1

	// White pieces
	constexpr int whiteKing{ king | white }; // 14
	constexpr int whiteQueen{ queen | white }; // 13
	constexpr int whiteRook{ rook | white }; // 12
	constexpr int whiteBishop{ bishop | white }; // 11
	constexpr int whiteKnight{ knight | white }; // 10
	constexpr int whitePawn{ pawn | white }; // 9

	// Masks for identifying piece information
	constexpr int pieceMask{ 0b0111 };
	constexpr int colorMask{ 0b1000 };

	// Easy iterator access to pieces
	constexpr int pieces[12] {
		blackKing, blackQueen, blackRook, blackBishop, blackKnight, blackPawn,
		whiteKing, whiteQueen, whiteRook, whiteBishop, whiteKnight, whitePawn
	};

	// Constructs piece
	constexpr int makePiece(int pieceType, int color) { return pieceType | color; }
	// Constructs piece
	constexpr int makePiece(int pieceType, bool isWhite) { return makePiece(pieceType, isWhite ? white : black); }
	// Returns the color of piece
	constexpr int color(int piece) { return piece & colorMask; }
	// Returns the type of a piece
	constexpr int type(int piece) { return piece & pieceMask; }
	// Returns true if piece has same color as parameter color
	constexpr bool isColor(int piece, int color) { return (piece & colorMask) == color && piece != empty; }
	// Returns true if piece has same type as parameter type
	constexpr bool isType(int piece, int pieceType) { return (piece & pieceMask) == pieceType; }
	// Returns true if both pieces have same type and color
	constexpr bool isPiece(int piece1, int piece2) { return piece1 == piece2; }
	// Returns true if piece can move diagonally and/or orthogonally
	constexpr bool isSlidingPiece(int piece) {
		int pieceType = piece & pieceMask;
		return pieceType == queen || pieceType == rook || pieceType == bishop;
	}
	// Returns true if piece moves orthogonally
	constexpr bool isOrthogonal(int piece) {
		int pieceType = piece & pieceMask;
		return pieceType == queen || pieceType == rook;
	}
	// Returns true if piece moves diagonally
	constexpr bool isDiagonal(int piece) {
		int pieceType = piece & pieceMask;
		return pieceType == queen || pieceType == bishop;
	}

	int symbolToPiece(char symbol);
	char pieceToSymbol(int piece);

}

#endif