#include "BoardState.h"

BoardState::BoardState() {

}

BoardState::BoardState(bool whiteTurn, int capturedPiece, int enPassantFile, int castlingRights, int fiftyMoveCounter, unsigned long long int zobristHash) {
	this->whiteTurn = whiteTurn;
	this->capturedPiece = capturedPiece;
	this->enPassantFile = enPassantFile;
	this->castlingRights = castlingRights;
	this->fiftyMoveCounter = fiftyMoveCounter;
	this->zobristHash = zobristHash;
}

constexpr bool BoardState::canShortCastle(bool isWhite) const {
	return isWhite ? castlingRights & whiteShortCastleMask : castlingRights & blackShortCastleMask;
}

constexpr bool BoardState::canLongCastle(bool isWhite) const {
	return isWhite ? castlingRights & whiteLongCastleMask : castlingRights & blackLongCastleMask;
}

void BoardState::nextMove() {
	whiteTurn = !whiteTurn;
}

void BoardState::prevMove() {
	whiteTurn = !whiteTurn;
}
