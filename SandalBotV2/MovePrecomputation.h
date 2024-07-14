#pragma once

#ifndef MOVEPRECOMPUTATION_H
#define MOVEPRECOMPUTATION_H

#include "Move.h"
#include "CoordHelper.h"

#include <vector>

class MovePrecomputation {
public:
	struct PrecompCalc {
		// Distances between piece and sides of board
		int top;
		int left;
		int right;
		int bottom;
		int minVertical;
		int minHorizontal;
		constexpr PrecompCalc(int top, int left, int right, int bottom);
	};

	std::vector<Move> kingMoves[64];
	std::vector<Move> queenMoves[64];
	std::vector<Move> rookMoves[64];
	std::vector<Move> bishopMoves[64];
	std::vector<Move> knightMoves[64];
	std::vector<Move> blackPawnMoves[64];
	std::vector<Move> whitePawnMoves[64];

	constexpr MovePrecomputation();
	constexpr void initKingMoves();
	constexpr void initQueenMoves();
	constexpr void initRookMoves();
	constexpr void initBishopMoves();
	constexpr void initKnightMoves();
	constexpr void initBlackPawnMoves();
	constexpr void initWhitePawnMoves();



};

#endif // !MOVEPRECOMPUTATION_H
