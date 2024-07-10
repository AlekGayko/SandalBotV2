#pragma once

#ifndef MOVE_H
#define MOVE_H

#include <bitset>

class Move {
private:
public:
	static constexpr short int targetSquareMask = 0b111111;
	static constexpr short int startingSquareMask = 0b111111000000;
	static constexpr short int flagMask = 0b1111 << 12;

	static constexpr short int noFlag = 0b0000;
	static constexpr short int enPassantCaptureFlag = 0b0001;
	static constexpr short int pawnTwoSquaresFlag = 0b0010;
	static constexpr short int castleFlag = 0b0011;
	
	static constexpr short int promoteToQueenFlag = 0b0100;
	static constexpr short int promoteToRookFlag = 0b0101;
	static constexpr short int promoteToBishopFlag = 0b0110;
	static constexpr short int promoteToKnightFlag = 0b0111;

	unsigned short int moveValue;
	short int startSquare;
	short int targetSquare;
	short int flag;
	Move();
	Move(unsigned short int move);
	Move(int startingSquare, int targetSquare);
	Move(int startingSquare, int targetSquare, int flag);

	Move& operator=(const Move& other);
};

#endif // !MOVE_H
