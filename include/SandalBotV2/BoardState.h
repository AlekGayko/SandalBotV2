#ifndef BOARDSTATE_H
#define BOARDSTATE_H

#include "Move.h"
#include "Types.h"

#include <bitset>

namespace SandalBot {

	// BoardState holds state information of a board at a single position
	struct BoardState {
		BoardState() {}

		BoardState(Piece capturedPiece, Square enPassantSquare, CastlingRights cr, int fiftyMoveCounter, HashKey zobristHash, Move move)
			: capturedPiece(capturedPiece), enPassantSquare(enPassantSquare), cr(cr),
			fiftyMoveCounter(fiftyMoveCounter), zobristHash(zobristHash), prevMove(move) {
		}

		BoardState(BoardState&& other) noexcept
			: capturedPiece(other.capturedPiece), enPassantSquare(other.enPassantSquare), cr(other.cr),
			fiftyMoveCounter(other.fiftyMoveCounter), zobristHash(other.zobristHash), prevMove(other.prevMove) {
		}

		BoardState& operator=(const BoardState& other) {
			this->capturedPiece = other.capturedPiece;
			this->enPassantSquare = other.enPassantSquare;
			this->cr = other.cr;
			this->fiftyMoveCounter = other.fiftyMoveCounter;
			this->zobristHash = other.zobristHash;
			this->prevMove = other.prevMove;
			return *this;
		}

		BoardState& operator=(BoardState&& other) noexcept {
			this->capturedPiece = other.capturedPiece;
			this->enPassantSquare = other.enPassantSquare;
			this->cr = other.cr;
			this->fiftyMoveCounter = other.fiftyMoveCounter;
			this->zobristHash = other.zobristHash;
			this->prevMove = other.prevMove;
			return *this;
		}

		// State information
		HashKey zobristHash{};
		int16_t fiftyMoveCounter{};
		Move prevMove{};
		Piece capturedPiece{};
		Square enPassantSquare{};
		CastlingRights cr{}; // Store castling rights in binary form to conserve memory
	};

}

#endif // !BOARDSTATE_H
