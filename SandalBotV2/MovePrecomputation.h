#pragma once

#ifndef MOVEPRECOMPUTATION_H
#define MOVEPRECOMPUTATION_H

#include "Move.h"
#include "CoordHelper.h"

#include <vector>

class MovePrecomputation {
public:
	struct dirDist {
		// Distances between piece and sides of board
		int top;
		int left;
		int right;
		int bottom;
		int direction[8];
		bool knightSquares[8];
		int minVertical;
		int minHorizontal;
		dirDist();
		dirDist(int top, int left, int right, int bottom);
	};
	dirDist directionDistances[64];


	MovePrecomputation();
	static constexpr void precomputeMoves();
	static constexpr void precomputeOrthogonalMoves();
	static constexpr void precomputeDiagonalMoves();
};

#endif // !MOVEPRECOMPUTATION_H
