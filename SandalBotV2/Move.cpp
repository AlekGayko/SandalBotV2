#include "Move.h"

#include <iostream>

Move::Move() {
	
}

Move::Move(unsigned short int move) {
	this->moveValue = move;
	this->startSquare = (move & startingSquareMask) >> 6;
	this->targetSquare = move & targetSquareMask;
	this->flag = move & flagMask;
}

Move::Move(int startingSquare, int targetSquare) {
	this->moveValue = (startingSquare << 6) & targetSquare;
	this->startSquare = startingSquare;
	this->targetSquare = targetSquare;
}

Move::Move(int startingSquare, int targetSquare, int flag) {
	this->moveValue = (flag << 12) & (startingSquare << 6) & targetSquare;
	this->startSquare = startingSquare;
	this->targetSquare = targetSquare;
	this->flag = flag;
}

bool Move::operator==(const Move& other) const {
	if (this == &other) {
		return true;
	}
	return moveValue == other.moveValue;
}

constexpr bool Move::isPromotion() const {
	return flag >= promoteToQueenFlag;
}

constexpr bool Move::isEnPassant() const {
	return flag == enPassantCaptureFlag;
}

constexpr bool Move::isCastle() const {
	return flag == castleFlag;
}

constexpr int Move::promotionPieceType() const {
	switch (flag) {
	case promoteToQueenFlag:
		return Piece::queen;
	case promoteToRookFlag:
		return Piece::rook;
	case promoteToBishopFlag:
		return Piece::bishop;
	case promoteToKnightFlag:
		return Piece::knight;
	default:
		return 0;
	}
}

std::ostream& operator<<(std::ostream& os, const Move& move) {
	os << "startSquare: " << move.startSquare << ", targetsquare: " << move.targetSquare << ", takenpiece: " << move.takenPiece << ", flag: " << move.flag << std::endl;
	return os;
}
