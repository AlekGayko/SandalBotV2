#ifndef BOARDSTATE_H
#define BOARDSTATE_H

#include "Move.h"
#include "Piece.h"

#include <bitset>

namespace SandalBot {

	struct BoardState {
		static constexpr int whiteShortCastleMask{ 0b0001 };
		static constexpr int whiteLongCastleMask{ 0b0010 };
		static constexpr int blackShortCastleMask{ 0b0100 };
		static constexpr int blackLongCastleMask{ 0b1000 };

		static constexpr int whiteCastleMask{ 0b0011 };
		static constexpr int blackCastleMask{ 0b1100 };

		bool whiteTurn{ true };
		int capturedPiece{};
		int enPassantSquare{};
		int castlingRights{ 0b0000 };
		int fiftyMoveCounter{};
		int moveCounter{};

		uint64_t zobristHash{};

		BoardState() {}

		BoardState(bool whiteTurn, int capturedPiece, int enPassantSquare, int castlingRights, int fiftyMoveCounter, int moveCounter, uint64_t zobristHash)
			: whiteTurn(whiteTurn), capturedPiece(capturedPiece), enPassantSquare(enPassantSquare), castlingRights(castlingRights), 
			fiftyMoveCounter(fiftyMoveCounter), moveCounter(moveCounter), zobristHash(zobristHash) {}

		BoardState(BoardState&& other) noexcept
			: whiteTurn(other.whiteTurn), capturedPiece(other.capturedPiece), enPassantSquare(other.enPassantSquare), castlingRights(other.castlingRights),
			fiftyMoveCounter(other.fiftyMoveCounter), moveCounter(other.moveCounter), zobristHash(other.zobristHash) {}
	
		BoardState& operator=(const BoardState& other) {
			this->whiteTurn = other.whiteTurn;
			this->capturedPiece = other.capturedPiece;
			this->enPassantSquare = other.enPassantSquare;
			this->castlingRights = other.castlingRights;
			this->fiftyMoveCounter = other.fiftyMoveCounter;
			this->moveCounter = other.moveCounter;
			this->zobristHash = other.zobristHash;
			return *this;
		}
		BoardState& operator=(BoardState&& other) noexcept {
			this->whiteTurn = other.whiteTurn;
			this->capturedPiece = other.capturedPiece;
			this->enPassantSquare = other.enPassantSquare;
			this->castlingRights = other.castlingRights;
			this->fiftyMoveCounter = other.fiftyMoveCounter;
			this->moveCounter = other.moveCounter;
			this->zobristHash = other.zobristHash;
			return *this;
		}

		constexpr bool canShortCastle(bool isWhite) const { 
			return isWhite ? castlingRights & whiteShortCastleMask : castlingRights & blackShortCastleMask; 
		}
		constexpr bool canLongCastle(bool isWhite) const { 
			return isWhite ? castlingRights & whiteLongCastleMask : castlingRights & blackLongCastleMask; 
		}
	};

}

#endif // !BOARDSTATE_H
