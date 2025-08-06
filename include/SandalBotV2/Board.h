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

		bool givesCheck(Move move) const;
		bool legalMove(Move move) const;

		void makeMove(Move move);
		void makeMove(Move move, bool checkGiven);
		void unMakeMove();

		void printBoard() const;
		void printBitboards() const;

		Bitboard pieces() const;
		Bitboard sidePieces(Color color) const;
		Bitboard pieces(PieceType type) const;
		Bitboard pieces(Color c, PieceType type) const;
		Bitboard pieces(PieceType type1, PieceType type2) const;

		Bitboard checkBB() const { return state->checkBB; }
		Bitboard kingBlockersBB(Color color) const { return state->checkBlockers[color]; }
		Bitboard checkSquares(PieceType type) const { return state->checkSquares[type]; }

		Bitboard threatsBB(Square sq, Color color) const;
		Bitboard threatsBB(Square sq, Color color, Bitboard occupied) const;
		Bitboard attacksBB(PieceType type, Color color) const;
		Bitboard attacksBB(Color color) const;

		Color sideToMove() const { return mSideToMove; }
		int moveCounter() const { return mMoveCounter; }

		bool seeGE(Move move, int threshold) const;
	private:
		Color mSideToMove;
		int mMoveCounter{ 0 };

		void initBitboards();

		void movePiece(Square from, Square to);
		void placePiece(Piece piece, Square sq);
		void deletePiece(Square sq);

		void setCheckState(bool checkGiven);
		template<Color Us>
		Bitboard sliderBlockers();

	};

	inline Bitboard Board::pieces() const {
		return typesBB[ALL_PIECES];
	}

	inline Bitboard Board::sidePieces(Color color) const {
		return typesBB[ALL_PIECES] & colorsBB[color];
	}

	inline Bitboard Board::pieces(PieceType type) const {
		return typesBB[type];
	}

	inline Bitboard Board::pieces(Color c, PieceType type) const {
		return typesBB[type] & colorsBB[c];
	}

	inline Bitboard Board::pieces(PieceType type1, PieceType type2) const {
		return typesBB[type1] | typesBB[type2];
	}

}

#endif // !BOARD_H
