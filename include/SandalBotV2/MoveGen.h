#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "Bitboards.h"
#include "Board.h"
#include "CoordHelper.h"
#include "MoveOrderer.h"
#include "Types.h"

#include <cassert>

namespace SandalBot {

	using PointValue = int16_t;

	// Struct holds a move and corresponding heuristic value
	struct MovePoint {
		PointValue value{ 0 };
		Move move{};
	};

	enum Stage : int {
		MAIN_TT, MAIN_CAPTURE_INIT, MAIN_CAPTURES, KILLERS, MAIN_QUIET_INIT, MAIN_QUIETS, MAIN_BAD_CAPTURES,
		Q_TT, Q_CAPTURES_INIT, Q_CAPTURES, Q_CHECKS_INIT, Q_CHECKS, Q_BAD_CAPTURES,
		EVASIONS_TT, EVASIONS_INIT, EVASION_MOVES,
		VANILLA_INIT, VANILLA
	};

	// MoveGen class generates all possible legalmoves in a given position
	class MoveGen {
		enum GenType {
			CAPTURES,
			QUIETS,
			EVASIONS,
			QUIET_CHECKS,
			ALL
		};
	public:
		MoveGen() = delete;

		~MoveGen() = default;

		// Main Search
		MoveGen(Board* board, Killer* killerMoves, Move ttMove, int depth) : board(board), ttMove(ttMove), depth(depth) {
			assert(board != nullptr);
			assert(killerMoves != nullptr);

			this->killerMoves[0] = killerMoves[depth].moveA;
			this->killerMoves[1] = killerMoves[depth].moveB;

			stage = board->checkBB() ? EVASIONS_TT : MAIN_TT;
		}
		// Q Search
		MoveGen(Board* board, Move ttMove, int depth) : board(board), ttMove(ttMove), depth(depth) {
			assert(board != nullptr);
			stage = board->checkBB() ? EVASIONS_TT : Q_TT;
		}

		// Generic Move Generation
		MoveGen(Board* board) : board(board) {
			assert(board != nullptr);
			ttMove = Move();
			stage = board->checkBB() ? EVASIONS_INIT : VANILLA_INIT;
		}

		MovePoint* begin() { return curr; }
		MovePoint* end() { return endMoves; }

		Move getMove();

	private:
		Board* board = nullptr;
		MovePoint moves[maxMoves];
		Move killerMoves[2];
		Move* currKiller{ killerMoves }, *endKiller{ killerMoves + 2 };
		Move ttMove;
		MovePoint* curr{ moves }, * endMoves{ moves }, * badMoves{ moves };

		int stage;
		int depth;

		Move selectMove();

		template <GenType Type>
		void generate();
		template <GenType Type, Color Us>
		void generateAllMoves();
		template <Color Us, PieceType Type, bool QuietChecking>
		void generateMoves(Bitboard target);
		template <GenType Type, Color Us>
		void generatePawnMoves(Bitboard target);
		template <GenType Type, Color Us, bool QuietChecking>
		void generateKingMoves(Bitboard target);

		void promotionMoves(Square from, Square to);

		template <GenType Type>
		void evaluateMoves();

		void addMove(Square from, Square to, Move::Flag flag = Move::Flag::NO_FLAG) {
			Move move = Move(from, to, flag);

			if (move == ttMove) {
				return;
			}
			(endMoves++)->move = std::move(move);
		}
	};

}

#endif
