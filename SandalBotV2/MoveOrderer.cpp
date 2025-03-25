#include "MoveOrderer.h"
#include "Searcher.h"

#include <iostream>

using namespace std;

namespace SandalBot {

	MoveOrderer::MoveOrderer(Board* board, MoveGen* gen, Searcher* searcher)
		: board(board), generator(gen), searcher(searcher) {}

	// Assigns each move in decayed c array a heuristic value depending on its effectiveness.
	// Sorts array based on list point values
	void MoveOrderer::order(Move moves[], Move bestMove, int numMoves, int depth, bool firstMove, bool qSearch) {
		// No need to sort one move
		if (numMoves <= 1) return;
		// 218 is maximum number of moves
		int moveVals[218];
		int colorIndex = board->state->whiteTurn ? Board::whiteIndex : Board::blackIndex;
		int dir = board->state->whiteTurn ? 1 : -1;
		int evalStart = Evaluator::colorStart[colorIndex];
		// For each move
		for (int it = 0; it < numMoves; it++) {
			// If not in quiescence search and found move is a previusly found best move, most likely best move
			if (!qSearch && moves[it] == bestMove) {
				moveVals[it] = 1000000;
				continue;
			}
			// If not quienscence search and found move is a killer move, most likely a good move
			if (!qSearch && killerMoves[depth].match(moves[it])) {
				moveVals[it] += killerValue;
			}
			int moveValue = 0;
			const int startSquare = moves[it].getStartSquare();
			const int targetSquare = moves[it].getTargetSquare();
			const int flag = moves[it].getFlag();
			int ownPiece = Piece::type(board->squares[startSquare]);
			int enemyPiece = Piece::type(board->squares[targetSquare]);
			bool targetSquareDefended = generator->opponentAttacks & (1ULL << targetSquare);

			// If taking an opponent's piece
			if (enemyPiece != Piece::empty) {
				int enemyValue = PieceEvaluations::pieceVals[enemyPiece];
				int ownValue = PieceEvaluations::pieceVals[ownPiece];
				int diffVal = enemyValue - ownValue;
				// If taking a piece worth less than our piece's value
				if (enemyValue < ownValue) {
					// If piece is defended
					if (targetSquareDefended) {
						moveValue += diffVal;
					}
					// If piece is undefended
					else {
						moveValue += enemyValue;
					}
				}
				// If we're taking a piece with greater value
				else {
					if (targetSquareDefended) {
						moveValue += diffVal;
					} else {
						moveValue += enemyValue;
					}
				}
			}
			// If we're not taking a piece
			else {
				if (targetSquareDefended) {
					moveValue += -200;
				}
			}

			// Add difference in piece positioning
			if (colorIndex == Board::blackIndex) {
				moveValue += PieceEvaluations::pieceEvals[ownPiece][Evaluator::blackEvalSquare[evalStart - targetSquare]];
				moveValue -= PieceEvaluations::pieceEvals[ownPiece][Evaluator::blackEvalSquare[evalStart - startSquare]];
			} else {
				moveValue += PieceEvaluations::pieceEvals[ownPiece][targetSquare];
				moveValue -= PieceEvaluations::pieceEvals[ownPiece][startSquare];
			}

			// Moves with flags are most likely special (good)
			switch (flag) {
			case Move::noFlag:
				break;
			case Move::enPassantCaptureFlag:
				moveValue += 300;
				break;
			case Move::castleFlag:
				moveValue += 300;
				break;
			case Move::pawnTwoSquaresFlag:
				moveValue += 100;
				break;
			case Move::promoteToQueenFlag:
				moveValue += 600;
				break;
			case Move::promoteToRookFlag:
				moveValue += 400;
				break;
			case Move::promoteToBishopFlag:
				moveValue += 300;
				break;
			case Move::promoteToKnightFlag:
				moveValue += 300;
				break;
			}
			moveVals[it] = moveValue;
		}
		quickSort(moves, moveVals, 0, numMoves); // Sort array
	}

	// Quicksort implementation which sorts moves array based on moveVals point system
	void MoveOrderer::quickSort(Move moves[], int moveVals[], int start, int end) {
		if (start >= end) return;
		int pivotSpot = end - 1;
		int pivot = moveVals[pivotSpot];
		Move& movePivot = moves[pivotSpot];
		int pivotIndex = start;

		for (int i = start; i < end; i++) {
			if (moveVals[i] > pivot) {
				swap(moveVals[i], moveVals[pivotIndex]);
				swap(moves[i], moves[pivotIndex]);

				pivotIndex++;
			}
		}

		swap(moveVals[pivotSpot], moveVals[pivotIndex]);
		swap(moves[pivotSpot], moves[pivotIndex]);

		quickSort(moves, moveVals, start, pivotIndex);
		quickSort(moves, moveVals, pivotIndex + 1, end);
	}
	// Add move to killer moves
	void MoveOrderer::addKiller(int depth, Move move) {
		if (depth >= 32)
			return;
		killerMoves[depth].add(move);
	}

}