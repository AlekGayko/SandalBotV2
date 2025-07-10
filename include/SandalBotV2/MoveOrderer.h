#ifndef MOVEORDERER_H
#define MOVEORDERER_H

#include "Board.h"
#include "PieceEvaluations.h"
#include "MoveGen.h"

#include <limits>

namespace SandalBot {

	using PointValue = int16_t;

	// Struct holds a move and corresponding heuristic value
	struct MovePoint {
		PointValue value{};
		Move move{};
	};

	// MoveOrderer heuristically orders and array of moves from best to worst.
	// Uses a scoring system to order moves, and also employs 'killer' moves,
	// which are previously found moves which have been historically good
	class MoveOrderer {
	public:
		MoveOrderer() {};
		MoveOrderer(MoveGen* gen);
		void order(Board* board, MovePoint moves[], Move bestMove, int numMoves, int depth, bool qSearch = false);
		void addKiller(int depth, Move move);

		static void quickSort(MovePoint moves[], int start, int end);
		static void bubbleSort(MovePoint moves[], int numMoves);
		static void insertionSort(MovePoint moves[], int numMoves);
		static void selectionSort(MovePoint moves[], int numMoves);
		static void mergeSort(MovePoint moves[], int start, int end);
	private:
		struct Killer {
			Move moveA{};
			Move moveB{};
			// Adds new killer move to struct. Demotes moveA to moveB and removes moveB
			void add(Move move) {
				if (move.moveValue != moveA.moveValue) {
					moveB = moveA;
					moveA = move;
				}
			}
			// Sees if given move is a killer move
			bool match(Move move) const {
				return move.moveValue == moveA.moveValue || move.moveValue == moveB.moveValue;
			}
		};

		static constexpr PointValue bestMoveValue{ std::numeric_limits<PointValue>::max() };
		static constexpr PointValue killerValue{ 10000 };
		static constexpr PointValue undefendedTargetSquareValue{ -200 };
		static constexpr PointValue enPassantValue{ 300 };
		static constexpr PointValue castleValue{ 300 };
		static constexpr PointValue pawnTwoSquareValue{ 100 };
		static constexpr PointValue queenPromotionValue{ 600 };
		static constexpr PointValue rookPromotionValue{ 400 };
		static constexpr PointValue bishopPromotionValue{ 300 };
		static constexpr PointValue knightPromotionValue{ 300 };

		Board* board{ nullptr };
		MoveGen* generator{ nullptr };

		Killer killerMoves[32]; // Array of killer moves where index is depth of killer move
	};

}

#endif