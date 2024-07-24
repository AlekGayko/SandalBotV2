#include "Board.h"
#include "FEN.h"
#include "MoveGen.h"

#include <iostream>
#include <stdexcept>
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
	boardStateHistory.reserve(150);
	boardStateHistory.push_back(BoardState(true, Piece::empty, -1, 0b1111, 0, 0, 0));
	state = &boardStateHistory.back();
	loadPieceLists();
	//loadBitBoards();
}

Board::~Board() {
}

void Board::loadPieceLists() {
	for (int color = 0; color < 2; color++) {
		for (int piece = 0; piece < 7; piece++) {
			pieceLists[color][piece].numPieces = 0;
		}
	}

	for (int square = 0; square < 64; square++) {
		int piece = squares[square];
		int colorIndex = Piece::isColor(piece, Piece::white) ? whiteIndex : blackIndex;
		int pieceType = Piece::type(piece);
		if (pieceType == Piece::empty) continue;
		pieceLists[colorIndex][pieceType].addPiece(square);
	}
}

void Board::loadPosition(std::string fen) {
	FEN::PositionInfo newPos = FEN::fenToPosition(fen);
	std::copy(std::begin(newPos.squares), std::end(newPos.squares), squares);

	boardStateHistory.clear();

	int enPassantSquare = newPos.enPassantSquare;
	bool whiteTurn = newPos.whiteTurn;
	int fiftyMoveCounter = newPos.fiftyMoveCount;
	int moveCounter = newPos.moveCount;
	int castlingRights = 0b000;

	if (newPos.whiteShortCastle) castlingRights = BoardState::whiteShortCastleMask;
	if (newPos.whiteLongCastle) castlingRights |= BoardState::whiteLongCastleMask;
	if (newPos.blackShortCastle) castlingRights |= BoardState::blackShortCastleMask;
	if (newPos.blackLongCastle) castlingRights |= BoardState::blackLongCastleMask;

	boardStateHistory.push_back(BoardState(whiteTurn, Piece::empty, enPassantSquare, castlingRights, fiftyMoveCounter, moveCounter, 0ULL));
	state = &boardStateHistory.back();

	loadPieceLists();
	//loadBitBoards();
}

void Board::loadBitBoards() {
	whitePieces = 0ULL;
	blackPieces = 0ULL;
	orthogonalPieces = 0ULL;
	diagonalPieces = 0ULL;
	knights = 0ULL;
	pawns = 0ULL;

	for (int square = 0; square < 64; square++) {
		int type = Piece::type(squares[square]);
		int color = Piece::color(squares[square]);
		switch (type) {
		case Piece::pawn:
			pawns |= 0b1 << square;
			break;
		case Piece::knight:
			knights |= 0b1 << square;
			break;
		case Piece::king:
			//kings |= 0b1 << square;
			break;
		case Piece::bishop:
			diagonalPieces |= 0b1 << square;
			break;
		case Piece::rook:
			orthogonalPieces |= 0b1 << square;
			break;
		case Piece::queen:
			orthogonalPieces |= 0b1 << square;
			diagonalPieces |= 0b1 << square;
			break;
		}

		switch (color) {
		case Piece::white:
			whitePieces |= 0b1 << square;
		case Piece::black:
			blackPieces |= 0b1 << square;
		}
	}
	allPieces = whitePieces | blackPieces;
}

void Board::makeMove(Move& move) {
	int startSquare = move.startSquare;
	int targetSquare = move.targetSquare;
	int piece = Piece::type(squares[startSquare]);
	int colorIndex = state->whiteTurn ? whiteIndex : blackIndex;
	int oppositeIndex = !(state->whiteTurn) ? whiteIndex : blackIndex;
	int takenPiece = squares[targetSquare];
	int flag = move.flag;
	int enPassantSquare = -1;
	int fiftyMoveCounter = piece == Piece::pawn || takenPiece != Piece::empty ? 0 : state->fiftyMoveCounter + 1;
	int castlingRights = state->castlingRights;

	if (Piece::isType(takenPiece, Piece::king)) {
		cout << printBoard() << endl;
		cout << move << endl;
		throw std::out_of_range("");
	}

	if (flag > Move::castleFlag) {
		pieceLists[colorIndex][piece].deletePiece(startSquare);
		makePromotionChanges(move, piece);
		pieceLists[colorIndex][piece].addPiece(targetSquare);
	} else {
		pieceLists[colorIndex][piece].movePiece(startSquare, targetSquare);
		squares[targetSquare] = squares[startSquare];
		squares[startSquare] = Piece::empty;
	}

	if (takenPiece != Piece::empty) {
		int type = Piece::type(takenPiece);
		try {
			pieceLists[oppositeIndex][type].deletePiece(targetSquare);
		} catch (...) {
			cout << printBoard() << endl;
			cout << move << endl;
			cout << move.targetSquare << endl;
			cout << Piece::pieceToSymbol(takenPiece) << endl;
			throw std::out_of_range("");
		}
	}
	int castleMask;

	switch (piece) {
	case Piece::king:
		castleMask = 0b0011;
		castleMask = state->whiteTurn ? castleMask : castleMask << 2;
		castlingRights &= ~castleMask & 0b1111;
		break;
	case Piece::rook:
		castleMask = 0b0001;
		castleMask = state->whiteTurn ? castleMask : castleMask << 2;
		if (startSquare == MoveGen::longCastleRookSquares[colorIndex]) {
			castleMask <<= 1;
		} else if (startSquare != MoveGen::shortCastleRookSquares[colorIndex]) {
			break;
		}
		castlingRights &= ~castleMask & 0b1111;
		break;
	}

	int pawnToBeDeleted;
	int rookOffset;
	int rookSquareOffset;
	switch (flag) {
	case Move::noFlag:
		break;
	case Move::pawnTwoSquaresFlag:
		enPassantSquare = targetSquare + (state->whiteTurn ? 8 : -8);
		break;
	case Move::enPassantCaptureFlag:
		makeEnPassantChanges(move);
		pawnToBeDeleted = targetSquare + (state->whiteTurn ? 8 : -8);
		pieceLists[oppositeIndex][Piece::pawn].deletePiece(pawnToBeDeleted);
		break;
	case Move::castleFlag:
		makeCastlingChanges(move, castlingRights);
		rookOffset = targetSquare % 8 > 4 ? 1 : -1;
		rookSquareOffset = rookOffset == 1 ? 3 : -4;
		pieceLists[colorIndex][Piece::rook].movePiece(startSquare + rookSquareOffset, startSquare + rookOffset);
		break;
	}

	//updateBitBoards(move);
	boardStateHistory.push_back(BoardState(!(state->whiteTurn), takenPiece, enPassantSquare, castlingRights, fiftyMoveCounter, state->moveCounter + 1, 0ULL));
	state = &boardStateHistory.back();
}

void Board::unMakeMove(Move& move) {
	const int colorIndex = !state->whiteTurn ? whiteIndex : blackIndex;
	const int oppositeIndex = state->whiteTurn ? whiteIndex : blackIndex;
	const int startSquare = move.startSquare;
	const int targetSquare = move.targetSquare;
	int piece = Piece::type(squares[targetSquare]);
	const int takenPiece = state->capturedPiece;
	const int flag = move.flag;
	const int pawn = !(state->whiteTurn) ? Piece::whitePawn : Piece::blackPawn;
	if (flag > Move::castleFlag) {
		pieceLists[colorIndex][piece].deletePiece(targetSquare);
		piece = Piece::pawn;
		pieceLists[colorIndex][piece].addPiece(startSquare);
		squares[startSquare] = pawn;
		squares[targetSquare] = takenPiece;
	} else {
		pieceLists[colorIndex][piece].movePiece(targetSquare, startSquare);

		squares[startSquare] = squares[targetSquare];
		squares[targetSquare] = takenPiece;
	}

	if (takenPiece != Piece::empty) {
		int type = Piece::type(takenPiece);
		try {
			pieceLists[oppositeIndex][type].addPiece(targetSquare);
		} catch (...) {
			cout << printBoard() << endl;
			cout << move << endl;
			cout << Piece::pieceToSymbol(takenPiece) << endl;
			cout << pieceLists[oppositeIndex][type].numPieces << endl;
			throw std::out_of_range("");
		}
	}

	int pawnToBeDeleted;
	int rookOffset;
	int rookSquareOffset;
	switch (flag) {
	case Move::noFlag:
		break;
	case Move::enPassantCaptureFlag:
		undoEnPassantChanges(move);
		pawnToBeDeleted = targetSquare + (state->whiteTurn ? -8 : 8);
		pieceLists[oppositeIndex][Piece::pawn].addPiece(pawnToBeDeleted);
		break;
	case Move::castleFlag:
		undoCastlingChanges(move);
		rookOffset = targetSquare % 8 > 4 ? 1 : -1;
		rookSquareOffset = rookOffset == 1 ? 3 : -4;
		pieceLists[colorIndex][Piece::rook].movePiece(startSquare + rookOffset, startSquare + rookSquareOffset);
		break;
	}

	//undoBitBoards(move);
	boardStateHistory.pop_back();
	state = &boardStateHistory.back();
}

void Board::makeEnPassantChanges(Move& move) {
	int pawnToBeDeleted = move.targetSquare + (state->whiteTurn ? 8 : -8);
	squares[pawnToBeDeleted] = Piece::empty;
}

void Board::makeCastlingChanges(Move& move, int& castlingRights) {
	const int rookDistance = move.targetSquare % 8 < 4 ? -4 : 3;
	const int rookSpawnOffset = move.targetSquare % 8 < 4 ? -1 : 1;
	const int friendlyRook = state->whiteTurn ? Piece::whiteRook : Piece::blackRook;
	squares[move.startSquare + rookDistance] = Piece::empty;
	squares[move.startSquare + rookSpawnOffset] = friendlyRook;
}

void Board::makePromotionChanges(Move& move, int& piece) {
	const int targetSquare = move.targetSquare;
	const int color = state->whiteTurn ? Piece::white : Piece::black;
	squares[move.startSquare] = Piece::empty;

	switch (move.flag) {
	case Move::promoteToQueenFlag:
		squares[targetSquare] = Piece::queen | color;
		piece = Piece::queen;
		break;
	case Move::promoteToRookFlag:
		squares[targetSquare] = Piece::rook | color;
		piece = Piece::rook;
		break;
	case Move::promoteToKnightFlag:
		squares[targetSquare] = Piece::knight | color;
		piece = Piece::knight;
		break;
	case Move::promoteToBishopFlag:
		squares[targetSquare] = Piece::bishop | color;
		piece = Piece::bishop;
		break;
	}
}

void Board::undoEnPassantChanges(Move& move) {
	const int enemyPawn = state->whiteTurn ? Piece::whitePawn : Piece::blackPawn;
	int pawnToBeDeleted = move.targetSquare + (state->whiteTurn ? -8 : 8);;
	squares[pawnToBeDeleted] = enemyPawn;
}

void Board::undoCastlingChanges(Move& move) {
	//cout << printBoard() << endl;
	const int rookDistance = move.targetSquare % 8 < 4 ? -4 : 3;
	const int rookSpawnOffset = move.targetSquare % 8 < 4 ? -1 : 1;
	const int friendlyRook = state->whiteTurn ? Piece::blackRook : Piece::whiteRook;

	squares[move.startSquare + rookDistance] = friendlyRook;
	squares[move.startSquare + rookSpawnOffset] = Piece::empty;
	//cout << printBoard() << endl;
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

void Board::updateBitBoards(Move& move, int pieceType, int takenPiece) {
	const int startSquare = move.startSquare;
	const int targetSquare = move.targetSquare;
	unsigned long long int* friendlyBoard = state->whiteTurn ? &whitePieces : &blackPieces;
	unsigned long long int* enemyBoard = !state->whiteTurn ? &whitePieces : &blackPieces;

	switch (pieceType) {
	case Piece::pawn:
		pawns &= ~(0b1 << startSquare);
		pawns |= (0b1 << targetSquare);
		break;
	case Piece::knight:
		knights &= ~(0b1 << startSquare);
		knights |= (0b1 << targetSquare);
		break;
	case Piece::rook:
		orthogonalPieces &= ~(0b1 << startSquare);
		orthogonalPieces |= (0b1 << targetSquare);
		break;
	case Piece::queen:
		orthogonalPieces &= ~(0b1 << startSquare);
		diagonalPieces &= ~(0b1 << startSquare);
		
		orthogonalPieces |= 0b1 << targetSquare;
		diagonalPieces |= 0b1 << targetSquare;
		break;
	case Piece::bishop:
		diagonalPieces &= ~(0b1 << startSquare);
		diagonalPieces |= (0b1 << targetSquare);
		break;
	}

	switch (takenPiece) {
	case Piece::pawn:
		pawns &= ~(0b1 << targetSquare);
		break;
	case Piece::knight:
		knights &= ~(0b1 << targetSquare);
		break;
	case Piece::rook:
		orthogonalPieces &= ~(0b1 << targetSquare);
		break;
	case Piece::queen:
		orthogonalPieces &= ~(0b1 << targetSquare);
		diagonalPieces &= ~(0b1 << targetSquare);
		break;
	case Piece::bishop:
		diagonalPieces &= ~(0b1 << targetSquare);
		break;
	}

}

void Board::undoBitBoards(Move& move, int pieceType, int takenPiece) {
}
