#include "Board.h"

#include <iostream>
#include <string>

using namespace std;

Board::Board() {
	int tempSquares[64] = {
			Piece::blackRook, Piece::blackKnight, Piece::blackBishop, Piece::blackQueen, Piece::blackKing, Piece::blackBishop, Piece::blackKnight, Piece::blackRook,
			Piece::blackPawn, Piece::blackPawn, Piece::blackPawn, Piece::blackPawn, Piece::blackPawn, Piece::blackPawn, Piece::blackPawn, Piece::blackPawn,
			Piece::empty, Piece::empty, Piece::empty, Piece::empty, Piece::empty, Piece::empty, Piece::empty, Piece::empty, 
			Piece::empty, Piece::empty, Piece::empty, Piece::empty, Piece::empty, Piece::empty, Piece::empty, Piece::empty, 
			Piece::empty, Piece::empty, Piece::empty, Piece::empty, Piece::empty, Piece::empty, Piece::empty, Piece::empty, 
			Piece::empty, Piece::empty, Piece::empty, Piece::empty, Piece::empty, Piece::empty, Piece::empty, Piece::empty,
			Piece::whitePawn, Piece::whitePawn, Piece::whitePawn, Piece::whitePawn, Piece::whitePawn, Piece::whitePawn, Piece::whitePawn, Piece::whitePawn, 
			Piece::whiteRook, Piece::whiteKnight, Piece::whiteBishop, Piece::whiteQueen, Piece::whiteKing, Piece::whiteBishop, Piece::whiteKnight, Piece::whiteRook 
		};
	std::copy(std::begin(tempSquares), std::end(tempSquares), squares);
	state = BoardState(true, 0, -1, 0b1111, 0, 0);
}

void Board::loadPosition(std::string fen) {
}

void Board::makeMove(Move move) {
	int startSquare = move.startSquare;
	int targetSquare = move.targetSquare;
	squares[targetSquare] = squares[startSquare];
	squares[startSquare] = 0;
	state.nextMove();
}

void Board::unMakeMove(Move move) {
	int startSquare = move.startSquare;
	int targetSquare = move.targetSquare;

	squares[startSquare] = squares[targetSquare];
	squares[targetSquare] = move.takenPiece;
	/*
	if (move.takenPiece != 0) {
		cout << "takenpiece: " << move.takenPiece << endl;
		cout << "piece: " << Piece::type(squares[startSquare]) << "startsquare: " << CoordHelper::indexToString(startSquare) << ", targetSquare: " << CoordHelper::indexToString(targetSquare) << endl;
	}
	*/

	state.prevMove();
}


string Board::printBoard() {
	// Thanks to ernestoyaquello
	string result = "";
	//int lastMoveSquare = board.AllGameMoves.Count > 0 ? board.AllGameMoves[^ 1].TargetSquare : -1;

	for (int y = 0; y < 8; y++) {
		//int rankIndex = blackAtTop ? 7 - y : y;
		result += "+---+---+---+---+---+---+---+---+\n";

		for (int x = 0; x < 8; x++) {
			int squareIndex = CoordHelper::coordToIndex(y, x);
			//bool highlight = squareIndex == lastMoveSquare;
			int piece = squares[squareIndex];
			/*
			if (highlight) {
				result.Append($"|({Piece.GetSymbol(piece)})");
			} else {
				result.Append($"| {Piece.GetSymbol(piece)} ");
			}
			*/
			result += "| ";
			result += Piece::pieceToSymbol(piece);
			result += " ";

			if (x == 7) {
				// Show rank number
				result += "| " + to_string(8 - y);
				result += '\n';
			}
		}

		if (y == 7) {
			// Show file names
			result += "+---+---+---+---+---+---+---+---+\n";
			const string fileNames = "  a   b   c   d   e   f   g   h  \n";
			const string fileNamesRev = "  h   g   f   e   d   c   b   a  \n";
			result += fileNames;
			result += '\n';

			/*
			if (includeFen) {
				result.AppendLine($"Fen         : {FenUtility.CurrentFen(board)}");
			}
			if (includeZobristKey) {
				result.AppendLine($"Zobrist Key : {board.ZobristKey}");
			}
			*/
		}
	}

	return result;
}
