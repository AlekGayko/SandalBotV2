#include "MoveGen.h"

MoveGen::MoveGen(Board board) {
	//this->board = board;
}

void MoveGen::generateMoves(int moves[]) {
	const int color = board.state.whiteTurn ? board.whiteIndex : board.blackIndex;

	for (int i = 0; i < 6; i++) {
		int numPieces = board.pieceLists[i][color].numPieces;
		int piece = i | color; // pieces 0-6 so works out
		for (int j = 0; j < board.pieceLists[i][color].numPieces; j++) {
			const int startSquare = board.pieceLists[i][color][j];

			if (Piece::isType(piece, Piece::pawn)) {
				generatePawnMoves(moves, startSquare);
				continue;
			} else if (Piece::isType(piece, Piece::knight)) {
				generateKnightMoves(moves, startSquare);
				continue;
			} else if (Piece::isType(piece, Piece::king)) {
				generateKingMoves(moves, startSquare);
				continue;
			}

			if (Piece::isOrthogonal(piece)) {
				generateOrthogonalMoves(moves, startSquare, i);
			}
			if (Piece::isDiagonal(piece)) {
				generateDiagonalMoves(moves, startSquare, i);
			}
			
		}
	}
}

void MoveGen::generateOrthogonalMoves(int moves[], int startSquare, int pieceType) {
}

void MoveGen::generateDiagonalMoves(int moves[], int startSquare, int pieceType) {
}

void MoveGen::generateKnightMoves(int moves[], int startSquare) {
}

void MoveGen::generateKingMoves(int moves[], int startSquare) {
}

void MoveGen::generatePawnMoves(int moves[], int startSquare) {
}
