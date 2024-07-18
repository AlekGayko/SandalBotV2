#include "MovePrecomputation.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

/*
const std::vector<Coord> MovePrecomputation::orthogonalDirections = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
const std::vector<Coord> MovePrecomputation::diagonalDirections = { { 1, 1 }, { -1, 1 }, { 1, -1 }, { -1, -1 } };
const std::vector<Coord> MovePrecomputation::knightDirections = { { 2, 1 }, { 2, -1 }, { 1, 2 }, { 1, -2 }, { -1, 2 }, { -1, -2 }, { -2, 1 }, { -2, -1 } };
const std::vector<Coord> MovePrecomputation::blackPawnMoveDirections = { { 1, 0 } };
const std::vector<Coord> MovePrecomputation::whitePawnMoveDirections = { { -1, 0 } };
*/

MovePrecomputation::MovePrecomputation() {
	for (int square = 0; square < 64; square++) {
		int row, col, top, left, right, bottom;
		row = square / 8;
		col = square % 8;
		top = row + 1;
		bottom = 8 - row;
		left = col + 1;
		right = 8 - col;
		directionDistances[square] = dirDist(top, left, right, bottom);
	}
}

MovePrecomputation::dirDist::dirDist() {
}

MovePrecomputation::dirDist::dirDist(int top, int left, int right, int bottom) : top(top), left(left), right(right), bottom(bottom) {
	direction[0] = top;
	direction[1] = right;
	direction[2] = bottom;
	direction[3] = left;
	direction[4] = std::min(left, top);
	direction[5] = std::min(right, top);
	direction[6] = std::min(right, bottom);
	direction[7] = std::min(left, bottom);

	knightSquares[0] = left > 1 && top > 2;
	knightSquares[1] = right > 1 && top > 2;
	knightSquares[2] = right > 2 && top > 1;
	knightSquares[3] = right > 2 && bottom > 1;
	knightSquares[4] = right > 1 && bottom > 2;
	knightSquares[5] = left > 1 && bottom > 2;
	knightSquares[6] = left > 2 && bottom > 1;
	knightSquares[7] = left > 2 && top > 1;

	this->minVertical = std::min(top, bottom);
	this->minHorizontal = std::min(left, right);
}
/*
MovePrecomputation::MovePrecomputation() {
	initMoves(orthogonalDirections, rookMoves, true);
	initMoves(diagonalDirections, bishopMoves, true);
	initMoves(orthogonalDirections, kingMoves);
	initMoves(diagonalDirections, kingMoves);
	initMoves(orthogonalDirections, queenMoves, true);
	initMoves(diagonalDirections, queenMoves, true);
	initMoves(knightDirections, knightMoves);
	initPawnMoves(blackPawnMoveDirections, blackPawnMoves, false);
	initPawnMoves(whitePawnMoveDirections, whitePawnMoves, true);
}
*/
/*
void MovePrecomputation::initMoves(std::vector<Coord> directions, std::vector<Move> moves[64], bool scalable) {
	for (int square = 0; square < 64; square++) {
		Coord startSquare = Coord(square);
		for (Coord direction : directions) {
			Coord scalar = direction;
			do {
				if (CoordHelper::validCoordAddition(startSquare, direction)) {
					Coord targetSquare = startSquare + direction;
					Move move = Move(square, CoordHelper::coordToIndex(startSquare + direction));
					moves[square].push_back(move);
				} else break;
				direction = direction + scalar;
			} while (scalable);
		}
	}
}

void MovePrecomputation::initPawnMoves(std::vector<Coord> directions, std::vector<Move> moves[64], bool isWhite) {
	initMoves(directions, moves);
	Coord startSquare = isWhite ? Coord(6, 0) : Coord(1, 0);
	for (int i = 0; i < 8; i++) {
		Coord startDirection = { isWhite ? -2 : 2, 0 };
		Move move = Move(CoordHelper::coordToIndex(startSquare), CoordHelper::coordToIndex(startSquare + startDirection));
		moves[CoordHelper::coordToIndex(startSquare)].push_back(move);
	}
}
*/