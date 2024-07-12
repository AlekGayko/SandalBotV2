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

	inline static constexpr int makePiece(int pieceType, int color);
	inline static constexpr int makePiece(int pieceType, bool isWhite);
	inline static constexpr int color(int piece);
	inline static constexpr int type(int piece);
	inline static constexpr bool isColor(int piece, int color);
	inline static constexpr bool isType(int piece, int pieceType);
	inline static constexpr bool isPiece(int piece1, int piece2);
	inline static constexpr bool isSlidingPiece(int piece);
	inline static constexpr bool isOrthogonal(int piece);
	inline static constexpr bool isDiagonal(int piece);

	inline static int symbolToPiece(char symbol);
	inline static char pieceToSymbol(int piece);

};

#endif