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
	/*
	static const std::vector<Coord> orthogonalDirections;
	static const std::vector<Coord> diagonalDirections;
	static const std::vector<Coord> knightDirections;
	static const std::vector<Coord> blackPawnMoveDirections;
	static const std::vector<Coord> whitePawnMoveDirections;

	static std::vector<Move> kingMoves[64];
	static std::vector<Move> queenMoves[64];
	static std::vector<Move> rookMoves[64];
	static std::vector<Move> bishopMoves[64];
	static std::vector<Move> knightMoves[64];
	static std::vector<Move> blackPawnMoves[64];
	static std::vector<Move> whitePawnMoves[64];
	*/

	MovePrecomputation();
	//void initMoves(std::vector<Coord> directions, std::vector<Move> moves[64], bool scalable = false);
	//void initPawnMoves(std::vector<Coord> directions, std::vector<Move> moves[64], bool isWhite);
};

#endif // !MOVEPRECOMPUTATION_H
