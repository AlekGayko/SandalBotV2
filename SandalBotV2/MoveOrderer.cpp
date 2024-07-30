#include "MoveOrderer.h"

#include <iostream>

using namespace std;

MoveOrderer::MoveOrderer() {
}

MoveOrderer::MoveOrderer(Board* board, MoveGen* gen) {
	this->board = board;
	this->generator = gen;
}

void MoveOrderer::order(Move moves[], int numMoves) {
	if (numMoves <= 1) return;
	int moveVals[218];
	for (int it = 0; it < numMoves; it++) {
		int moveValue = 0;
		const int startSquare = moves[it].startSquare;
		const int targetSquare = moves[it].targetSquare;
		const int flag = moves[it].flag;
		int ownPiece = Piece::type(board->squares[startSquare]);
		int enemyPiece = Piece::type(board->squares[targetSquare]);
		int enemyValue = PieceEvaluations::pieceVals[enemyPiece];
		int ownValue = PieceEvaluations::pieceVals[ownPiece];
		int diffVal = enemyValue - ownValue;
		if (enemyValue) {
			moveValue += 100;
		}
		if (diffVal > 0) {
			moveValue += diffVal;
		}
		if (generator->opponentAttacks & (1ULL << targetSquare)) {
			moveValue -= 200;
		}
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
}

void MoveOrderer::quickSort(Move moves[], int moveVals[], int start, int end) {
	if (start >= end) return;
	int pivotSpot = end - 1;
	int pivot = moveVals[pivotSpot];
	int pivotIndex = start;

	for (int i = start; i < end; i++) {
		if (moveVals[i] > pivot) {
			int temp = moveVals[i];
			moveVals[i] = moveVals[pivotIndex];
			moveVals[pivotIndex] = temp;

			pivotIndex++;
		}
	}
	moveVals[pivotSpot] = moveVals[pivotIndex];
	moveVals[pivotIndex] = pivot;

	quickSort(moves, moveVals, start, pivotIndex);
	quickSort(moves, moveVals, pivotIndex + 1, end);
}
