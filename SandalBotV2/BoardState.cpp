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

BoardState::BoardState(BoardState&& other) {
	this->whiteTurn = other.whiteTurn;
	this->capturedPiece = other.capturedPiece;
	this->enPassantSquare = other.enPassantSquare;
	this->castlingRights = other.castlingRights;
	this->fiftyMoveCounter = other.fiftyMoveCounter;
	this->moveCounter = other.moveCounter;
	this->zobristHash = other.zobristHash;
}

BoardState& BoardState::operator=(const BoardState& other) {
	this->whiteTurn = other.whiteTurn;
	this->capturedPiece = other.capturedPiece;
	this->enPassantSquare = other.enPassantSquare;
	this->castlingRights = other.castlingRights;
	this->fiftyMoveCounter = other.fiftyMoveCounter;
	this->moveCounter = other.moveCounter;
	this->zobristHash = other.zobristHash;
	return *this;
}

constexpr bool BoardState::canShortCastle(bool isWhite) const {
	return isWhite ? castlingRights & whiteShortCastleMask : castlingRights & blackShortCastleMask;
}

constexpr bool BoardState::canLongCastle(bool isWhite) const {
	return isWhite ? castlingRights & whiteLongCastleMask : castlingRights & blackLongCastleMask;
}