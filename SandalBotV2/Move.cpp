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
	this->moveValue = (startingSquare << 6) | targetSquare;
	this->startSquare = startingSquare;
	this->targetSquare = targetSquare;
}

Move::Move(int startingSquare, int targetSquare, int flag) {
	this->moveValue = (flag << 12) | (startingSquare << 6) | targetSquare;
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

Move& Move::operator=(const Move& other) {
	this->moveValue = other.moveValue;
	this->startSquare = other.startSquare;
	this->targetSquare = other.targetSquare;
	this->flag = other.flag;

	return *this;
}

std::ostream& operator<<(std::ostream& os, const Move& move) {
	os << CoordHelper::indexToString(move.startSquare) << CoordHelper::indexToString(move.targetSquare);
	if (move.flag > Move::castleFlag) {
		std::string flag;
		switch (move.flag) {
		case Move::promoteToQueenFlag:
			flag = "q";
			break;
		case Move::promoteToRookFlag:
			flag = "r";
			break;
		case Move::promoteToBishopFlag:
			flag = "b";
			break;
		case Move::promoteToKnightFlag:
			flag = "n";
			break;
		}
	}
	return os;
}
