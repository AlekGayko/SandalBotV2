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
