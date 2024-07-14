#include "MoveGen.h"

#include <stdexcept>

MoveGen::MoveGen() {

}

MoveGen::MoveGen(Board* board) {
	if (board == nullptr) throw std::invalid_argument("board cannot be nullptr");
	this->board = board;
}

int MoveGen::generateMoves(Move moves[], bool isWhite) {
	const int color = isWhite ? board->whiteIndex : board->blackIndex;
	currentColor = color == board->whiteIndex ? Piece::white : Piece::black;
	currentMoves = 0;

	for (int piece = 0; piece < 6; piece++) {
		int numPieces = board->pieceLists[piece][color].numPieces;
		int pieceCol = piece | color; // pieces 0-6 so works out
		for (int j = 0; j < numPieces; j++) {
			const int startSquare = board->pieceLists[piece][color][j];

			switch (piece) {
			case Piece::pawn:
				generatePawnMoves(moves, startSquare);
				continue;
			case Piece::knight: 
				generatePawnMoves(moves, startSquare);
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
	}
	return currentMoves;
}

void MoveGen::generateOrthogonalMoves(Move moves[], int startSquare) {
	for (int direction = minOrthogonal; direction < maxOrthogonal; direction++) {
		for (int targetSquare = startSquare; true; targetSquare += direction) {
			// If coordinate out of bounds break
			if (!CoordHelper::validCoordAddition(startSquare, targetSquare)) break;
			// If targetsquare contains friendly piece break
			if (Piece::isColor(board->squares[targetSquare], currentColor)) break;
			// Add move
			moves[currentMoves] = Move(startSquare, targetSquare);
			currentMoves++;
			// If targetsquare contains opposing piece break
			if (board->squares[targetSquare] != Piece::empty) break;
		}
	}
}

void MoveGen::generateDiagonalMoves(Move moves[], int startSquare) {
	for (int direction = minDiagonal; direction < maxDiagonal; direction++) {
		for (int targetSquare = startSquare; true; targetSquare += direction) {
			if (!CoordHelper::validCoordAddition(startSquare, targetSquare)) break;
			if (Piece::isColor(board->squares[targetSquare], currentColor)) break;
			moves[currentMoves] = Move(startSquare, targetSquare);
			currentMoves++;
			if (board->squares[targetSquare] != Piece::empty) break;
		}
	}
}

void MoveGen::generateKnightMoves(Move moves[], int startSquare) {
	for (int direction = 0; direction < 8; direction++) {
		int targetSquare = startSquare + direction;
		if (!CoordHelper::validCoordAddition(startSquare, targetSquare)) continue;
		if (Piece::isColor(board->squares[targetSquare], currentColor)) continue;
		moves[currentMoves] = Move(startSquare, targetSquare);
		currentMoves++;
	}
}

void MoveGen::generateKingMoves(Move moves[], int startSquare) {
	for (int direction = minOrthogonal; direction < maxDiagonal; direction++) {
		int targetSquare = startSquare + direction;
		if (!CoordHelper::validCoordAddition(startSquare, targetSquare)) continue;
		if (Piece::isColor(board->squares[targetSquare], currentColor)) continue;
		moves[currentMoves] = Move(startSquare, targetSquare);
		currentMoves++;
	}
}

void MoveGen::generatePawnMoves(Move moves[], int startSquare) {
	const int direction = board->state.whiteTurn ? -1 : 1;
	const int startRow = board->state.whiteTurn ? 6 : 1;
	if (board->squares[startSquare + direction] == Piece::empty) {
		moves[currentMoves] = Move(startSquare, startSquare + direction);
		currentMoves++;
	} else return;
	if (startSquare / 8 == startRow && board->squares[startSquare + 2 * direction] == Piece::empty) {
		moves[currentMoves] = Move(startSquare, startSquare + 2 * direction);
		currentMoves++;
	}
	return;
}
