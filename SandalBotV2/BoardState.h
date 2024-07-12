#pragma once

#ifndef BOARDSTATE_H
#define BOARDSTATE_H

#include <bitset>

class BoardState {
private:
	
public:
	bool check = false;
	bool whiteTurn;
	int capturedPiece;
	int enPassantFile;
	int castlingRights = 0b0000;
	int fiftyMoveCounter;
	unsigned long long int zobristHash;

	static constexpr int whiteShortCastleMask = 0b0001;
	static constexpr int whiteLongCastleMask = 0b0010;
	static constexpr int blackShortCastleMask = 0b0100;
	static constexpr int blackLongCastleMask = 0b1000;

	BoardState();
	BoardState(bool whiteTurn, int capturedPiece, int enPassantFile, int castlingRights, int fiftyMoveCounter, unsigned long long int zobristHash);
	bool canShortCastle(bool isWhite);
	bool canLongCastle(bool isWhite);
};

#endif // !BOARDSTATE_H
