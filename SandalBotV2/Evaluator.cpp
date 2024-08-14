#include "Evaluator.h"

#include <iostream>

using namespace std;

int Evaluator::colorStart[2] = { 63, 0 };
int Evaluator::blackEvalSquare[64];

Evaluator::Evaluator() {
	initBlackSquares();
}

int Evaluator::Evaluate(Board* board) {
	endGameWeight = 0;
	int evaluation;
	int whiteEval = 0;
	int blackEval = 0;
	maximisingSide = board->state->whiteTurn ? 1 : -1;

	for (int colorIndex = Board::blackIndex; colorIndex <= Board::whiteIndex; colorIndex++) {
		for (int piece = Piece::pawn; piece <= Piece::king; piece++) {
			int numPieces = board->pieceLists[colorIndex][piece].numPieces;
			int pieceEval = PieceEvaluations::pieceVals[piece];
			if (colorIndex == Board::blackIndex) {
				blackEval += pieceEval * numPieces;
			} else {
				whiteEval += pieceEval * numPieces;
			}
			endGameWeight += numPieces;
			for (int it = 0; it < numPieces; it++) {
				int square = board->pieceLists[colorIndex][piece][it];
				if (colorIndex == Board::blackIndex) {
					blackEval += PieceEvaluations::pieceEvals[piece][blackEvalSquare[square]];
				} else {
					whiteEval += PieceEvaluations::pieceEvals[piece][square];
				}
			}
		}
	}

	evaluation = maximisingSide * (whiteEval - blackEval);
	evaluation += kingDist(board);

	return evaluation;
}

int Evaluator::kingDist(Board* board) {
	int evaluation = 0;
	int whiteKingSquare = board->pieceLists[Board::whiteIndex][Piece::king][0];
	int blackKingSquare = board->pieceLists[Board::blackIndex][Piece::king][0];


	int whiteDistRow = max(3 - whiteKingSquare / 8, whiteKingSquare / 8 - 4);
	int whiteDistCol = max(3 - whiteKingSquare % 8, whiteKingSquare % 8 - 4);
	evaluation = whiteDistRow + whiteDistCol;

	int blackDistRow = abs(blackKingSquare - whiteKingSquare) / 8;
	int blackDistCol = abs(blackKingSquare - blackKingSquare) % 8;

	evaluation += min(50, (10 / endGameWeight) * (14 - (blackDistCol + blackDistRow)));
	return evaluation;
}

void Evaluator::initBlackSquares() {
	for (int square = 0; square < 64; square++) {
		int row = square / 8;
		int col = square % 8;
		blackEvalSquare[square] = (7 - row) * 8 + col;
	}
}
