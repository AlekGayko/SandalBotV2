#ifndef MOVEORDERER_H
#define MOVEORDERER_H

#include "Board.h"
#include "PieceEvaluations.h"
#include "MoveGen.h"

namespace SandalBot {

	class Searcher;

	// MoveOrderer heuristically orders and array of moves from best to worst.
	// Uses a scoring system to order moves, and also employs 'killer' moves,
	// which are previously found moves which have been historically good
	class MoveOrderer {
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

		Board* board{ nullptr };
		MoveGen* generator{ nullptr };
		Searcher* searcher{ nullptr };

		const int killerValue{ 1000 }; // Heuristic value of a killer move
		Killer killerMoves[32]; // Array of killer moves where index is depth of killer move
	public:
		MoveOrderer() {};
		MoveOrderer(Board* board, MoveGen* gen, Searcher* searcher);
		void order(Move moves[], Move bestMove, int numMoves, int depth, bool firstMove = false, bool qSearch = false);
		static void quickSort(Move moves[], int moveVals[], int start, int end);
		void addKiller(int depth, Move move);
	};

}

#endif