#include "BoardState.h"

BoardState::BoardState() {

}

BoardState::BoardState(bool whiteTurn, int capturedPiece, int enPassantFile, int castlingRights, int fiftyMoveCounter, int moveCounter, unsigned long long int zobristHash) {
	this->whiteTurn = whiteTurn;
	this->capturedPiece = capturedPiece;
	this->enPassantFile = enPassantFile;
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

void BoardState::nextMove(Move& move, int piece) {
	moveCounter++;
	fiftyMoveCounter++;

	if (move.flag == Move::pawnTwoSquaresFlag) {
		enPassantFile = move.targetSquare % 8;
	} else {
		enPassantFile = -1;
	}

	if (piece == Piece::pawn) {
		fiftyMoveCounter = 0;
	} else if (move.takenPiece != Piece::empty) {
		fiftyMoveCounter = 0;
	}

	capturedPiece = move.takenPiece;
	
	whiteTurn = !whiteTurn;
}

void BoardState::prevMove(Move& move, int piece) {
	moveCounter--;
	fiftyMoveCounter--;

	if (piece == Piece::pawn) {
		fiftyMoveCounter = 0;
	} else if (move.takenPiece != Piece::empty) {
		fiftyMoveCounter = 0;
	}

	whiteTurn = !whiteTurn;
}
