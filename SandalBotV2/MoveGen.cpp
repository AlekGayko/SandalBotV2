#include "MoveGen.h"

#include <iostream>
#include <stdexcept>

using namespace std;

MoveGen::MoveGen() {

}

MoveGen::MoveGen(Board* board) {
	if (board == nullptr) throw std::invalid_argument("board cannot be nullptr");
	this->board = board;
}

int MoveGen::generateMoves(Move moves[], bool isWhite) {
	const int color = isWhite ? board->state.whiteTurn : board->blackIndex;
	currentColor = board->state.whiteTurn ? Piece::white : Piece::black;
	opposingColor = !board->state.whiteTurn ? Piece::white : Piece::black;
	currentMoves = 0;

	for (int square = 0; square < 64; square++) {
		//int numPieces = board->pieceLists[piece][color].numPieces;
		if (!Piece::isColor(board->squares[square], currentColor)) continue;
		const int startSquare = square; //board->pieceLists[piece][color][j];
		const int piece = Piece::type(board->squares[square]);
		switch (piece) {
		case Piece::pawn:
			generatePawnMoves(moves, startSquare);
			continue;
		case Piece::knight: 
			generateKnightMoves(moves, startSquare);
			continue;
		case Piece::king:
			generateKingMoves(moves, startSquare);
			continue;
		}

		if (Piece::isOrthogonal(piece)) {
			generateOrthogonalMoves(moves, startSquare);
		}
		if (Piece::isDiagonal(piece)) {
			generateDiagonalMoves(moves, startSquare);
		}
	}
	return currentMoves;
}

void MoveGen::generateOrthogonalMoves(Move moves[], int startSquare) {
	for (Coord direction : orthogonalDirections) {
		for (int scalar = 1; true; scalar++) {
			int targetSquare = direction * scalar + startSquare;
			// If coordinate out of bounds break
			if (!CoordHelper::validCoordAddition(startSquare, direction, scalar)) break;
			// If targetsquare contains friendly piece break
			if (Piece::isColor(board->squares[targetSquare], currentColor)) break;
			// Add move
			moves[currentMoves] = Move(startSquare, targetSquare);
			currentMoves++;
			// If targetsquare contains opposing piece break
			if (Piece::isColor(board->squares[targetSquare], opposingColor)) {
				moves[currentMoves - 1].takenPiece = board->squares[targetSquare];
				break;
			}
		}
	}
}

void MoveGen::generateDiagonalMoves(Move moves[], int startSquare) {
	for (Coord direction : diagonalDirections) {
		for (int scalar = 1; true; scalar++) {
			int targetSquare = direction * scalar + startSquare;
			if (!CoordHelper::validCoordAddition(startSquare, direction, scalar)) break;
			if (Piece::isColor(board->squares[targetSquare], currentColor)) break;
			moves[currentMoves] = Move(startSquare, targetSquare);
			currentMoves++;
			if (Piece::isColor(board->squares[targetSquare], opposingColor)) {
				moves[currentMoves - 1].takenPiece = board->squares[targetSquare];
				break;
			}
		}
	}
}

void MoveGen::generateKnightMoves(Move moves[], int startSquare) {
	for (Coord direction : knightDirections) {
		int targetSquare = direction + startSquare;
		if (!CoordHelper::validCoordAddition(startSquare, direction)) continue;
		if (Piece::isColor(board->squares[targetSquare], currentColor)) continue;

		moves[currentMoves] = Move(startSquare, targetSquare);
		currentMoves++;
		if (Piece::isColor(board->squares[targetSquare], opposingColor)) moves[currentMoves - 1].takenPiece = board->squares[targetSquare];
	}
}

void MoveGen::generateKingMoves(Move moves[], int startSquare) {
	for (Coord direction : diagonalDirections) {
		int targetSquare = direction + startSquare;
		if (!CoordHelper::validCoordAddition(startSquare, direction)) continue;
		if (Piece::isColor(board->squares[targetSquare], currentColor)) continue;
		moves[currentMoves] = Move(startSquare, targetSquare);
		currentMoves++;
		if (Piece::isColor(board->squares[targetSquare], opposingColor)) moves[currentMoves - 1].takenPiece = board->squares[targetSquare];
	}

	for (Coord direction : orthogonalDirections) {
		int targetSquare = direction + startSquare;
		// If coordinate out of bounds break
		if (!CoordHelper::validCoordAddition(startSquare, direction)) continue;
		// If targetsquare contains friendly piece break
		if (Piece::isColor(board->squares[targetSquare], currentColor)) continue;
		// Add move
		moves[currentMoves] = Move(startSquare, targetSquare);
		currentMoves++;
		// If targetsquare contains opposing piece break
		if (Piece::isColor(board->squares[targetSquare], opposingColor)) moves[currentMoves - 1].takenPiece = board->squares[targetSquare];
	}
}

void MoveGen::generatePawnMoves(Move moves[], int startSquare) {
	const Coord direction = board->state.whiteTurn ? whitePawnDirection : blackPawnDirection;
	const Coord* attackDirection = board->state.whiteTurn ? whitePawnAttacks : blackPawnAttacks;
	const int startRow = board->state.whiteTurn ? 6 : 1;
	for (int i = 0; i < 2; i++) {
		int targetSquare = attackDirection[i] + startSquare;
		if (!CoordHelper::validCoordAddition(startSquare, attackDirection[i])) continue;
		if (!Piece::isColor(board->squares[targetSquare], opposingColor)) continue;
		moves[currentMoves] = Move(startSquare, targetSquare);
		moves[currentMoves].takenPiece = board->squares[targetSquare];
		currentMoves++;
	}

	if (board->squares[direction + startSquare] == Piece::empty) {
		moves[currentMoves] = Move(startSquare, direction + startSquare);
		currentMoves++;
	} else return;
	if (startSquare / 8 == startRow && board->squares[direction * 2 + startSquare] == Piece::empty) {
		moves[currentMoves] = Move(startSquare, direction * 2 + startSquare);
		currentMoves++;
	}

	return;
}
