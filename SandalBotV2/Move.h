#pragma once

#ifndef MOVE_H
#define MOVE_H

#include <bitset>

#include "Piece.h"

struct Move {
	static constexpr short int startingSquareMask = 0b111111000000;
	static constexpr short int targetSquareMask = 0b000000111111;
	static constexpr short int flagMask = 0b1111 << 12;

	static constexpr short int noFlag = 0b0000;
	static constexpr short int enPassantCaptureFlag = 0b0001;
	static constexpr short int pawnTwoSquaresFlag = 0b0010;
	static constexpr short int castleFlag = 0b0011;
	
	static constexpr short int promoteToQueenFlag = 0b0100;
	static constexpr short int promoteToRookFlag = 0b0101;
	static constexpr short int promoteToBishopFlag = 0b0110;
	static constexpr short int promoteToKnightFlag = 0b0111;

	unsigned short int moveValue = 0;
	short int startSquare = 0;
	short int targetSquare = 0;
	short int flag = 0;
	int takenPiece = Piece::empty;

	Move();
	Move(unsigned short int move);
	Move(int startingSquare, int targetSquare);
	Move(int startingSquare, int targetSquare, int flag);

	bool operator==(const Move& other) const;
	constexpr bool isPromotion() const;
	constexpr bool isEnPassant() const;
	constexpr bool isCastle() const;
	constexpr int promotionPieceType() const;
	friend std::ostream& operator<<(std::ostream& os, const Move& move);
};

#endif // !MOVE_H
