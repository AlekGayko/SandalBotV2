#include "Move.h"

Move::Move() {
	
}

Move::Move(unsigned short int move) {
	moveValue = move;
	startSquare = (move & startingSquareMask) >> 6;
	targetSquare = move & targetSquareMask;
	flag = move & flagMask;
}

Move::Move(int startingSquare, int targetSquare) {
	moveValue = (startingSquare << 6) & targetSquare;
	startSquare = startingSquare;
	targetSquare = targetSquare;
}

Move::Move(int startingSquare, int targetSquare, int flag) {
	moveValue = (flag << 12) & (startingSquare << 6) & targetSquare;
	startSquare = startingSquare;
	targetSquare = targetSquare;
	flag = flag;
}

bool Move::operator==(const Move& other) const {
	if (this == &other) {
		return true;
	}
	return moveValue == other.moveValue;
}

bool Move::isPromotion() {
	return flag >= promoteToQueenFlag;
}

int Move::promotionPieceType() {
	switch (flag) {
	case promoteToQueenFlag:
		return 0;
	case promoteToRookFlag:
		return 0;
	case promoteToBishopFlag:
		return 0;
	case promoteToKnightFlag:
		return 0;
	default:
		return 0;
	}
}
