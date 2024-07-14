#include "MovePrecomputation.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

const std::vector<Coord> MovePrecomputation::orthogonalDirections = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
const std::vector<Coord> MovePrecomputation::diagonalDirections = { { 1, 1 }, { -1, 1 }, { 1, -1 }, { -1, -1 } };
const std::vector<Coord> MovePrecomputation::knightDirections = { { 2, 1 }, { 2, -1 }, { 1, 2 }, { 1, -2 }, { -1, 2 }, { -1, -2 }, { -2, 1 }, { -2, -1 } };
const std::vector<Coord> MovePrecomputation::blackPawnMoveDirections = { { 1, 0 } };
const std::vector<Coord> MovePrecomputation::whitePawnMoveDirections = { { -1, 0 } };
MovePrecomputation::MovePrecomputation() {
}
/*
MovePrecomputation::PrecompCalc::PrecompCalc(int top, int left, int right, int bottom) : top(top), left(left), right(right), bottom(bottom) {
	this->minVertical = std::min(top, bottom);
	this->minHorizontal = std::min(left, right);
}

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
