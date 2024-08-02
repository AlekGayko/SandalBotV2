#include "MoveGen.h"

#include <iostream>
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
	//cout << "ischeck: " << isCheck << ", doublecheck: " << doubleCheck << ", checkbb: " << checkBB << ", raybb: " << checkRayBB << endl;
	generateCheckData();
	//cout << "ischeck: " << isCheck << ", doublecheck: " << doubleCheck << ", checkbb: " << checkBB << ", raybb: " << checkRayBB << endl;
	generateKingMoves(moves);
	//cout << "moves after king: " << currentMoves << endl;
	if (doubleCheck) return currentMoves;
	generatePawnMoves(moves);
	//cout << "moves after pawns: " << currentMoves << endl;
	generateKnightMoves(moves);
	//cout << "moves after knight: " << currentMoves << endl;
	generateOrthogonalMoves(moves);
	//cout << "moves after orthog: " << currentMoves << endl;
	generateDiagonalMoves(moves);
	//cout << "moves after diag: " << currentMoves << endl;

	//updateResults(moves);
	/*
	for (int i = 0; i < currentMoves; i++) {
		if (Piece::isType(squares[moves[i].targetSquare], Piece::king)) {
			BitBoardUtility::printBB(opponentAttacks);
			BitBoardUtility::printBB(checkBB);
			BitBoardUtility::printBB(checkRayBB);
		}
	}
	*/
	//BitBoardUtility::printBB(opponentAttacks);
	//BitBoardUtility::printBB(checkBB);
	//BitBoardUtility::printBB(checkRayBB);
	return currentMoves;
}

void MoveGen::initVariables(bool capturesOnly) {
	squares = board->squares;
	isCheck = false;// board->state->check;
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
	friendlyPieceLists = board->pieceLists[colorIndex];
	enemyPieceLists = board->pieceLists[!colorIndex];

	orthogonalSliders[0] = friendlyPieceLists[Piece::rook];
	orthogonalSliders[1] = friendlyPieceLists[Piece::queen];

	diagonalSliders[0] = friendlyPieceLists[Piece::bishop];
	diagonalSliders[1] = friendlyPieceLists[Piece::queen];

	enemyOrthogonalSliders[0] = enemyPieceLists[Piece::rook];
	enemyOrthogonalSliders[1] = enemyPieceLists[Piece::queen];

	enemyDiagonalSliders[0] = enemyPieceLists[Piece::bishop];
	enemyDiagonalSliders[1] = enemyPieceLists[Piece::queen];


	pawns = friendlyPieceLists[Piece::pawn];
	friendlyKingSquare = friendlyPieceLists[Piece::king][0];

	checkBB = 0ULL;
	checkRayBB = 0ULL;
	opponentAttacks = 0ULL;

	generateCaptures = capturesOnly;
}

void MoveGen::generateSlideMoves(Move moves[]) {

}

void MoveGen::generateOrthogonalMoves(Move moves[]) {
	int startSquare;
	int direction;
	int distance;
	int targetSquare;
	int numOrthoSliders;
	bool isPinned;
	unsigned int pinDir = 0;

	for (int type = 0; type < 2; type++) {
		numOrthoSliders = orthogonalSliders[type].numPieces;
		for (int it = 0; it < numOrthoSliders; it++) {
			startSquare = orthogonalSliders[type][it];
			isPinned = checkRayBB & (1ULL << startSquare);
			pinDir = 0;
			if (isPinned && friendlyKingSquare / 8 != startSquare / 8 && (friendlyKingSquare - startSquare) % 8 != 0) continue;
			if (isPinned && abs(startSquare - friendlyKingSquare) <= 9) {
				int kingDir = abs(friendlyKingSquare - startSquare);
				bool isDiag = false;
				for (int dir = startOrthogonal; dir < endDiagonal; dir++) {
					if (abs(slideDirections[dir]) == kingDir) {
						pinDir = kingDir;
						if (dir >= startDiagonal) isDiag = true;
						break;
					}
				}
				if (isDiag) continue;
			}
			for (int dirIndex = startOrthogonal; dirIndex < endOrthogonal; dirIndex++) {
				direction = slideDirections[dirIndex];
				distance = preComp.directionDistances[startSquare].direction[dirIndex];
				if (pinDir && pinDir != abs(direction)) continue;
				for (int it = 1; it < distance; it++) {
					int targetSquare = direction * it + startSquare;
					// If targetsquare contains friendly piece break
					if (Piece::isColor(squares[targetSquare], currentColor)) break;
					if (isPinned && !(checkRayBB & (1ULL << targetSquare))) break;
					if (isCheck && !(checkBB & 1ULL << targetSquare)) {
						if (squares[targetSquare] != Piece::empty) {
							break;
						}
						continue;
					}
					if (generateCaptures && squares[targetSquare] == Piece::empty) continue;
					// Add move
					moves[currentMoves++] = Move(startSquare, targetSquare);
					// If targetsquare contains opposing piece break
					if (squares[targetSquare] != Piece::empty)	break;
				}
			}
		}
	}
}

void MoveGen::generateDiagonalMoves(Move moves[]) {
	int startSquare;
	int direction;
	int distance;
	int targetSquare;
	int numDiagSliders;
	bool isPinned;
	unsigned int pinDir = 0;

	for (int type = 0; type < 2; type++) {
		numDiagSliders = diagonalSliders[type].numPieces;
		for (int it = 0; it < numDiagSliders; it++) {
			const int startSquare = diagonalSliders[type][it];
			isPinned = checkRayBB & (1ULL << startSquare);
			pinDir = 0;
			if (isPinned && friendlyKingSquare / 8 == startSquare / 8) continue;
			if (isPinned && (friendlyKingSquare - startSquare) % 8 == 0) continue;
			if (isPinned && abs(startSquare - friendlyKingSquare) <= 9) {
				int kingDir = abs(friendlyKingSquare - startSquare);
				for (int dir = startDiagonal; dir < endDiagonal; dir++) {
					if (abs(slideDirections[dir]) == kingDir) {
						pinDir = kingDir;
						break;
					}
				}
				if (!pinDir) continue;
			}
			for (int dirIndex = startDiagonal; dirIndex < endDiagonal; dirIndex++) {
				direction = slideDirections[dirIndex];
				distance = preComp.directionDistances[startSquare].direction[dirIndex];
				if (pinDir && pinDir != abs(direction)) continue;
				for (int it = 1; it < distance; it++) {
					targetSquare = direction * it + startSquare;
					// If targetsquare contains friendly piece break
					if (Piece::isColor(squares[targetSquare], currentColor)) break;
					if (isPinned && !(checkRayBB & (1ULL << targetSquare))) break;
					if (isCheck && !(checkBB & 1ULL << targetSquare)) {
						if (squares[targetSquare] != Piece::empty) {
							break;
						}
						continue;
					}
					if (generateCaptures && squares[targetSquare] == Piece::empty) continue;
					// Add move
					moves[currentMoves++] = Move(startSquare, targetSquare);
					// If targetsquare contains opposing piece break
					if (squares[targetSquare] != Piece::empty) break;
				}
			}
		}
	}
}

void MoveGen::generateKnightMoves(Move moves[]) {
	int numKnights = friendlyPieceLists[Piece::knight].numPieces;
	bool isPinned;
	for (int it = 0; it < numKnights; it++) {
		const int startSquare = friendlyPieceLists[Piece::knight][it];
		isPinned = checkRayBB & (1ULL << startSquare);
		if (isPinned) continue;
		for (int dirIndex = 0; dirIndex < 8; dirIndex++) {
			if (!preComp.directionDistances[startSquare].knightSquares[dirIndex]) continue;
			const int targetSquare = knightDirections[dirIndex] + startSquare;
			if (Piece::isColor(squares[targetSquare], currentColor)) continue;
			if (isCheck && !(checkBB & 1ULL << targetSquare)) continue;
			if (generateCaptures && squares[targetSquare] == Piece::empty) continue;
			moves[currentMoves++] = Move(startSquare, targetSquare);
		}
	}
}

void MoveGen::generateKingMoves(Move moves[]) {
	const int startSquare = friendlyPieceLists[Piece::king][0];
	const int castleMask = whiteTurn ? BoardState::whiteCastleMask : BoardState::blackCastleMask;
	const int colorIndex = whiteTurn ? board->whiteIndex : board->blackIndex;
	if (startSquare == startingKingSquares[colorIndex] && castlingRights & castleMask && !isCheck) castlingMoves(moves, startSquare);

	for (int dirIndex = 0; dirIndex < 8; dirIndex++) {
		if (preComp.directionDistances[startSquare].direction[dirIndex] <= 1) continue;
		const int targetSquare = slideDirections[dirIndex] + startSquare;
		if (Piece::isColor(squares[targetSquare], currentColor)) continue;
		if (opponentAttacks & (1ULL << targetSquare)) continue;
		if (generateCaptures && squares[targetSquare] == Piece::empty) continue;
		moves[currentMoves++] = Move(startSquare, targetSquare);
	}
}

void MoveGen::generatePawnMoves(Move moves[]) {
	int numPawns = pawns.numPieces;
	int direction = whiteTurn ? whitePawnDirection : blackPawnDirection;
	const int* attackDirection = whiteTurn ? whitePawnAttackDirections : blackPawnAttackDirections;
	const int startRow = whiteTurn ? 6 : 1;
	const int promotionRow = whiteTurn ? 1 : 6;
	bool isPinned;
	int pinDir = 0;

	for (int it = 0; it < numPawns; it++) {
		const int startSquare = pawns[it];
		int kingDir = abs(friendlyKingSquare - startSquare);
		bool skipDiag = false;
		bool skipOrth = false;
		isPinned = checkRayBB & (1ULL << startSquare);
		pinDir = 0;

		if (isPinned && kingDir % 8 == 0) skipDiag = true;
		if (isPinned && friendlyKingSquare / 8 == startSquare / 8) continue;

		if (isPinned && abs(startSquare - friendlyKingSquare) <= 9) {
			for (int dir = startDiagonal; dir < endDiagonal; dir++) {
				if (abs(slideDirections[dir]) == kingDir) {
					pinDir = kingDir;
					skipOrth = true;
					break;
				}
			}
		}
		//if (startSquare / 8 == 7) throw std::out_of_range("Pawn cannot exist on last rank.");
		for (int i = 0; i < 2; i++) {
			if (skipDiag) break;
			if (pinDir && pinDir != abs(attackDirection[i])) continue;
			if (preComp.directionDistances[startSquare].direction[3 - i * 2] <= 1) continue;
			int targetSquare = attackDirection[i] + startSquare;
			if (isPinned && !(checkRayBB & (1ULL << targetSquare))) continue;
			if (targetSquare == enPassantSquare && squares[targetSquare] == Piece::empty) {
				enPassantMoves(moves, targetSquare, startSquare);
			}

			if (isCheck && !(checkBB & 1ULL << targetSquare)) continue;
			if (!Piece::isColor(squares[targetSquare], opposingColor)) {
				continue;
			}
			if (startSquare / 8 == promotionRow) {
				promotionMoves(moves, targetSquare, startSquare);
				continue;
			}
			moves[currentMoves++] = Move(startSquare, targetSquare);
		}
		if (generateCaptures) continue;
		if (skipOrth) continue;
		bool stillPinned, blockingCheck;
		stillPinned = isPinned && !(checkRayBB & (1ULL << (direction + startSquare)));
		blockingCheck = isCheck && !(checkBB & 1ULL << direction + startSquare);

		if (squares[direction + startSquare] == Piece::empty) {
			if (!stillPinned && !blockingCheck) {
				if (startSquare / 8 == promotionRow) {
					promotionMoves(moves, direction + startSquare, startSquare);
					continue;
				}
				moves[currentMoves++] = Move(startSquare, direction + startSquare);
			}
		} else continue;
		if (isPinned && stillPinned) continue;
		stillPinned = isPinned && !(checkRayBB & (1ULL << (direction * 2 + startSquare)));
		blockingCheck = isCheck && !(checkBB & 1ULL << direction * 2 + startSquare);

		if (startSquare / 8 == startRow && squares[direction * 2 + startSquare] == Piece::empty) {
			if (!stillPinned && !blockingCheck) {
				moves[currentMoves++] = Move(startSquare, direction * 2 + startSquare, Move::pawnTwoSquaresFlag);
			}
		}
	}
}

void MoveGen::enPassantMoves(Move moves[], int targetSquare, int startSquare) {
	const int enemyPawnOffset = whiteTurn ? 8 : -8;
	const int enemyPawnSquare = targetSquare + enemyPawnOffset;

	if (isCheck && !(checkBB & 1ULL << targetSquare) && !(checkBB & 1ULL << enemyPawnSquare)) return;

	if (enPassantPin(startSquare, enemyPawnSquare)) return;
	//cout << "enpassanting" << endl;
	moves[currentMoves++] = Move(startSquare, targetSquare, Move::enPassantCaptureFlag);
}

bool MoveGen::enPassantPin(int friendlyPawnSquare, int enemyPawnSquare) {
	if (friendlyKingSquare / 8 != friendlyPawnSquare / 8) return false;
	const int directionIndex = friendlyKingSquare > enemyPawnSquare ? 3 : 1;

	const int direction = slideDirections[directionIndex];
	int distance = preComp.directionDistances[friendlyKingSquare].direction[directionIndex];
	for (int it = 1; it < distance; it++) {
		int targetSquare = direction * it + friendlyKingSquare;
		if (targetSquare == friendlyPawnSquare || targetSquare == enemyPawnSquare) continue;
		// If targetsquare contains friendly piece break
		if (Piece::isColor(squares[targetSquare], currentColor)) break;
		
		if (Piece::isOrthogonal(squares[targetSquare])) return true;

		// If targetsquare contains opposing piece break
		if (squares[targetSquare] != Piece::empty)	break;
	}

	return false;
}

void MoveGen::promotionMoves(Move moves[], int targetSquare, int startSquare) {
	//cout << "promoting" << endl;
	for (int i = 0; i < 4; i++) {
		moves[currentMoves++] = Move(startSquare, targetSquare, promotionFlags[i]);
		//cout << moves[currentMoves - 1] << endl;
		//cout << board->printBoard() << endl;
	}
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
		//cout << "castling short" << endl;
		moves[currentMoves++] = Move(startingKingSquares[colorIndex], shortCastleKingSquares[colorIndex], Move::castleFlag);
	}
	if (!(opponentAttacks & (1ULL << longCastleKingSquares[colorIndex])) && !longAttacked && castlingRights & longMask && squares[longCastleRookSquares[colorIndex]] == friendlyRook && squares[startSquare - 1] == Piece::empty && squares[startSquare - 2] == Piece::empty && squares[startSquare - 3] == Piece::empty) {
		//cout << "castling long" << endl;
		moves[currentMoves++] = Move(startingKingSquares[colorIndex], longCastleKingSquares[colorIndex], Move::castleFlag);
	}
}

void MoveGen::generateSlideAttackData() {
	int startSquare;
	int direction;
	int distance;
	int targetSquare;
	int numOrthoSliders;
	int numDiagSliders;

	for (int type = 0; type < 2; type++) {
		numOrthoSliders = enemyOrthogonalSliders[type].numPieces;
		for (int it = 0; it < numOrthoSliders; it++) {
			startSquare = enemyOrthogonalSliders[type][it];
			for (int dirIndex = startOrthogonal; dirIndex < endOrthogonal; dirIndex++) {
				direction = slideDirections[dirIndex];
				distance = preComp.directionDistances[startSquare].direction[dirIndex];
				for (int it = 1; it < distance; it++) {
					int targetSquare = direction * it + startSquare;
					// Add move
					opponentAttacks |= 1ULL << targetSquare;
					// If targetsquare contains opposing piece break
					if (squares[targetSquare] != Piece::empty && squares[targetSquare] != (Piece::king | currentColor)) break;
				}
			}
		}
	}

	for (int type = 0; type < 2; type++) {
		numDiagSliders = enemyDiagonalSliders[type].numPieces;
		for (int it = 0; it < numDiagSliders; it++) {
			const int startSquare = enemyDiagonalSliders[type][it];
			for (int dirIndex = startDiagonal; dirIndex < endDiagonal; dirIndex++) {
				direction = slideDirections[dirIndex];
				distance = preComp.directionDistances[startSquare].direction[dirIndex];
				for (int it = 1; it < distance; it++) {
					targetSquare = direction * it + startSquare;
					// Add move
					opponentAttacks |= 1ULL << targetSquare;
					// If targetsquare contains opposing piece break
					if (squares[targetSquare] != Piece::empty && squares[targetSquare] != (Piece::king | currentColor)) break;
				}
			}
		}
	}

}

void MoveGen::generateKnightAttackData() {
	int numKnights = enemyPieceLists[Piece::knight].numPieces;
	for (int it = 0; it < numKnights; it++) {
		const int startSquare = enemyPieceLists[Piece::knight][it];
		for (int dirIndex = 0; dirIndex < 8; dirIndex++) {
			if (!preComp.directionDistances[startSquare].knightSquares[dirIndex]) continue;
			const int targetSquare = knightDirections[dirIndex] + startSquare;
			opponentAttacks |= 1ULL << targetSquare;

		}
	}
}
void MoveGen::generatePawnAttackData() {
	int numPawns = enemyPieceLists[Piece::pawn].numPieces;
	const int* attackDirection = !whiteTurn ? whitePawnAttackDirections : blackPawnAttackDirections;

	for (int it = 0; it < numPawns; it++) {
		const int startSquare = enemyPieceLists[Piece::pawn][it];
		//if (startSquare / 8 == 7) throw std::out_of_range("Pawn cannot exist on last rank.");
		for (int i = 0; i < 2; i++) {
			if (preComp.directionDistances[startSquare].direction[3 - i * 2] <= 1) continue;
			int targetSquare = attackDirection[i] + startSquare;
			opponentAttacks |= 1ULL << targetSquare;
		}
	}
}
void MoveGen::generateKingAttackData() {
	const int startSquare = enemyPieceLists[Piece::king][0];

	for (int dirIndex = 0; dirIndex < 8; dirIndex++) {
		if (preComp.directionDistances[startSquare].direction[dirIndex] <= 1) continue;
		const int targetSquare = slideDirections[dirIndex] + startSquare;
		opponentAttacks |= 1ULL << targetSquare;
	}
}

void MoveGen::generateAttackData() {
	generateSlideAttackData();
	generateKingAttackData();
	generateKnightAttackData();
	generatePawnAttackData();
}

void MoveGen::generateCheckData() {
	generateAttackData();

	const int kingSquare = friendlyPieceLists[Piece::king][0];
	int targetSquare;
	int direction;
	int distance;
	for (int dirIndex = startOrthogonal; dirIndex < endDiagonal; dirIndex++) {
		bool isOrth = dirIndex < 4;
		bool foundFriendly = false;
		uint64_t dirBB = 0ULL;
		uint64_t dirRayBB = 0ULL;
		// bool dirCheck = false;
		direction = slideDirections[dirIndex];
		distance = preComp.directionDistances[kingSquare].direction[dirIndex];
		for (int it = 1; it < distance; it++) {
			targetSquare = direction * it + kingSquare;
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
					//cout << "kingsquare: " << kingSquare << ", targetsquare: " << targetSquare << ", piece: " << squares[targetSquare] << endl;
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

	const int enemyKnight = whiteTurn ? Piece::blackKnight : Piece::whiteKnight;
	for (int dirIndex = 0; dirIndex < 8; dirIndex++) {
		if (!preComp.directionDistances[kingSquare].knightSquares[dirIndex]) continue;
		targetSquare = knightDirections[dirIndex] + kingSquare;
		if (squares[targetSquare] != enemyKnight) continue;
		checkBB |= 1ULL << targetSquare;
		doubleCheck = isCheck;
		isCheck = true;
	}

	const int enemyPawn = whiteTurn ? Piece::blackPawn : Piece::whitePawn;
	const int* attackDirection = whiteTurn ? whitePawnAttackDirections : blackPawnAttackDirections;
	for (int i = 0; i < 2; i++) {
		if (preComp.directionDistances[kingSquare].direction[3 - i * 2] <= 1) continue;
		targetSquare = attackDirection[i] + kingSquare;
		if (squares[targetSquare] != enemyPawn) continue;
		checkBB |= 1ULL << targetSquare;
		doubleCheck = isCheck;
		isCheck = true;
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