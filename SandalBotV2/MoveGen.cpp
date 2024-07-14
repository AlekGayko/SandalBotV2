#include "MoveGen.h"

MoveGen::MoveGen(Board* board) {
	this->board = board;
}

void MoveGen::generateMoves(Move moves[]) {
	const int color = board->state.whiteTurn ? board->whiteIndex : board->blackIndex;
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
}

void MoveGen::generateOrthogonalMoves(Move moves[], int startSquare) {
	for (int direction = minOrthogonal; direction < maxOrthogonal; direction++) {
		for (int targetSquare = startSquare; targetSquare < 64 && targetSquare >= 0; targetSquare += direction) {
			if (Piece::isColor(board->squares[targetSquare], currentColor)) break;
			moves[currentMoves] = Move(startSquare, targetSquare);
			currentMoves++;
			if (board->squares[targetSquare] != Piece::empty) break;
		}
	}
}

void MoveGen::generateDiagonalMoves(Move moves[], int startSquare) {
	for (int direction = minDiagonal; direction < maxDiagonal; direction++) {
		for (int targetSquare = startSquare; targetSquare < 64 && targetSquare >= 0; targetSquare += direction) {
			if (Piece::isColor(board->squares[targetSquare], currentColor)) break;
			moves[currentMoves] = Move(startSquare, targetSquare);
			currentMoves++;
			if (board->squares[targetSquare] != Piece::empty) break;
		}
	}
}

void MoveGen::generateKnightMoves(Move moves[], int startSquare) {
}

void MoveGen::generateKingMoves(Move moves[], int startSquare) {
	for (int direction = minOrthogonal; direction < maxDiagonal; direction++) {
		int targetSquare = startSquare + direction;
		if (Piece::isColor(board->squares[targetSquare], currentColor)) break;
		moves[currentMoves] = Move(startSquare, targetSquare);
		currentMoves++;
	}
}

void MoveGen::generatePawnMoves(Move moves[], int startSquare) {
}
