#include "MovePrecomputation.h"

#include <iostream>

MovePrecomputation::PrecompCalc::PrecompCalc(int top, int left, int right, int bottom) : top(top), left(left), right(right), bottom(bottom) {
	this->minVertical = std::min(top, bottom);
	this->minHorizontal = std::min(left, right);
}

constexpr MovePrecomputation::MovePrecomputation() {
	initKingMoves();
	initQueenMoves();
	initRookMoves();
	initBishopMoves();
	initKnightMoves();
	initBlackPawnMoves();
	initWhitePawnMoves();
}

constexpr void MovePrecomputation::initKingMoves() {
	for (int square = 0; square < 64; square++) {
		Coord coord = Coord(square);
	}
}

constexpr void MovePrecomputation::initQueenMoves() {
}

constexpr void MovePrecomputation::initRookMoves() {
}

constexpr void MovePrecomputation::initBishopMoves() {
}

constexpr void MovePrecomputation::initKnightMoves() {
}

constexpr void MovePrecomputation::initBlackPawnMoves() {
}

constexpr void MovePrecomputation::initWhitePawnMoves() {
}
