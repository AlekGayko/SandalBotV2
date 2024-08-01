#include "BoardState.h"

BoardState::BoardState() {

}

BoardState::BoardState(bool whiteTurn, int capturedPiece, int enPassantSquare, int castlingRights, int fiftyMoveCounter, int moveCounter, uint64_t zobristHash) {
	this->whiteTurn = whiteTurn;
	this->capturedPiece = capturedPiece;
	this->enPassantSquare = enPassantSquare;
	this->castlingRights = castlingRights;
	this->fiftyMoveCounter = fiftyMoveCounter;
	this->moveCounter = moveCounter;
	this->zobristHash = zobristHash;
}

constexpr bool BoardState::canShortCastle(bool isWhite) const {
	return isWhite ? castlingRights & whiteShortCastleMask : castlingRights & blackShortCastleMask;
}

constexpr bool BoardState::canLongCastle(bool isWhite) const {
	return isWhite ? castlingRights & whiteLongCastleMask : castlingRights & blackLongCastleMask;
}