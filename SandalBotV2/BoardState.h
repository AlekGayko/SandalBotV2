#pragma once

#ifndef BOARDSTATE_H
#define BOARDSTATE_H

#include "Move.h"
#include "Piece.h"

#include <bitset>

struct BoardState {
	bool whiteTurn = true;
	int capturedPiece;
	int enPassantSquare;
	int castlingRights = 0b0000;
	int fiftyMoveCounter;
	int moveCounter;

	uint64_t zobristHash;

	static constexpr int whiteShortCastleMask = 0b0001;
	static constexpr int whiteLongCastleMask = 0b0010;
	static constexpr int blackShortCastleMask = 0b0100;
	static constexpr int blackLongCastleMask = 0b1000;

	static constexpr int whiteCastleMask = 0b0011;
	static constexpr int blackCastleMask = 0b1100;

	BoardState();
	BoardState(bool whiteTurn, int capturedPiece, int enPassantFile, int castlingRights, int fiftyMoveCounter, int moves, uint64_t zobristHash);
	constexpr bool canShortCastle(bool isWhite) const;
	constexpr bool canLongCastle(bool isWhite) const;
};

#endif // !BOARDSTATE_H
