#pragma once

#ifndef BOARDSTATE_H
#define BOARDSTATE_H

#include "Move.h"
#include "Piece.h"

#include <bitset>

class BoardState {
private:
	
public:
	bool check = false;
	bool whiteTurn = true;
	int capturedPiece;
	int enPassantFile;
	int castlingRights = 0b0000;
	int fiftyMoveCounter;
	int moveCounter;

	unsigned long long int zobristHash;

	static constexpr int whiteShortCastleMask = 0b0001;
	static constexpr int whiteLongCastleMask = 0b0010;
	static constexpr int blackShortCastleMask = 0b0100;
	static constexpr int blackLongCastleMask = 0b1000;

	static constexpr int whiteCastleMask = 0b0011;
	static constexpr int blackCastleMask = 0b1100;

	BoardState();
	BoardState(bool whiteTurn, int capturedPiece, int enPassantFile, int castlingRights, int fiftyMoveCounter, int moves, unsigned long long int zobristHash);
	constexpr bool canShortCastle(bool isWhite) const;
	constexpr bool canLongCastle(bool isWhite) const;
	void nextMove(Move& move, int piece);
	void prevMove(Move& move, int piece);
};

#endif // !BOARDSTATE_H
