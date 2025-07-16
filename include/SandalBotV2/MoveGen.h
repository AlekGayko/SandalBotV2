#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "Bitboards.h"
#include "Board.h"
#include "CoordHelper.h"
#include "MoveOrderer.h"
#include "Types.h"

namespace SandalBot {

	struct MovePoint;

	// MoveGen class generates all possible legalmoves in a given position
	class MoveGen {
		friend class MoveOrderer;
		friend class Searcher;
	public:
		static constexpr int maxMoves{ 218 };

		bool isCheck{}; // Tracks whether there is check in position

		MoveGen() = default;

		~MoveGen() = default;

		MoveGen(Board* board) : board(board) {
			if (board == nullptr) 
				throw std::invalid_argument("board cannot be nullptr");
		}

		int generate(MovePoint moves[], bool capturesOnly = false);

	private:
		Board* board = nullptr;

		uint64_t currentMoves{};
		bool doubleCheck{};
		bool generateCaptures{};

		// Utility bitboards
		Bitboard opponentAttacks{};
		Bitboard checkBB{};
		Bitboard checkRayBB{};

		void initVariables();

		template <Color Us>
		int generateAllMoves(MovePoint moves[], bool capturesOnly = false);
		template <Color Us, PieceType Type>
		void generateMoves(MovePoint moves[], bool capturesOnly);
		template <Color Us>
		void generatePawnMoves(MovePoint moves[], bool capturesOnly);
		template <Color Us>
		void generateKingMoves(MovePoint moves[], bool capturesOnly);

		template <Color Us>
		void enPassantMoves(MovePoint moves[], Square from, Square to, bool isPinned);
		template <Color Us>
		void promotionMoves(MovePoint moves[], Square from, Bitboard movementBB);
		template <Color Us>
		bool enPassantPin(Square friendlyPawnSquare, Square enemyPawnSquare);

		template <Color Us>
		void castlingMoves(MovePoint moves[], Square from);

		template <Color Us>
		Bitboard generatePawnAttackData();
		template <Color Us, PieceType Type>
		Bitboard generateAttackData();
		template <Color Us>
		void generateAllAttackData();
		template <Color Us>
		void generateCheckData();

		void addMove(MovePoint moves[], Square from, Square to, Move::Flag flag = Move::Flag::NO_FLAG) {
			moves[currentMoves++].move = std::move(Move(from, to, flag));
		}
	};

}

#endif
