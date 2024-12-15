#include "Move.h"

#include <iostream>

Move::Move() {
	this->moveValue = 0;
}

Move::Move(unsigned short int move) {
	this->moveValue = move;
}

Move::Move(int startingSquare, int targetSquare) {
	this->moveValue = (startingSquare << 6) | targetSquare;
}

Move::Move(int startingSquare, int targetSquare, int flag) {
	this->moveValue = (flag << 12) | (startingSquare << 6) | targetSquare;
}

bool Move::operator==(const Move& other) {
	if (this == &other) {
		return true;
	}
	return moveValue == other.moveValue;
}

bool Move::isPromotion() {
	return getFlag() >= promoteToQueenFlag;
}

bool Move::isEnPassant() {
	return getFlag() == enPassantCaptureFlag;
}

bool Move::isCastle() {
	return getFlag() == castleFlag;
}

int Move::promotionPieceType() {
	switch (getFlag()) {
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

unsigned short int Move::getStartSquare() const {
	return (moveValue & startingSquareMask) >> 6;
}

unsigned short int Move::getTargetSquare() const {
	return moveValue & targetSquareMask;
}

unsigned short int Move::getFlag() const {
	return (moveValue & flagMask) >> 12;
}

Move& Move::operator=(const Move& other) {
	this->moveValue = other.moveValue;

	return *this;
}

bool Move::operator!=(const Move& other) {
	return moveValue != other.moveValue;
}

std::string Move::str() const {
	std::string str = "";
	str += CoordHelper::indexToString(getStartSquare());
	str += CoordHelper::indexToString(getTargetSquare());

	switch (getFlag()) {
	case castleFlag:
		if (getTargetSquare() > getStartSquare()) {
			str = "O-O";
		} else {
			str = "O-O-O";
		}
		break;
	case promoteToQueenFlag:
		str += "q";
		break;
	case promoteToRookFlag:
		str += "r";
		break;
	case promoteToBishopFlag:
		str += "b";
		break;
	case promoteToKnightFlag:
		str += "n";
		break;
	}

	return str;
}

std::string Move::binStr() const {
	std::string binaryStr = "";
	
	// Flag
	for (int i = 15; i > 11; i--) {
		binaryStr += std::to_string((moveValue >> i) & 0b1);
	}

	binaryStr += " ";

	// StartSquare
	for (int i = 11; i > 5; i--) {
		binaryStr += std::to_string((moveValue >> i) & 0b1);
	}

	binaryStr += " ";

	// TargetSquare
	for (int i = 5; i >= 0; i--) {
		binaryStr += std::to_string((moveValue >> i) & 0b1);
	}

	return binaryStr;
}

std::ostream& operator<<(std::ostream& os, const Move& move) {
	os << move.str();
	return os;
}
