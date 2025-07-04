#ifndef BOARD_H
#define BOARD_H

#include "BoardHistory.h"
#include "CoordHelper.h"
#include "Move.h"
#include "PieceList.h"
#include "StateHistory.h"

namespace SandalBot {

	struct BoardState;
	class FEN;
	class Evaluator;

	// Board class encapsulates the current and previous states of the board
	// including piece positions, and previous positions
	class Board {
		friend class Searcher;
	private:
		void loadPieceLists();
		void loadBitBoards();
		void flagMoveChanges(Move move, uint64_t& newZobristHash, int& enPassantSquare, int colorIndex, int oppositeIndex);
		int updateCastlingRights(const int piece, const int colorIndex, const int startSquare, int castlingRights);
		void makeEnPassantChanges(Move move, uint64_t& newZobristHash, int oppositeIndex);
		void makeCastlingChanges(Move move, uint64_t& newZobristHash, int colorIndex);
		int makePromotionChanges(Move move, int piece, int colorIndex);
		void undoEnPassantChanges(Move move);
		void undoCastlingChanges(Move move);
		void updateBitBoards(Move move, int pieceType, int takenPiece);
		void undoBitBoards(Move move, int pieceType, int takenPiece);
	public:
		const std::string startPosFEN{ "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" };

		int squares[64]; // Represents the pieces on each board square

		PieceList pieceLists[2][7]; // index [i][0] is redundant since 0 = empty

		BoardHistory history{}; // Stores position history, used for detecting draw by repetition
		StateHistory stateHistory{}; // Stores history for states, used for rolling back moves
		BoardState* state{ nullptr }; // Points to most current state in stateHistory
		Evaluator* evaluator{ nullptr };
		// Bitboards
		uint64_t allPieces{};
		uint64_t whitePieces{};
		uint64_t blackPieces{};
		uint64_t orthogonalPieces{};
		uint64_t diagonalPieces{};
		uint64_t pawns{};
		uint64_t knights{};

		// Indexes for pieceLists first iterator
		static constexpr int blackIndex{ 0 };
		static constexpr int whiteIndex{ 1 };

		Board();

		void setEvaluator(Evaluator* evaluator);
		
		void loadPosition(const std::string& fen);
		void makeMove(Move move, bool hashBoard = true);
		void unMakeMove(Move move);
		void printBoard();
		std::vector<uint64_t> getBitBoards();
	};

}

#endif // !BOARD_H
