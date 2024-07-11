#include "BoardState.h"

BoardState::BoardState(bool whiteTurn, int capturedPiece, int enPassantFile, int castlingRights, int fiftyMoveCounter, unsigned long long int zobristHash) {
	this->whiteTurn = whiteTurn;
	this->capturedPiece = capturedPiece;
	this->enPassantFile = enPassantFile;
	this->castlingRights = castlingRights;
	this->fiftyMoveCounter = fiftyMoveCounter;
	this->zobristHash = zobristHash;
}

bool BoardState::canShortCastle(bool isWhite) {
	return isWhite ? castlingRights & whiteShortCastleMask : castlingRights & blackShortCastleMask;
}

bool BoardState::canLongCastle(bool isWhite) {
	return isWhite ? castlingRights & whiteLongCastleMask : castlingRights & blackLongCastleMask;
}
