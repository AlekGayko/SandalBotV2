#include "Board.h"
#include "Evaluator.h"
#include "FEN.h"
#include "MoveGen.h"
#include "ZobristHash.h"

#include <intrin.h>

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
	loadPieceLists();
	BoardState initState = BoardState(true, Piece::empty, -1, 0b1111, 0, 0, 0ULL);
	stateHistory.push(initState);
	state = &stateHistory.back();
	state->zobristHash = ZobristHash::hashBoard(this);
	history.push(state->zobristHash, false);
	loadBitBoards();
}

Board::~Board() {

}

void Board::setEvaluator(Evaluator* evaluator) {
	this->evaluator = evaluator;
	this->evaluator->initStaticVariables();
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

	stateHistory.clear();

	int enPassantSquare = newPos.enPassantSquare;
	bool whiteTurn = newPos.whiteTurn;
	int fiftyMoveCounter = newPos.fiftyMoveCount;
	int moveCounter = newPos.moveCount;
	int castlingRights = 0b000;

	if (newPos.whiteShortCastle) castlingRights = BoardState::whiteShortCastleMask;
	if (newPos.whiteLongCastle) castlingRights |= BoardState::whiteLongCastleMask;
	if (newPos.blackShortCastle) castlingRights |= BoardState::blackShortCastleMask;
	if (newPos.blackLongCastle) castlingRights |= BoardState::blackLongCastleMask;

	loadPieceLists();
	BoardState newState = BoardState(whiteTurn, Piece::empty, enPassantSquare, castlingRights, fiftyMoveCounter, moveCounter, 0ULL);
	stateHistory.push(newState);
	state = &stateHistory.back();
	state->zobristHash = ZobristHash::hashBoard(this);
	history.push(state->zobristHash, false);
	loadBitBoards();
	evaluator->initStaticVariables();
}

void Board::loadBitBoards() {
	whitePieces = 0ULL;
	blackPieces = 0ULL;
	orthogonalPieces = 0ULL;
	diagonalPieces = 0ULL;
	knights = 0ULL;
	pawns = 0ULL;

	for (int color = blackIndex; color <= whiteIndex; color++) {
		for (int type = Piece::pawn; type <= Piece::king; type++) {
			for (int i = 0; i < pieceLists[color][type].numPieces; i++) {
				int square = pieceLists[color][type][i];
				switch (type) {
				case Piece::pawn:
					pawns |= 1ULL << square;
					break;
				case Piece::knight:
					knights |= 1ULL << square;
					break;
				case Piece::bishop:
					diagonalPieces |= 1ULL << square;
					break;
				case Piece::rook:
					orthogonalPieces |= 1ULL << square;
					break;
				case Piece::queen:
					orthogonalPieces |= 1ULL << square;
					diagonalPieces |= 1ULL << square;
					break;
				}

				switch (color) {
				case whiteIndex:
					whitePieces |= 1ULL << square;
					break;
				case blackIndex:
					blackPieces |= 1ULL << square;
					break;
				}
			}
		}
	}
	allPieces = whitePieces | blackPieces;
}

void Board::makeMove(Move& move, bool hashBoard) {
	int startSquare = move.getStartSquare();
	int targetSquare = move.getTargetSquare();
	int piece = Piece::type(squares[startSquare]);
	int colorIndex = state->whiteTurn ? whiteIndex : blackIndex;
	int oppositeIndex = !(state->whiteTurn) ? whiteIndex : blackIndex;
	int takenPiece = squares[targetSquare];
	int flag = move.getFlag();
	int promotedPiece = 0;
	int enPassantSquare = -1;
	int fiftyMoveCounter = piece == Piece::pawn || takenPiece != Piece::empty ? 0 : state->fiftyMoveCounter + 1;
	int castlingRights = state->castlingRights;
	uint64_t newZobristHash = state->zobristHash;

	newZobristHash ^= ZobristHash::whiteMoveHash;

	if (Piece::type(takenPiece) == Piece::king) {
		cout << move << endl;
		printBoard();
		BitBoardUtility::printBB(allPieces);
		BitBoardUtility::printBB(whitePieces);
		BitBoardUtility::printBB(blackPieces);
		BitBoardUtility::printBB(orthogonalPieces);
		BitBoardUtility::printBB(diagonalPieces);
		BitBoardUtility::printBB(knights);
		BitBoardUtility::printBB(pawns);
		cout << stateHistory.getSecondLast().zobristHash << endl;
		throw exception();
	}

	newZobristHash ^= ZobristHash::pieceHashes[colorIndex][piece][startSquare];

	if (flag >= Move::promoteToQueenFlag) {
		promotedPiece = makePromotionChanges(move, piece, colorIndex);
		newZobristHash ^= ZobristHash::pieceHashes[colorIndex][promotedPiece][targetSquare];
	} else {
		evaluator->staticPieceMove(piece, startSquare, targetSquare, state->whiteTurn);
		pieceLists[colorIndex][piece].movePiece(startSquare, targetSquare);
		squares[targetSquare] = squares[startSquare];
		squares[startSquare] = Piece::empty;
		newZobristHash ^= ZobristHash::pieceHashes[colorIndex][piece][targetSquare];
	}

	if (takenPiece != Piece::empty) {
		int type = Piece::type(takenPiece);
		evaluator->staticPieceDelete(type, targetSquare, !state->whiteTurn);
		pieceLists[oppositeIndex][type].deletePiece(targetSquare);
		newZobristHash ^= ZobristHash::pieceHashes[colorIndex][type][targetSquare];
	}

	castlingRights = updateCastlingRights(piece, colorIndex, startSquare, castlingRights);

	flagMoveChanges(move, newZobristHash, enPassantSquare, castlingRights, colorIndex, oppositeIndex);

	if (state->enPassantSquare != -1) 
		newZobristHash ^= ZobristHash::enPassantHash[state->enPassantSquare];

	newZobristHash ^= ZobristHash::castlingRightsHash[state->castlingRights];
	newZobristHash ^= ZobristHash::castlingRightsHash[castlingRights];

	updateBitBoards(move, piece, Piece::type(takenPiece));
	BoardState newState = BoardState(!(state->whiteTurn), takenPiece, enPassantSquare, castlingRights, fiftyMoveCounter, state->moveCounter + 1, newZobristHash);
	stateHistory.push(newState);
	state = &stateHistory.back();

	bool reset = takenPiece != Piece::empty || piece == Piece::pawn;

	history.push(state->zobristHash, reset);
}

void Board::flagMoveChanges(Move& move, uint64_t& newZobristHash, int& enPassantSquare, int& castlingRights, int colorIndex, int oppositeIndex) {
	switch (move.getFlag()) {
	case Move::pawnTwoSquaresFlag:
		enPassantSquare = move.getTargetSquare() + (state->whiteTurn ? 8 : -8);
		newZobristHash ^= ZobristHash::enPassantHash[enPassantSquare];
		break;
	case Move::enPassantCaptureFlag:
		makeEnPassantChanges(move, newZobristHash, oppositeIndex);
		break;
	case Move::castleFlag:
		makeCastlingChanges(move, newZobristHash, castlingRights, colorIndex);
		break;
	}
}

int Board::updateCastlingRights(const int& piece, const int& colorIndex, const int& startSquare, int castlingRights) {
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

	return castlingRights;
}

void Board::unMakeMove(Move& move) {
	const int colorIndex = !state->whiteTurn ? whiteIndex : blackIndex;
	const int oppositeIndex = state->whiteTurn ? whiteIndex : blackIndex;
	const int startSquare = move.getStartSquare();
	const int targetSquare = move.getTargetSquare();
	int piece = Piece::type(squares[targetSquare]);
	const int takenPiece = state->capturedPiece;
	const int flag = move.getFlag();
	const int pawn = !(state->whiteTurn) ? Piece::whitePawn : Piece::blackPawn;
	if (flag >= Move::promoteToQueenFlag) {
		pieceLists[colorIndex][piece].deletePiece(targetSquare);
		evaluator->staticPieceDelete(piece, targetSquare, !state->whiteTurn);
		evaluator->staticPieceSpawn(Piece::pawn, startSquare, !state->whiteTurn);
		piece = Piece::pawn;
		pieceLists[colorIndex][piece].addPiece(startSquare);
		squares[startSquare] = pawn;
		squares[targetSquare] = takenPiece;
	} else {
		pieceLists[colorIndex][piece].movePiece(targetSquare, startSquare);
		evaluator->staticPieceMove(piece, targetSquare, startSquare, !state->whiteTurn);
		squares[startSquare] = squares[targetSquare];
		squares[targetSquare] = takenPiece;
	}

	if (takenPiece != Piece::empty) {
		int type = Piece::type(takenPiece);
		pieceLists[oppositeIndex][type].addPiece(targetSquare);
		evaluator->staticPieceSpawn(type, targetSquare, state->whiteTurn);
	}

	int pawnToBeDeleted;
	int rookOffset;
	int rookSquareOffset;
	switch (flag) {
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

	undoBitBoards(move, piece, Piece::type(takenPiece));
	history.pop();
	stateHistory.pop();
	state = &stateHistory.back();
}

void Board::makeEnPassantChanges(Move& move, uint64_t& newZobristHash, int oppositeIndex) {
	int pawnToBeDeleted = move.getTargetSquare() + (state->whiteTurn ? 8 : -8);

	pieceLists[oppositeIndex][Piece::pawn].deletePiece(pawnToBeDeleted);
	newZobristHash ^= ZobristHash::pieceHashes[oppositeIndex][Piece::pawn][pawnToBeDeleted];
	squares[pawnToBeDeleted] = Piece::empty;
	evaluator->staticPieceDelete(Piece::pawn, pawnToBeDeleted, !state->whiteTurn);
}

void Board::makeCastlingChanges(Move& move, uint64_t& newZobristHash, int& castlingRights, int colorIndex) {
	const int startSquare = move.getStartSquare() + (move.getTargetSquare() < move.getStartSquare() ? -4 : 3);
	const int targetSquare = move.getStartSquare() + (move.getTargetSquare() < move.getStartSquare() ? -1 : 1);
	const int friendlyRook = state->whiteTurn ? Piece::whiteRook : Piece::blackRook;

	squares[startSquare] = Piece::empty;
	squares[targetSquare] = friendlyRook;
	pieceLists[colorIndex][Piece::rook].movePiece(startSquare, targetSquare);
	newZobristHash ^= ZobristHash::pieceHashes[colorIndex][Piece::rook][startSquare];
	newZobristHash ^= ZobristHash::pieceHashes[colorIndex][Piece::rook][targetSquare];
	evaluator->staticPieceMove(Piece::rook, startSquare, targetSquare, state->whiteTurn);
}

int Board::makePromotionChanges(Move& move, int& piece, int colorIndex) {
	int promotedPiece = piece;
	const int ownColor = state->whiteTurn ? Piece::white : Piece::black;

	switch (move.getFlag()) {
	case Move::promoteToQueenFlag:
		promotedPiece = Piece::queen;
		break;
	case Move::promoteToRookFlag:
		promotedPiece = Piece::rook;
		break;
	case Move::promoteToKnightFlag:
		promotedPiece = Piece::knight;
		break;
	case Move::promoteToBishopFlag:
		promotedPiece = Piece::bishop;
		break;
	}

	squares[move.getStartSquare()] = Piece::empty;
	squares[move.getTargetSquare()] = promotedPiece | ownColor;
	pieceLists[colorIndex][piece].deletePiece(move.getStartSquare());
	pieceLists[colorIndex][promotedPiece].addPiece(move.getTargetSquare());
	evaluator->staticPieceDelete(Piece::pawn, move.getStartSquare(), state->whiteTurn);
	evaluator->staticPieceSpawn(promotedPiece, move.getTargetSquare(), state->whiteTurn);

	return promotedPiece;
}

void Board::undoEnPassantChanges(Move& move) {
	const int enemyPawn = state->whiteTurn ? Piece::whitePawn : Piece::blackPawn;
	int pawnToBeDeleted = move.getTargetSquare() + (state->whiteTurn ? -8 : 8);;
	squares[pawnToBeDeleted] = enemyPawn;
	evaluator->staticPieceSpawn(Piece::pawn, pawnToBeDeleted, state->whiteTurn);
}

void Board::undoCastlingChanges(Move& move) {
	const int startSquare = move.getStartSquare() + (move.getTargetSquare() < move.getStartSquare() ? -4 : 3);
	const int targetSquare = move.getStartSquare() + (move.getTargetSquare() < move.getStartSquare() ? -1 : 1);
	const int friendlyRook = state->whiteTurn ? Piece::blackRook : Piece::whiteRook;

	squares[startSquare] = friendlyRook;
	squares[targetSquare] = Piece::empty;
	evaluator->staticPieceMove(Piece::rook, targetSquare, startSquare, !state->whiteTurn);
}

void Board::printBoard() {
	string result = "";

	for (int y = 0; y < 8; y++) {
		result += "+---+---+---+---+---+---+---+---+\n";

		for (int x = 0; x < 8; x++) {
			int squareIndex = CoordHelper::coordToIndex(y, x);

			int piece = squares[squareIndex];

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
		}
	}

	result += "Fen: ";
	result += FEN::generateFEN(this);
	result += '\n';

	result += "Key: ";
	result += to_string(state->zobristHash);
	result += '\n';

	cout << result << endl;
}

// Get function for all bitboards
std::vector<uint64_t> Board::getBitBoards() {
	vector<uint64_t> bitboards;
	bitboards.reserve(7);

	bitboards.push_back(allPieces);
	bitboards.push_back(whitePieces);
	bitboards.push_back(blackPieces);
	bitboards.push_back(pawns);
	bitboards.push_back(knights);
	bitboards.push_back(orthogonalPieces);
	bitboards.push_back(diagonalPieces);

	return bitboards;
}

// Update piece bitboards based on move
void Board::updateBitBoards(Move& move, int pieceType, int takenPieceType) {
	// Initialise commonly used variables
	const int& startSquare = move.getStartSquare();
	const int& targetSquare = move.getTargetSquare();
	uint64_t* friendlyBoard = state->whiteTurn ? &whitePieces : &blackPieces;
	uint64_t* enemyBoard = !state->whiteTurn ? &whitePieces : &blackPieces;

	// Move the moved piece along its bitboard
	switch (pieceType) {
	case Piece::pawn:
		BitBoardUtility::moveBit(pawns, startSquare, targetSquare);
		break;
	case Piece::knight:
		BitBoardUtility::moveBit(knights, startSquare, targetSquare);
		break;
	case Piece::rook:
		BitBoardUtility::moveBit(orthogonalPieces, startSquare, targetSquare);
		break;
	case Piece::queen:
		BitBoardUtility::moveBit(orthogonalPieces, startSquare, targetSquare);
		BitBoardUtility::moveBit(diagonalPieces, startSquare, targetSquare);
		break;
	case Piece::bishop:
		BitBoardUtility::moveBit(diagonalPieces, startSquare, targetSquare);
		break;
	}

	BitBoardUtility::moveBit(*friendlyBoard, startSquare, targetSquare);

	// If taken piece is not empty and not the same type, delete bit from respective bitboard
	if (takenPieceType != Piece::empty && pieceType != takenPieceType) {
		switch (takenPieceType) {
		case Piece::pawn:
			BitBoardUtility::deleteBit(pawns, targetSquare);
			break;
		case Piece::knight:
			BitBoardUtility::deleteBit(knights, targetSquare);
			break;
		case Piece::rook:
			if (pieceType == Piece::queen)
				break;
			BitBoardUtility::deleteBit(orthogonalPieces, targetSquare);
			break;
		case Piece::bishop:
			if (pieceType == Piece::queen)
				break;
			BitBoardUtility::deleteBit(diagonalPieces, targetSquare);
			break;
		case Piece::queen:
			if (pieceType != Piece::rook)
				BitBoardUtility::deleteBit(orthogonalPieces, targetSquare);

			if (pieceType != Piece::bishop)
				BitBoardUtility::deleteBit(diagonalPieces, targetSquare);
			break;
		}
	}

	
	// If taken piece not empty, remove from enemy bitboard
	if (takenPieceType != Piece::empty) {
		BitBoardUtility::deleteBit(*enemyBoard, targetSquare);
	}

	// If move is special and has unconsidered consequences from previous sections, cover edge cases
	switch (move.getFlag()) {
		// Remove pawn that is removed during en passant
	case Move::enPassantCaptureFlag:
		BitBoardUtility::deleteBit(pawns, *enemyBoard, targetSquare + (state->whiteTurn ? 8 : -8));
		break;
		// Move rook during castling
	case Move::castleFlag:
		BitBoardUtility::moveBit(orthogonalPieces, *friendlyBoard, targetSquare < startSquare ? startSquare - 4 : startSquare + 3,
			targetSquare < startSquare ? targetSquare + 1 : targetSquare - 1);
		break;
		// Delete pawn from bitboard and replace with respective promotions
	case Move::promoteToQueenFlag:
		BitBoardUtility::deleteBit(pawns, targetSquare);
		BitBoardUtility::setBit(orthogonalPieces, targetSquare);
		BitBoardUtility::setBit(diagonalPieces, targetSquare);
		break;
	case Move::promoteToRookFlag:
		BitBoardUtility::deleteBit(pawns, targetSquare);
		BitBoardUtility::setBit(orthogonalPieces, targetSquare);
		break;
	case Move::promoteToBishopFlag:
		BitBoardUtility::deleteBit(pawns, targetSquare);
		BitBoardUtility::setBit(diagonalPieces, targetSquare);
		break;
	case Move::promoteToKnightFlag:
		BitBoardUtility::deleteBit(pawns, targetSquare);
		BitBoardUtility::setBit(knights, targetSquare);
		break;
	}

	// Update all Pieces with union of whitePieces and blackPieces
	allPieces = whitePieces | blackPieces;
}

void Board::undoBitBoards(Move& move, int pieceType, int takenPieceType) {
	// Initialise commonly used variables
	const int& startSquare = move.getStartSquare();
	const int& targetSquare = move.getTargetSquare();
	uint64_t* friendlyBoard = !state->whiteTurn ? &whitePieces : &blackPieces;
	uint64_t* enemyBoard = state->whiteTurn ? &whitePieces : &blackPieces;

	// If move is special and has unconsidered consequences from previous sections, cover edge cases
	switch (move.getFlag()) {
		// Remove pawn that is removed during en passant
	case Move::enPassantCaptureFlag:
		BitBoardUtility::setBit(pawns, *enemyBoard, targetSquare + (!state->whiteTurn ? 8 : -8));
		break;
		// Move rook during castling
	case Move::castleFlag:
		BitBoardUtility::moveBit(orthogonalPieces, *friendlyBoard, targetSquare < startSquare ? targetSquare + 1 : targetSquare - 1,
			targetSquare < startSquare ? startSquare - 4 : startSquare + 3);
		break;
		// Delete pawn from bitboard and replace with respective promotions
	case Move::promoteToQueenFlag:
		BitBoardUtility::setBit(pawns, targetSquare);
		BitBoardUtility::deleteBit(orthogonalPieces, targetSquare);
		BitBoardUtility::deleteBit(diagonalPieces, targetSquare);
		break;
	case Move::promoteToRookFlag:
		BitBoardUtility::setBit(pawns, targetSquare);
		BitBoardUtility::deleteBit(orthogonalPieces, targetSquare);
		break;
	case Move::promoteToBishopFlag:
		BitBoardUtility::setBit(pawns, targetSquare);
		BitBoardUtility::deleteBit(diagonalPieces, targetSquare);
		break;
	case Move::promoteToKnightFlag:
		BitBoardUtility::setBit(pawns, targetSquare);
		BitBoardUtility::deleteBit(knights, targetSquare);
		break;
	}

	// Move the moved piece along its bitboard
	switch (pieceType) {
	case Piece::pawn:
		BitBoardUtility::moveBit(pawns, targetSquare, startSquare);
		break;
	case Piece::knight:
		BitBoardUtility::moveBit(knights, targetSquare, startSquare);
		break;
	case Piece::rook:
		BitBoardUtility::moveBit(orthogonalPieces, targetSquare, startSquare);
		break;
	case Piece::queen:
		BitBoardUtility::moveBit(orthogonalPieces, targetSquare, startSquare);
		BitBoardUtility::moveBit(diagonalPieces, targetSquare, startSquare);
		break;
	case Piece::bishop:
		BitBoardUtility::moveBit(diagonalPieces, targetSquare, startSquare);
		break;
	}

	BitBoardUtility::moveBit(*friendlyBoard, targetSquare, startSquare);

	// If taken piece not empty, remove from enemy bitboard
	if (takenPieceType != Piece::empty) {
		BitBoardUtility::setBit(*enemyBoard, targetSquare);
	}

	// If taken piece is not empty and not the same type, delete bit from respective bitboard
	if (takenPieceType != Piece::empty) {
		switch (takenPieceType) {
		case Piece::pawn:
			BitBoardUtility::setBit(pawns, targetSquare);
			break;
		case Piece::knight:
			BitBoardUtility::setBit(knights, targetSquare);
			break;
		case Piece::rook:
			BitBoardUtility::setBit(orthogonalPieces, targetSquare);
			break;
		case Piece::bishop:
			BitBoardUtility::setBit(diagonalPieces, targetSquare);
			break;
		case Piece::queen:
			BitBoardUtility::setBit(orthogonalPieces, targetSquare);
			BitBoardUtility::setBit(diagonalPieces, targetSquare);
			break;
		}
	}

	// Update all Pieces with union of whitePieces and blackPieces
	allPieces = whitePieces | blackPieces;
}
