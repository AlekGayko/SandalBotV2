#pragma once

#ifndef PIECE_H
#define PIECE_H

#include <bitset>

class Piece {
public:
	static constexpr int king = 0b0110;
	static constexpr int queen = 0b0101;
	static constexpr int rook = 0b0100;
	static constexpr int bishop = 0b0011;
	static constexpr int knight = 0b0010;
	static constexpr int pawn = 0b0001;
	static constexpr int empty = 0b0000;

	static constexpr int black = 0b0000;
	static constexpr int white = 0b1000;

	static constexpr int blackKing = king | black;
	static constexpr int blackQueen = queen | black;
	static constexpr int blackRook = rook | black;
	static constexpr int blackBishop = bishop | black;
	static constexpr int blackKnight = knight | black;
	static constexpr int blackPawn = pawn | black;

	static constexpr int whiteKing = king | white;
	static constexpr int whiteQueen = queen | white;
	static constexpr int whiteRook = rook | white;
	static constexpr int whiteBishop = bishop | white;
	static constexpr int whiteKnight = knight | white;
	static constexpr int whitePawn = pawn | white;

	static constexpr int pieceMask = 0b0111;
	static constexpr int colorMask = 0b1000;

	static constexpr int pieces[12] = {
		blackKing, blackQueen, blackRook, blackBishop, blackKnight, blackPawn,
		whiteKing, whiteQueen, whiteRook, whiteBishop, whiteKnight, whitePawn
	};

	 static int makePiece(int pieceType, int color);
	 static int makePiece(int pieceType, bool isWhite);
	 static int color(int piece);
	 static int type(int piece);
	 static bool isColor(int piece, int color);
	 static bool isType(int piece, int pieceType);
	 static bool isPiece(int piece1, int piece2);
	 static bool isSlidingPiece(int piece);
	 static bool isOrthogonal(int piece);
	 static bool isDiagonal(int piece);

	static int symbolToPiece(char symbol);
	static char pieceToSymbol(int piece);

};

#endif