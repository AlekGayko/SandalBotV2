#ifndef BOARD_H
#define BOARD_H

#include "BoardHistory.h"
#include "CoordHelper.h"
#include "Move.h"
#include "StateHistory.h"
#include "Types.h"

#include <string_view>

namespace SandalBot {

	struct BoardState;

	// Board class encapsulates the current and previous states of the board
	// including piece positions, and previous positions
	class Board {
		friend class Searcher;
	public:
		Piece squares[SQUARES_NB]; // Represents the pieces on each board square

		BoardHistory history{}; // Stores position history, used for detecting draw by repetition
		StateHistory stateHistory{}; // Stores history for states, used for rolling back moves
		BoardState* state{ nullptr }; // Points to most current state in stateHistory

		// Bitboards
		Bitboard typesBB[PIECE_TYPE_NB];
		Bitboard colorsBB[COLOR_NB];

		Square kingSquares[COLOR_NB];

		int pieceCount[PIECE_NB];

		int sideValues[COLOR_NB];
		int pieceSquareValues[COLOR_NB];
		int MMPieces[COLOR_NB]; // Number of major and minor pieces

		Board();

		void loadPosition(std::string_view fen);
		void makeMove(Move move);
		void unMakeMove();
		void printBoard() const;
		void printBitboards() const;
		Color sideToMove() const { return mSideToMove; }
		int moveCounter() const { return mMoveCounter; }
	private:
		Color mSideToMove;
		int mMoveCounter{ 0 };

		void initBitboards();

		void movePiece(Square from, Square to);
		void placePiece(Piece piece, Square sq);
		void deletePiece(Square sq);
	};

}

#endif // !BOARD_H
