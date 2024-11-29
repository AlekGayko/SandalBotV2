#include "MovePrecomputation.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

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

constexpr void MovePrecomputation::precomputeMoves() {
	precomputeDiagonalMoves();
	precomputeOrthogonalMoves();

}

constexpr void MovePrecomputation::precomputeOrthogonalMoves() {

}

constexpr void MovePrecomputation::precomputeDiagonalMoves() {

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