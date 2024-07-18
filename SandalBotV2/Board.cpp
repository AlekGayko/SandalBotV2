#include "Board.h"
#include "FEN.h"

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
	state = BoardState(true, Piece::empty, -1, 0b1111, 0, 0, 0);
	loadPieceLists();
}

void Board::loadPieceLists() {
	for (int square = 0; square < 64; square++) {
		int piece = squares[square];
		int colorIndex = Piece::isColor(piece, Piece::white) ? whiteIndex : blackIndex;
		int pieceType = Piece::type(piece);
		
		pieceLists[colorIndex][pieceType].addPiece(square);
	}
}

void Board::loadPosition(std::string fen) {
	FEN::PositionInfo newPos = FEN::fenToPosition(fen);

	for (int square = 0; square < 64; square++) {
		squares[square] = newPos.squares[square];
	}
	state.enPassantFile = newPos.enPassantFile;
	state.whiteTurn = newPos.whiteTurn;
	state.fiftyMoveCounter = newPos.fiftyMoveCount;
	state.moveCounter = newPos.moveCount;

	state.castlingRights = 0b000;
	if (newPos.whiteShortCastle) state.castlingRights = state.castlingRights | BoardState::whiteShortCastleMask;
	if (newPos.whiteLongCastle) state.castlingRights = state.castlingRights | BoardState::whiteLongCastleMask;
	if (newPos.blackShortCastle) state.castlingRights = state.castlingRights | BoardState::blackShortCastleMask;
	if (newPos.blackLongCastle) state.castlingRights = state.castlingRights | BoardState::blackLongCastleMask;

	loadPieceLists();
}

void Board::makeMove(Move& move) {
	if (Piece::type(move.takenPiece) == Piece::king) return;
	int piece = Piece::type(squares[move.startSquare]);
	int colorIndex = state.whiteTurn ? whiteIndex : blackIndex;
	int oppositeIndex = !state.whiteTurn ? whiteIndex : blackIndex;
	int startSquare = move.startSquare;
	int targetSquare = move.targetSquare;
	pieceLists[colorIndex][piece].movePiece(startSquare, targetSquare);
	squares[targetSquare] = squares[startSquare];
	squares[startSquare] = 0;

	if (move.takenPiece != Piece::empty) {
		//cout << move << endl;
		int type = Piece::type(move.takenPiece);
		//cout << type << ", " << !state.whiteTurn << endl;
		pieceLists[oppositeIndex][type].deletePiece(targetSquare);
	}

	state.nextMove(move, piece);
}

void Board::makeEnPassantChanges(Move& move) {
	const int fileDiff = move.startSquare % 8 - state.enPassantFile;
	squares[move.startSquare + fileDiff] = Piece::empty;
}

void Board::makeCastlingChanges(Move& move) {
	const int rookDistance = move.targetSquare % 8 < 4 ? -4 : 3;
	const int rookSpawnOffset = move.targetSquare % 8 < 4 ? 1 : -1;
	const int friendlyRook = state.whiteTurn ? Piece::whiteRook : Piece::blackRook;

	squares[move.startSquare + rookDistance] = Piece::empty;
	squares[move.targetSquare + rookSpawnOffset] = friendlyRook;

	int castleMask = 0b0001;
	castleMask = state.whiteTurn ? castleMask : castleMask << 2;
	castleMask = move.targetSquare % 8 < 4 ? castleMask << 1 : castleMask;
	state.castlingRights = state.castlingRights & (~castleMask & 0b1111);
}

void Board::makePromotionChanges(Move& move) {
	const int targetSquare = move.targetSquare;
	const int color = state.whiteTurn ? Piece::white : Piece::black;

	switch (move.flag) {
	case Move::promoteToQueenFlag:
		squares[targetSquare] = Piece::queen | color;
		break;
	case Move::promoteToRookFlag:
		squares[targetSquare] = Piece::rook | color;
		break;
	case Move::promoteToKnightFlag:
		squares[targetSquare] = Piece::knight | color;
		break;
	case Move::promoteToBishopFlag:
		squares[targetSquare] = Piece::bishop | color;
		break;
	}
}

void Board::unMakeMove(Move& move) {
	if (Piece::type(move.takenPiece) == Piece::king) return;
	const int piece = Piece::type(squares[move.targetSquare]);
	const int colorIndex = !state.whiteTurn ? whiteIndex : blackIndex;
	const int oppositeIndex = state.whiteTurn ? whiteIndex : blackIndex;
	const int startSquare = move.startSquare;
	const int targetSquare = move.targetSquare;

	pieceLists[colorIndex][piece].movePiece(targetSquare, startSquare);

	squares[startSquare] = squares[targetSquare];
	squares[targetSquare] = move.takenPiece;
	if (move.takenPiece != Piece::empty) {
		//cout << move << endl;
		//cout << printBoard() << endl;
		int type = Piece::type(move.takenPiece);
		pieceLists[oppositeIndex][type].addPiece(targetSquare);
	}

	state.prevMove(move, piece);
}

void Board::undoEnPassantChanges(Move& move) {
	// is enpassantfile correct?
	const int fileDiff = move.startSquare % 8 - state.enPassantFile;
	squares[move.startSquare + fileDiff] = move.takenPiece;
}

void Board::undoCastlingChanges(Move& move) {
	const int rookDistance = move.targetSquare % 8 < 4 ? -4 : 3;
	const int rookSpawnOffset = move.targetSquare % 8 < 4 ? 1 : -1;
	const int friendlyRook = state.whiteTurn ? Piece::whiteRook : Piece::blackRook;

	squares[move.startSquare + rookDistance] = friendlyRook;
	squares[move.targetSquare + rookSpawnOffset] = Piece::empty;

	int castleMask = 0b0001;
	castleMask = state.whiteTurn ? castleMask : castleMask << 2;
	castleMask = move.targetSquare % 8 < 4 ? castleMask << 1 : castleMask;
	state.castlingRights = state.castlingRights | castleMask;
}

void Board::undoPromotionChanges(Move& move) {
	const int startSquare = move.startSquare;
	const int pawn = state.whiteTurn ? Piece::whitePawn : Piece::whitePawn;

	switch (move.flag) {
	case Move::promoteToQueenFlag:
		squares[startSquare] = pawn;
		break;
	case Move::promoteToRookFlag:
		squares[startSquare] = pawn;
		break;
	case Move::promoteToKnightFlag:
		squares[startSquare] = pawn;
		break;
	case Move::promoteToBishopFlag:
		squares[startSquare] = pawn;
		break;
	}
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

	result += FEN::generateFEN(this);
	result += '\n';

	return result;
}
