#pragma once

#ifndef PIECE_H
#define PIECE_H

#include <bitset>

class Piece {
public:
	static constexpr int king = 0b0110; // 6
	static constexpr int queen = 0b0101; // 5
	static constexpr int rook = 0b0100; // 4
	static constexpr int bishop = 0b0011; // 3
	static constexpr int knight = 0b0010; // 2
	static constexpr int pawn = 0b0001; // 1
	static constexpr int empty = 0b0000; // 0

	static constexpr int black = 0b0000; // 0
	static constexpr int white = 0b1000; // 8

	static constexpr int blackKing = king | black; // 6
	static constexpr int blackQueen = queen | black; // 5
	static constexpr int blackRook = rook | black; // 4
	static constexpr int blackBishop = bishop | black; // 3
	static constexpr int blackKnight = knight | black; // 2
	static constexpr int blackPawn = pawn | black; // 1

	static constexpr int whiteKing = king | white; // 14
	static constexpr int whiteQueen = queen | white; // 13
	static constexpr int whiteRook = rook | white; // 12
	static constexpr int whiteBishop = bishop | white; // 11
	static constexpr int whiteKnight = knight | white; // 10
	static constexpr int whitePawn = pawn | white; // 9

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