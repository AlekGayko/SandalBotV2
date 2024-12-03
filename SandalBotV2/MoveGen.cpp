#include "MoveGen.h"

#include <iostream>
#include <limits>
#include <stdexcept>

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
	const int* attackDirection = whiteTurn ? whitePawnAttackDirections : blackPawnAttackDirections;
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

		for (int dir = 0; dir < 2; dir++) {
			if (isPinned && kingDir % abs(attackDirection[dir]) != 0) continue;
			//if (pinDir && pinDir != abs(attackDirection[i])) continue;
			if (preComp.directionDistances[startSquare].direction[3 - dir * 2] <= 1) continue;
			targetSquare = attackDirection[dir] + startSquare;
			if (isPinned && !(checkRayBB & (1ULL << targetSquare))) continue;
			if (targetSquare == enPassantSquare && squares[targetSquare] == Piece::empty) {
				enPassantMoves(moves, targetSquare, startSquare);
			}

			if (isCheck && !(checkBB & 1ULL << targetSquare)) continue;
			if (!Piece::isColor(squares[targetSquare], opposingColor)) {
				continue;
			}
			if (startSquare / 8 == promotionRow) {
				for (int i = 0; i < 4; i++) {
					moves[currentMoves++] = Move(startSquare, targetSquare, promotionFlags[i]);
				}
				continue;
			}
			moves[currentMoves++] = Move(startSquare, targetSquare);
		}

		// If generating captures, moving forward does not capture
		if (generateCaptures) continue;
		// If pinned, and not on king's file, pinned diagonally or horizontally
		if (isPinned && kingDir % 8 != 0)
			continue;

		if (squares[startSquare + direction] != Piece::empty)
			continue;

		// If in check, and square not in check ray
		if (!isCheck || (isCheck && checkBB & 1ULL << (startSquare + direction))) {
			if (startSquare / 8 == promotionRow) {
				for (int i = 0; i < 4; i++) {
					moves[currentMoves++] = Move(startSquare, startSquare + direction, promotionFlags[i]);
				}

			} else {
				moves[currentMoves++] = Move(startSquare, direction + startSquare);
			}
		}
		if (!(startSquare / 8 == startRow && squares[direction * 2 + startSquare] == Piece::empty)) {
			continue;
		}
		// If in check, and square not in check ray
		if (!isCheck || (isCheck && checkBB & 1ULL << (startSquare + 2 * direction))) {
			moves[currentMoves++] = Move(startSquare, direction * 2 + startSquare, Move::pawnTwoSquaresFlag);
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
		uint64_t moveBitboard = preComp.getPawnAttackMoves(startSquare, !whiteTurn ? Piece::white : Piece::black);
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
	int direction;
	int distance;
	/*
	uint64_t orthogonalBlockers = board->allPieces & preComp.getBlockerOrthogonalMask(friendlyKingSquare);
	uint64_t diagonalBlockers = board->allPieces & preComp.getBlockerDiagonalMask(friendlyKingSquare);

	checkBB = preComp.getOrthMovementBoard(friendlyKingSquare, orthogonalBlockers);
	checkBB |= preComp.getDiagMovementBoard(friendlyKingSquare, diagonalBlockers);

	orthogonalBlockers &= ~(checkBB & friendlyBoard);
	diagonalBlockers &= ~(checkBB & friendlyBoard);
	checkRayBB = preComp.getOrthMovementBoard(friendlyKingSquare, orthogonalBlockers);
	checkRayBB = preComp.getDiagMovementBoard(friendlyKingSquare, diagonalBlockers);

	checkBB &= ~(friendlyBoard);

	while (checkBB != 0) {
		BitBoardUtility::popLSB(checkBB);
		doubleCheck = isCheck;
		isCheck = true;
		if (doubleCheck)
			break;
	}
	*/
	for (int dirIndex = startOrthogonal; dirIndex < endDiagonal; dirIndex++) {
		bool isOrth = dirIndex < 4;
		bool foundFriendly = false;
		uint64_t dirBB = 0ULL;
		uint64_t dirRayBB = 0ULL;
		// bool dirCheck = false;
		direction = slideDirections[dirIndex];
		distance = preComp.directionDistances[friendlyKingSquare].direction[dirIndex];
		for (int it = 1; it < distance; it++) {
			targetSquare = direction * it + friendlyKingSquare;
			if (squares[targetSquare] == Piece::empty) {
				dirBB |= 1ULL << targetSquare;
				continue;
			}
			bool dangerPiece = isOrth ? Piece::isOrthogonal(squares[targetSquare]) : Piece::isDiagonal(squares[targetSquare]);
			// If targetsquare contains opposing sliding piece break
			if (Piece::isColor(squares[targetSquare], opposingColor)) {
				if (!dangerPiece) break;
				dirBB |= 1ULL << targetSquare;
				if (foundFriendly) checkRayBB |= dirBB;
				else {
					checkBB |= dirBB;
					doubleCheck = isCheck;
					isCheck = true;
				}
				break;
			} // If isnt sliding opposite piece
			else {
				if (foundFriendly) break;
				dirBB |= 1ULL << targetSquare;
				foundFriendly = true;
				continue;
			}
		}
		// cannot be checked by more than two pieces
		if (doubleCheck) break;
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
	uint64_t pawnMoveBitboard = preComp.getPawnAttackMoves(friendlyKingSquare, whiteTurn ? Piece::white : Piece::black);
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