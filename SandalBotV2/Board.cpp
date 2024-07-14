#include "Board.h"

Board::Board() {
}

inline void Board::MakeMove(Move move) {
	int startSquare = move.startSquare;
	int targetSquare = move.targetSquare;

	squares[targetSquare] = squares[startSquare];
	squares[startSquare] = 0;
}

inline void Board::UnMakeMove(Move move) {
	int startSquare = move.startSquare;
	int targetSquare = move.targetSquare;

	squares[startSquare] = squares[targetSquare];
	squares[targetSquare] = move.takenPiece;
}
