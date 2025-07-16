#include "MoveOrderer.h"

#include "MoveGen.h"
#include "Searcher.h"

#include <algorithm>
#include <iostream>

using namespace std;

namespace SandalBot {

	// Assigns each move in decayed c array a heuristic value depending on its effectiveness.
	// Sorts array based on list point values
	void MoveOrderer::order(Board* board, MoveGen* generator, MovePoint moves[], Move bestMove, int numMoves, int depth, bool qSearch) {
		// No need to sort one move
		if (numMoves <= 1) return;

		// For each move
		for (int it = 0; it < numMoves; ++it) {
			// If not in quiescence search and found move is a previusly found best move, most likely best move
			if (!qSearch && moves[it].move == bestMove) {
				moves[it].value = bestMoveValue;
				continue;
			}
			// If not quienscence search and found move is a killer move, most likely a good move
			if (!qSearch && killerMoves[depth].match(moves[it].move)) {
				moves[it].value = killerValue;
				continue;
			}

			PointValue moveValue = 0;
			const Square from = moves[it].move.from();
			const Square to = moves[it].move.to();
			const Move::Flag flag = moves[it].move.flag();
			PieceType ownPiece = typeOf(board->squares[from]);
			PieceType enemyPiece = typeOf(board->squares[to]);
			bool toDefended = generator->opponentAttacks & (1ULL << to);

			// If taking an opponent's piece
			if (enemyPiece != NO_PIECE) {
				PointValue enemyValue = PieceEvaluations::pieceVals[enemyPiece];
				PointValue ownValue = PieceEvaluations::pieceVals[ownPiece];
				PointValue diffVal = enemyValue - ownValue;
				// If taking a piece worth less than our piece's value
				if (enemyValue < ownValue) {
					// If piece is defended
					if (toDefended) {
						moveValue += diffVal;
					}
					// If piece is undefended
					else {
						moveValue += enemyValue;
					}
				}
				// If we're taking a piece with greater value
				else {
					if (toDefended) {
						moveValue += diffVal;
					} else {
						moveValue += enemyValue;
					}
				}
			}
			// If we're not taking a piece
			else {
				if (toDefended) {
					moveValue += undefendedtoValue;
				}
			}

			// Add difference in piece positioning
			if (board->sideToMove() == BLACK) {
				moveValue += PieceEvaluations::pieceEvals[ownPiece][flipRow(to)];
				moveValue -= PieceEvaluations::pieceEvals[ownPiece][flipRow(from)];
			} else {
				moveValue += PieceEvaluations::pieceEvals[ownPiece][to];
				moveValue -= PieceEvaluations::pieceEvals[ownPiece][from];
			}

			// Moves with flags are most likely special (good)
			switch (flag) {
			case Move::Flag::NO_FLAG:
				break;
			case Move::Flag::EN_PASSANT:
				moveValue += enPassantValue;
				break;
			case Move::Flag::CASTLE:
				moveValue += castleValue;
				break;
			case Move::Flag::PAWN_TWO_SQUARES:
				moveValue += pawnTwoSquareValue;
				break;
			case Move::Flag::QUEEN:
				moveValue += queenPromotionValue;
				break;
			case Move::Flag::ROOK:
				moveValue += rookPromotionValue;
				break;
			case Move::Flag::BISHOP:
				moveValue += bishopPromotionValue;
				break;
			case Move::Flag::KNIGHT:
				moveValue += knightPromotionValue;
				break;
			}

			moves[it].value = moveValue;
		}

		std::sort(moves, moves + numMoves, [](MovePoint& mp1, MovePoint& mp2) { return mp1.value > mp2.value; });
	}

	// Quicksort implementation which sorts moves array based on moveVals point system
	void MoveOrderer::quickSort(MovePoint moves[], int start, int end) {
		if (start >= end) return;

		int pivotSpot{ end - 1 };
		int pivotValue = moves[pivotSpot].value;
		int pivotIndex{ start };

		for (int i{ start }; i < end; ++i) {
			if (i == pivotSpot)
				continue;

			if (moves[i].value > pivotValue) {
				std::swap(moves[i], moves[pivotIndex]);

				pivotIndex++;
			}
		}

		std::swap(moves[pivotSpot], moves[pivotIndex]);

		quickSort(moves, start, pivotIndex);
		quickSort(moves, pivotIndex + 1, end);
	}

	void MoveOrderer::bubbleSort(MovePoint moves[], int numMoves) {
		if (numMoves <= 1) return;

		bool swapped{};
		for (int i{ 0 }; i < numMoves - 1; ++i) {
			swapped = false;
			for (int j{ 0 }; j < numMoves - i - 1; ++j) {
				if (moves[j].value < moves[j + 1].value) {
					std::swap(moves[j], moves[j + 1]);
					swapped = true;
				}
			}

			if (!swapped) 
				break;
		}
	}

	void MoveOrderer::insertionSort(MovePoint moves[], int numMoves) {
		if (numMoves <= 1) return;

		for (int i{ 1 }; i < numMoves; ++i) {
			for (int j{ i }; j > 0; --j) {
				if (moves[j].value > moves[j - 1].value) {
					std::swap(moves[j], moves[j - 1]);
				} else {
					break;
				}
			}
		}
	}

	void MoveOrderer::selectionSort(MovePoint moves[], int numMoves) {
		if (numMoves <= 1) return;

		int maxIndex{};

		for (int i{ 0 }; i < numMoves - 1; ++i) {
			maxIndex = i;
			for (int j{ i + 1 }; j < numMoves; ++j) {
				if (moves[j].value > moves[maxIndex].value) {
					maxIndex = j;
				}
			}
			if (i != maxIndex)
				std::swap(moves[i], moves[maxIndex]);
		}
	}

	void MoveOrderer::mergeSort(MovePoint moves[], int start, int end) {
		if (start >= end) return;

		int mid = start + ((end - start) / 2);

		mergeSort(moves, start, mid);
		mergeSort(moves, mid, end);

		int l{ 0 };
		int r{ 0 };
		int idx{ start };
		while (l < (mid - start) && r < (end - mid)) {
			if (moves[l].value > moves[r].value) {
				std::swap(moves[idx], moves[start + l]);
				++l;
			} else {
				std::swap(moves[idx], moves[mid + r]);
				++r;
			}
			++idx;
		}

		while (l < (mid - start)) {
			moves[idx++] = moves[start + l];
			++l;
		}

		while (r < (end - mid)) {
			moves[idx++] = moves[mid + r];
			++r;
		}
	}

	// Add move to killer moves
	void MoveOrderer::addKiller(int depth, Move move) {
		if (depth >= 32)
			return;
		killerMoves[depth].add(move);
	}

}