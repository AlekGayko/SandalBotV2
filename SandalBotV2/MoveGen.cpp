#include "MoveGen.h"

#include <iostream>
#include <limits>
#include <stdexcept>
#include <thread>

using namespace std;

const int MoveGen::startingKingSquares[2] = { 4, 60 };
const int MoveGen::shortCastleKingSquares[2] = { 6, 62 };
const int MoveGen::longCastleKingSquares[2] = { 2, 58 };
const int MoveGen::shortCastleRookSquares[2] = { 7, 63 };
const int MoveGen::longCastleRookSquares[2] = { 0, 56 };
const int MoveGen::shortCastleRookSpawn[2] = { 5, 61 };
const int MoveGen::longCastleRookSpawn[2] = { 3, 59 };

MoveGen::MoveGen() {

}

MoveGen::MoveGen(Board* board) {
	if (board == nullptr) throw std::invalid_argument("board cannot be nullptr");
	this->board = board;
}

int MoveGen::generateMoves(Move moves[], bool capturesOnly) {
	initVariables(capturesOnly);
	generateCheckData();
	generateKingMoves(moves);
	if (doubleCheck) return currentMoves;
	generatePawnMoves(moves);
	generateKnightMoves(moves);
	generateOrthogonalMoves(moves);
	generateDiagonalMoves(moves);

	//BitBoardUtility::printBB(opponentAttacks);
	//BitBoardUtility::printBB(checkBB);
	//BitBoardUtility::printBB(checkRayBB);
	return currentMoves;
}

void MoveGen::initVariables(bool capturesOnly) {
	squares = board->squares;
	isCheck = false;
	doubleCheck = false;
	whiteTurn = board->state->whiteTurn;
	enPassantSquare = board->state->enPassantSquare;
	castlingRights = board->state->castlingRights;
	fiftyMoveCounter = board->state->fiftyMoveCounter;
	moveCounter = board->state->moveCounter;

	colorIndex = whiteTurn ? Board::whiteIndex : Board::blackIndex;
	enemyColorIndex = !whiteTurn ? Board::whiteIndex : Board::blackIndex;
	currentColor = whiteTurn ? Piece::white : Piece::black;
	opposingColor = !whiteTurn ? Piece::white : Piece::black;
	currentMoves = 0ULL;
	/*
	friendlyPieceLists = board->pieceLists[colorIndex];
	enemyPieceLists = board->pieceLists[!colorIndex];

	orthogonalSliders[0] = &friendlyPieceLists[Piece::rook];
	orthogonalSliders[1] = &friendlyPieceLists[Piece::queen];

	diagonalSliders[0] = &friendlyPieceLists[Piece::bishop];
	diagonalSliders[1] = &friendlyPieceLists[Piece::queen];

	enemyOrthogonalSliders[0] = &enemyPieceLists[Piece::rook];
	enemyOrthogonalSliders[1] = &enemyPieceLists[Piece::queen];

	enemyDiagonalSliders[0] = &enemyPieceLists[Piece::bishop];
	enemyDiagonalSliders[1] = &enemyPieceLists[Piece::queen];


	pawns = &friendlyPieceLists[Piece::pawn];
	*/
	friendlyKingSquare = board->pieceLists[colorIndex][Piece::king][0];
	enemyKingSquare = board->pieceLists[enemyColorIndex][Piece::king][0];

	checkBB = 0ULL;
	checkRayBB = 0ULL;
	opponentAttacks = 0ULL;

	friendlyBoard = whiteTurn ? board->whitePieces : board->blackPieces;
	enemyBoard = !whiteTurn ? board->whitePieces : board->blackPieces;

	generateCaptures = capturesOnly;
}

void MoveGen::generateSlideMoves(Move moves[]) {

}

void MoveGen::generateOrthogonalMoves(Move moves[]) {
	int startSquare;
	int targetSquare;
	bool isPinned;
	uint64_t orthSliders = board->orthogonalPieces & friendlyBoard;
	while (orthSliders != 0ULL) {
		startSquare = BitBoardUtility::popLSB(orthSliders);
			isPinned = checkRayBB & (1ULL << startSquare);

			
			if (isPinned && abs(friendlyKingSquare - startSquare) % 7 == 0) {
				continue;
			} else if (isPinned && abs(friendlyKingSquare - startSquare) % 9 == 0) {
				continue;
			}

			uint64_t blockers = board->allPieces & preComp.getBlockerOrthogonalMask(startSquare);
			uint64_t moveBitboard = preComp.getOrthMovementBoard(startSquare, blockers);
			
			moveBitboard &= ~(friendlyBoard);
			if (isCheck) {
				moveBitboard &= checkBB;
			}
			if (isPinned) {
				moveBitboard &= checkRayBB;
			}
			if (generateCaptures) {
				moveBitboard &= enemyBoard;
			}


			if (isPinned && friendlyKingSquare / 8 == startSquare / 8) {
				moveBitboard &= preComp.getRowMask(startSquare);
			}
			if (isPinned && abs(friendlyKingSquare - startSquare) % 8 == 0) {
				moveBitboard &= preComp.getColMask(startSquare);
			}

			while (moveBitboard != 0ULL) {
				targetSquare = BitBoardUtility::popLSB(moveBitboard);

				moves[currentMoves++] = Move(startSquare, targetSquare);
			}

	}
}

void MoveGen::generateDiagonalMoves(Move moves[]) {
	int startSquare;
	int targetSquare;
	bool isPinned;
	uint64_t diagSliders = board->diagonalPieces & friendlyBoard;
	while (diagSliders != 0ULL) {
		startSquare = BitBoardUtility::popLSB(diagSliders);

			isPinned = checkRayBB & (1ULL << startSquare);

			if (isPinned && friendlyKingSquare / 8 == startSquare / 8) {
				continue;
			} else if (isPinned && abs(friendlyKingSquare - startSquare) % 8 == 0) {
				continue;
			}

			uint64_t blockers = board->allPieces & preComp.getBlockerDiagonalMask(startSquare);
			uint64_t moveBitboard = preComp.getDiagMovementBoard(startSquare, blockers);
			moveBitboard &= ~(friendlyBoard);

			if (isCheck) {
				moveBitboard &= checkBB;
			}
			if (isPinned) {
				moveBitboard &= checkRayBB;
			}
			if (generateCaptures) {
				moveBitboard &= enemyBoard;
			}

			if (isPinned && abs(friendlyKingSquare - startSquare) % 7 == 0) {
				moveBitboard &= preComp.getForwardMask(startSquare);
			} else if (isPinned && abs(friendlyKingSquare - startSquare) % 9 == 0) {
				moveBitboard &= preComp.getBackwardMask(startSquare);
			}

			while (moveBitboard != 0ULL) {
				targetSquare = BitBoardUtility::popLSB(moveBitboard);

				moves[currentMoves++] = Move(startSquare, targetSquare);
			}

	}
}

void MoveGen::generateKnightMoves(Move moves[]) {
	int startSquare;
	int targetSquare;
	bool isPinned;
	uint64_t knights = board->knights & friendlyBoard;
	while (knights != 0ULL) {
		startSquare = BitBoardUtility::popLSB(knights);
		isPinned = checkRayBB & (1ULL << startSquare);
		if (isPinned) continue;

		uint64_t moveBitboard = preComp.getKnightBoard(startSquare);
		moveBitboard &= ~(friendlyBoard);
		if (isCheck)
			moveBitboard &= checkBB;
		if (generateCaptures)
			moveBitboard &= enemyBoard;

		while (moveBitboard != 0) {
			targetSquare = BitBoardUtility::popLSB(moveBitboard);

			moves[currentMoves++] = Move(startSquare, targetSquare);
		}
	}
}

void MoveGen::generateKingMoves(Move moves[]) {
	int targetSquare;
	const int castleMask = whiteTurn ? BoardState::whiteCastleMask : BoardState::blackCastleMask;
	const int colorIndex = whiteTurn ? board->whiteIndex : board->blackIndex;
	if (friendlyKingSquare == startingKingSquares[colorIndex] && castlingRights & castleMask && !isCheck) 
		castlingMoves(moves, friendlyKingSquare);

	uint64_t moveBitboard = preComp.getKingMoves(friendlyKingSquare);
	moveBitboard &= ~(opponentAttacks);
	moveBitboard &= ~(friendlyBoard);

	while (moveBitboard != 0ULL) {
		targetSquare = BitBoardUtility::popLSB(moveBitboard);

		moves[currentMoves++] = Move(friendlyKingSquare, targetSquare);
	}
}

void MoveGen::generatePawnMoves(Move moves[]) {
	int direction = whiteTurn ? whitePawnDirection : blackPawnDirection;
	//const int* attackDirection = whiteTurn ? whitePawnAttackDirections : blackPawnAttackDirections;
	const int startRow = whiteTurn ? 6 : 1;
	const int promotionRow = whiteTurn ? 1 : 6;
	bool isPinned;
	int startSquare;
	int targetSquare;
	int kingDir;

	uint64_t pawns = board->pawns & friendlyBoard;

	while (pawns != 0ULL) {
		startSquare = BitBoardUtility::popLSB(pawns);
		kingDir = abs(friendlyKingSquare - startSquare);
		isPinned = checkRayBB & (1ULL << startSquare);

		if (isPinned && friendlyKingSquare / 8 == startSquare / 8) continue;

		uint64_t moveBitboard = (1ULL << (startSquare + direction)) & ~board->allPieces;
		if (moveBitboard != 0ULL && startSquare / 8 == startRow) {
			moveBitboard |= (1ULL << (startSquare + 2 * direction)) & ~board->allPieces;
		}

		if (generateCaptures)
			moveBitboard = 0ULL;
		if (isPinned && friendlyKingSquare % 8 != startSquare % 8)
			moveBitboard = 0ULL;
		
		uint64_t attackBitboard = preComp.getPawnAttackMoves(startSquare, currentColor);
		if (isPinned && friendlyKingSquare % 8 == startSquare % 8) {
			attackBitboard = 0ULL;
		} else if (isPinned && kingDir % 9 == 0) {
			attackBitboard &= preComp.getBackwardMask(startSquare);
		} else if (isPinned && kingDir % 7 == 0) {
			attackBitboard &= preComp.getForwardMask(startSquare);
		}
		// En Passant
		if (enPassantSquare != -1 && (attackBitboard & 1ULL << enPassantSquare)) {
			enPassantMoves(moves, enPassantSquare, startSquare);
		}
		attackBitboard &= enemyBoard;
		moveBitboard |= attackBitboard;
		if (isCheck)
			moveBitboard &= checkBB;

		while (moveBitboard != 0ULL) {
			targetSquare = BitBoardUtility::popLSB(moveBitboard);
			if (startSquare / 8 == promotionRow) {
				for (int i = 0; i < 4; i++) {
					moves[currentMoves++] = Move(startSquare, targetSquare, promotionFlags[i]);
				}
			} else if (abs(targetSquare - startSquare) < 10) {
				moves[currentMoves++] = Move(startSquare, targetSquare);
			} else {
				moves[currentMoves++] = Move(startSquare, targetSquare, Move::pawnTwoSquaresFlag);
			}
		}
	}
}

void MoveGen::enPassantMoves(Move moves[], int targetSquare, int startSquare) {
	const int enemyPawnOffset = whiteTurn ? 8 : -8;
	const int enemyPawnSquare = targetSquare + enemyPawnOffset;

	if (isCheck && !(checkBB & 1ULL << targetSquare) && !(checkBB & 1ULL << enemyPawnSquare)) return;

	if (enPassantPin(startSquare, enemyPawnSquare)) return;

	moves[currentMoves++] = Move(startSquare, targetSquare, Move::enPassantCaptureFlag);
}

bool MoveGen::enPassantPin(int friendlyPawnSquare, int enemyPawnSquare) {
	// If king and pawn on different rows, no pin possible
	if (friendlyKingSquare / 8 != friendlyPawnSquare / 8) return false;

	// Create blocker board
	uint64_t blockers = preComp.getBlockerOrthogonalMask(friendlyKingSquare) & board->allPieces;
	// After en passant pawns will be gone
	blockers &= ~(1ULL << friendlyPawnSquare) & ~(1ULL << enemyPawnSquare);
	// Block column
	blockers |= preComp.getColMask(friendlyKingSquare) & ~(1ULL << friendlyKingSquare);
	blockers &= ~preComp.getRowMask(0);
	blockers &= ~preComp.getRowMask(63);
	// Get movement board
	uint64_t movementBitboard = preComp.getOrthMovementBoard(friendlyKingSquare, blockers);
	// Restrict movement to row and enemy orthogonal pieces
	movementBitboard &= board->orthogonalPieces & enemyBoard & preComp.getRowMask(friendlyKingSquare);
	// If movement board != 0ULL, there is a pin
	return movementBitboard != 0ULL;
}

void MoveGen::castlingMoves(Move moves[], int startSquare) {
	const int shortMask = whiteTurn ? BoardState::whiteShortCastleMask : BoardState::blackShortCastleMask;
	const int longMask = whiteTurn ? BoardState::whiteLongCastleMask : BoardState::blackLongCastleMask;
	const int friendlyRook = whiteTurn ? Piece::whiteRook : Piece::blackRook;
	const int travellingShortSquare = startSquare + 1;
	const int travellingLongSquare = startSquare - 1;
	const bool shortAttacked = opponentAttacks & (1ULL << travellingShortSquare);
	const bool longAttacked = opponentAttacks & (1ULL << travellingLongSquare);

	if (!(opponentAttacks & (1ULL << shortCastleKingSquares[colorIndex])) && !shortAttacked && castlingRights & shortMask && squares[shortCastleRookSquares[colorIndex]] == friendlyRook && squares[startSquare + 1] == Piece::empty && squares[startSquare + 2] == Piece::empty) {
		moves[currentMoves++] = Move(startingKingSquares[colorIndex], shortCastleKingSquares[colorIndex], Move::castleFlag);
	}
	if (!(opponentAttacks & (1ULL << longCastleKingSquares[colorIndex])) && !longAttacked && castlingRights & longMask && squares[longCastleRookSquares[colorIndex]] == friendlyRook && squares[startSquare - 1] == Piece::empty && squares[startSquare - 2] == Piece::empty && squares[startSquare - 3] == Piece::empty) {
		moves[currentMoves++] = Move(startingKingSquares[colorIndex], longCastleKingSquares[colorIndex], Move::castleFlag);
	}
}

void MoveGen::generateSlideAttackData() {
	int startSquare;
	uint64_t orthSliders = board->orthogonalPieces & enemyBoard;
	uint64_t diagSliders = board->diagonalPieces & enemyBoard;

	while (orthSliders != 0ULL) {
			startSquare = BitBoardUtility::popLSB(orthSliders);

			uint64_t blockers = board->allPieces & preComp.getBlockerOrthogonalMask(startSquare);
			BitBoardUtility::deleteBit(blockers, friendlyKingSquare);
			uint64_t moveBitboard = preComp.getOrthMovementBoard(startSquare, blockers);

			opponentAttacks |= moveBitboard;
	}

	while (diagSliders != 0ULL) {			
		startSquare = BitBoardUtility::popLSB(diagSliders);
			
			uint64_t blockers = board->allPieces & preComp.getBlockerDiagonalMask(startSquare);
			BitBoardUtility::deleteBit(blockers, friendlyKingSquare);
			uint64_t moveBitboard = preComp.getDiagMovementBoard(startSquare, blockers);

			opponentAttacks |= moveBitboard;
	}

}

void MoveGen::generateKnightAttackData() {
	int startSquare;
	uint64_t knights = board->knights & enemyBoard;
	while (knights != 0ULL) {
		startSquare = BitBoardUtility::popLSB(knights);
		opponentAttacks |= preComp.getKnightBoard(startSquare);
	}
}

void MoveGen::generatePawnAttackData() {
	int startSquare;
	uint64_t pawns = board->pawns & enemyBoard;

	while (pawns != 0ULL) {
		startSquare = BitBoardUtility::popLSB(pawns);
		uint64_t moveBitboard = preComp.getPawnAttackMoves(startSquare, opposingColor);
		opponentAttacks |= moveBitboard;
	}
}
void MoveGen::generateKingAttackData() {
	const int startSquare = enemyKingSquare;
	uint64_t moveBitboard = preComp.getKingMoves(startSquare);
	opponentAttacks |= moveBitboard;
}

void MoveGen::generateAttackData() {
	generateSlideAttackData();
	generateKingAttackData();
	generateKnightAttackData();
	generatePawnAttackData();
}

void MoveGen::generateCheckData() {
	generateAttackData();

	int targetSquare;
	uint64_t temp;
	uint64_t enemyBlockers;

	uint64_t orthogonalBlockers = board->allPieces & preComp.getBlockerOrthogonalMask(friendlyKingSquare);
	uint64_t diagonalBlockers = board->allPieces & preComp.getBlockerDiagonalMask(friendlyKingSquare);

	temp = preComp.getOrthMovementBoard(friendlyKingSquare, orthogonalBlockers);
	uint64_t checkBoard = temp;
	enemyBlockers = temp & enemyBoard & board->orthogonalPieces;
	temp = preComp.getDiagMovementBoard(friendlyKingSquare, diagonalBlockers);
	checkBoard |= temp;
	enemyBlockers |= temp & enemyBoard & board->diagonalPieces;

	while (enemyBlockers != 0ULL) {
		targetSquare = BitBoardUtility::popLSB(enemyBlockers);
		temp = preComp.getDirectionMask(friendlyKingSquare, targetSquare);
		checkBB |= temp & checkBoard;

		doubleCheck = isCheck;
		isCheck = true;
		if (doubleCheck)
			break;
	}
	if (board->state->zobristHash == 0ULL) {
		BitBoardUtility::printBB(orthogonalBlockers);
		BitBoardUtility::printBB(diagonalBlockers);
	}
	orthogonalBlockers &= ~(checkBoard & friendlyBoard);
	diagonalBlockers &= ~(checkBoard & friendlyBoard);
	if (board->state->zobristHash == 0ULL) {
		BitBoardUtility::printBB(orthogonalBlockers);
		BitBoardUtility::printBB(diagonalBlockers);
	}
	temp = preComp.getOrthMovementBoard(friendlyKingSquare, orthogonalBlockers);
	checkBoard = temp;
	enemyBlockers = temp & enemyBoard & board->orthogonalPieces;
	temp = preComp.getDiagMovementBoard(friendlyKingSquare, diagonalBlockers);
	checkBoard |= temp;
	enemyBlockers |= temp & enemyBoard & board->diagonalPieces;

	while (enemyBlockers != 0ULL) {
		targetSquare = BitBoardUtility::popLSB(enemyBlockers);
		temp = preComp.getDirectionMask(friendlyKingSquare, targetSquare);
		checkRayBB |= temp & checkBoard;
	}

	uint64_t enemyKnights = board->knights & enemyBoard;
	uint64_t knightMoveBitboard = preComp.getKnightBoard(friendlyKingSquare);
	knightMoveBitboard &= enemyKnights;

	checkBB |= knightMoveBitboard;
	
	while (knightMoveBitboard != 0) {
		BitBoardUtility::popLSB(knightMoveBitboard);
		doubleCheck = isCheck;
		isCheck = true;
		if (doubleCheck)
			break;
	}

	uint64_t enemyPawns = board->pawns & enemyBoard;
	uint64_t pawnMoveBitboard = preComp.getPawnAttackMoves(friendlyKingSquare, currentColor);
	pawnMoveBitboard &= enemyPawns;

	checkBB |= pawnMoveBitboard;

	while (pawnMoveBitboard != 0) {
		BitBoardUtility::popLSB(pawnMoveBitboard);
		doubleCheck = isCheck;
		isCheck = true;
		if (doubleCheck)
			break;
	}
}

void PerftResults::reset() {
	captures = 0;
	enPassants = 0;
	castles = 0;
	promotions = 0;
	checks = 0;
	discoveryChecks = 0;
	doubleChecks = 0;
	checkmates = 0;
	stalemates = 0;
}

std::ostream& operator<<(std::ostream& os, const PerftResults& res) {
	os << "Captures: " << res.captures << std::endl;
	os << "En Passants: " << res.enPassants << std::endl;
	os << "Castles: " << res.castles << std::endl;
	os << "Promotions: " << res.promotions << std::endl;
	os << "Checks: " << res.checks << std::endl;
	os << "Discovery Checks: " << res.discoveryChecks << std::endl;
	os << "DoubleChecks: " << res.doubleChecks << std::endl;
	os << "Checkmates: " << res.checkmates << std::endl;
	os << "Stalemates: " << res.stalemates << std::endl;
	return os;
}

void MoveGen::updateResults(Move moves[]) {
	for (int i = 0; i < currentMoves; i++) {
		if (squares[moves[i].targetSquare] != Piece::empty) {
			perftRes.captures++;
			continue;
		}
		if (moves[i].flag == Move::enPassantCaptureFlag) {
			perftRes.enPassants++;
			continue;
		}
		if (moves[i].flag == Move::castleFlag) {
			perftRes.castles++;
			continue;
		}
		if (moves[i].flag > Move::castleFlag) {
			perftRes.promotions++;
			continue;
		}
	}
	if (isCheck) {
		perftRes.checks++;
	}
	if (doubleCheck) {
		perftRes.doubleChecks++;
	}
	if (currentMoves == 0) {
		if (isCheck) perftRes.checkmates++;
		else perftRes.stalemates++;
	}
}