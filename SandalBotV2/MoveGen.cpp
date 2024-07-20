#include "MoveGen.h"

#include <iostream>
#include <stdexcept>

using namespace std;

MoveGen::MoveGen() {

}

MoveGen::MoveGen(Board* board) {
	if (board == nullptr) throw std::invalid_argument("board cannot be nullptr");
	this->board = board;
}

int MoveGen::generateMoves(Move moves[]) {
	initVariables();
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

	updateResults(moves);

	return currentMoves;
}

void MoveGen::initVariables() {
	isCheck = board->state.check;
	doubleCheck = false;
	whiteTurn = board->state.whiteTurn;
	enPassantFile = board->state.enPassantFile;
	castlingRights = board->state.castlingRights;
	fiftyMoveCounter = board->state.fiftyMoveCounter;
	moveCounter = board->state.moveCounter;

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

	checkBB = 0ULL;
	checkRayBB = 0ULL;
	opponentAttacks = 0ULL;
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

	for (int type = 0; type < 2; type++) {
		numOrthoSliders = orthogonalSliders[type].numPieces;
		for (int it = 0; it < numOrthoSliders; it++) {
			startSquare = orthogonalSliders[type][it];
			isPinned = checkRayBB & (0b1 << startSquare);
			for (int dirIndex = startOrthogonal; dirIndex < endOrthogonal; dirIndex++) {
				direction = slideDirections[dirIndex];
				distance = preComp.directionDistances[startSquare].direction[dirIndex];
				for (int it = 1; it < distance; it++) {
					int targetSquare = direction * it + startSquare;
					if (isPinned && !(checkRayBB & (0b1 << targetSquare))) break;
					if (isCheck && !(checkBB & 0b1 << targetSquare)) continue;
					// If targetsquare contains friendly piece break
					if (Piece::isColor(board->squares[targetSquare], currentColor)) break;
					// Add move
					moves[currentMoves++] = Move(startSquare, targetSquare);
					// If targetsquare contains opposing piece break
					if (Piece::isColor(board->squares[targetSquare], opposingColor)) {
						moves[currentMoves - 1].takenPiece = board->squares[targetSquare];
						break;
					}
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

	for (int type = 0; type < 2; type++) {
		numDiagSliders = diagonalSliders[type].numPieces;
		for (int it = 0; it < numDiagSliders; it++) {
			const int startSquare = diagonalSliders[type][it];
			isPinned = checkRayBB & (0b1 << startSquare);
			for (int dirIndex = startDiagonal; dirIndex < endDiagonal; dirIndex++) {
				direction = slideDirections[dirIndex];
				distance = preComp.directionDistances[startSquare].direction[dirIndex];
				for (int it = 1; it < distance; it++) {
					targetSquare = direction * it + startSquare;
					if (isPinned && !(checkRayBB & (0b1 << targetSquare))) break;
					if (isCheck && !(checkBB & 0b1 << targetSquare)) continue;
					// If targetsquare contains friendly piece break
					if (Piece::isColor(board->squares[targetSquare], currentColor)) break;
					// Add move
					moves[currentMoves++] = Move(startSquare, targetSquare);
					// If targetsquare contains opposing piece break
					if (Piece::isColor(board->squares[targetSquare], opposingColor)) {
						moves[currentMoves - 1].takenPiece = board->squares[targetSquare];
						break;
					}
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
		isPinned = checkRayBB & (0b1 << startSquare);
		if (isPinned) continue;
		for (int dirIndex = 0; dirIndex < 8; dirIndex++) {
			if (!preComp.directionDistances[startSquare].knightSquares[dirIndex]) continue;
			const int targetSquare = knightDirections[dirIndex] + startSquare;
			if (isCheck && !(checkBB & 0b1 << targetSquare)) continue;
			if (Piece::isColor(board->squares[targetSquare], currentColor)) continue;
			moves[currentMoves++] = Move(startSquare, targetSquare);

			if (Piece::isColor(board->squares[targetSquare], opposingColor)) moves[currentMoves - 1].takenPiece = board->squares[targetSquare];
		}
	}
}

void MoveGen::generateKingMoves(Move moves[]) {
	const int startSquare = friendlyPieceLists[Piece::king][0];
	const int castleMask = whiteTurn ? BoardState::whiteCastleMask : BoardState::blackCastleMask;
	const int colorIndex = whiteTurn ? board->whiteIndex : board->blackIndex;
	if (startSquare % 8 == startingKingSquares[colorIndex] && castlingRights & castleMask) castlingMoves(moves, startSquare);

	for (int dirIndex = 0; dirIndex < 8; dirIndex++) {
		if (preComp.directionDistances[startSquare].direction[dirIndex] <= 1) continue;
		const int targetSquare = slideDirections[dirIndex] + startSquare;
		if (opponentAttacks & (0b1 << targetSquare)) continue;
		if (Piece::isColor(board->squares[targetSquare], currentColor)) continue;
		moves[currentMoves++] = Move(startSquare, targetSquare);
		if (Piece::isColor(board->squares[targetSquare], opposingColor)) moves[currentMoves - 1].takenPiece = board->squares[targetSquare];
	}
}

void MoveGen::generatePawnMoves(Move moves[]) {
	int numPawns = pawns.numPieces;
	int direction = whiteTurn ? whitePawnDirection : blackPawnDirection;
	const int* attackDirection = whiteTurn ? whitePawnAttackDirections : blackPawnAttackDirections;
	const int startRow = whiteTurn ? 6 : 1;
	const int enPassantRow = whiteTurn ? 3 : 4;
	const int promotionRow = whiteTurn ? 0 : 7;
	bool isPinned;

	for (int it = 0; it < numPawns; it++) {
		const int startSquare = pawns[it];
		isPinned = checkRayBB & (0b1 << startSquare);
		//if (startSquare / 8 == 7) throw std::out_of_range("Pawn cannot exist on last rank.");
		for (int i = 0; i < 2; i++) {
			if (preComp.directionDistances[startSquare].direction[3 - i * 2] <= 1) continue;
			int targetSquare = attackDirection[i] + startSquare;
			if (isPinned && !(checkRayBB & (0b1 << targetSquare))) break;
			if (isCheck && !(checkBB & 0b1 << targetSquare)) continue;
			if (!Piece::isColor(board->squares[targetSquare], opposingColor)) {
				if (targetSquare % 8 == enPassantFile && targetSquare / 8 == enPassantRow && board->squares[targetSquare] == Piece::empty) {
					enPassantMoves(moves, targetSquare, startSquare);
				}
				continue;
			}
			if (startSquare / 8 == promotionRow) {
				promotionMoves(moves, targetSquare, startSquare);
				continue;
			}
			moves[currentMoves++] = Move(startSquare, targetSquare);
			moves[currentMoves - 1].takenPiece = board->squares[targetSquare];
		}
		if (isPinned && !(checkRayBB & (0b1 << (direction + startSquare)))) continue;
		if (isCheck && !(checkBB & 0b1 << direction + startSquare)) continue;
		if (board->squares[direction + startSquare] == Piece::empty) {
			moves[currentMoves++] = Move(startSquare, direction + startSquare);
			if (startSquare / 8 == promotionRow) promotionMoves(moves, direction + startSquare, startSquare);
		} else continue;
		if (startSquare / 8 == startRow && board->squares[direction * 2 + startSquare] == Piece::empty) {
			moves[currentMoves++] = Move(startSquare, direction * 2 + startSquare, Move::pawnTwoSquaresFlag);
		}
	}
}

void MoveGen::enPassantMoves(Move moves[], int targetSquare, int startSquare) {
	cout << "enpassanting" << endl;
	moves[currentMoves++] = Move(startSquare, targetSquare, Move::enPassantCaptureFlag);
}

void MoveGen::promotionMoves(Move moves[], int targetSquare, int startSquare) {
	cout << "promoting" << endl;
	for (int i = 0; i < 4; i++) {
		moves[currentMoves++] = Move(startSquare, targetSquare, promotionFlags[i]);
	}
}

void MoveGen::castlingMoves(Move moves[], int startSquare) {
	const int shortMask = whiteTurn ? BoardState::whiteShortCastleMask : BoardState::blackShortCastleMask;
	const int longMask = whiteTurn ? BoardState::whiteLongCastleMask : BoardState::blackLongCastleMask;
	const int friendlyRook = whiteTurn ? Piece::whiteRook : Piece::blackRook;


	if (castlingRights & shortMask && board->squares[shortCastleRookSquares[colorIndex]] == friendlyRook && board->squares[startSquare + 1] == Piece::empty && board->squares[startSquare + 2] == Piece::empty) {
		cout << "castling" << endl;
		moves[currentMoves++] = Move(startingKingSquares[colorIndex], shortCastleKingSquares[colorIndex], Move::castleFlag);
	}
	if (castlingRights & longMask && board->squares[shortCastleRookSquares[colorIndex]] == friendlyRook && board->squares[startSquare - 1] == Piece::empty && board->squares[startSquare - 2] == Piece::empty && board->squares[startSquare - 3]) {
		cout << "castling" << endl;
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
					// If targetsquare contains friendly piece break
					if (Piece::isColor(board->squares[targetSquare], opposingColor)) break;
					// Add move
					opponentAttacks |= 0b1 << targetSquare;
					// If targetsquare contains opposing piece break
					if (Piece::isColor(board->squares[targetSquare], currentColor)) break;
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
					// If targetsquare contains friendly piece break
					if (Piece::isColor(board->squares[targetSquare], opposingColor)) break;
					// Add move
					opponentAttacks |= 0b1 << targetSquare;
					// If targetsquare contains opposing piece break
					if (Piece::isColor(board->squares[targetSquare], currentColor)) break;
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
			if (Piece::isColor(board->squares[targetSquare], opposingColor)) continue;
			opponentAttacks |= 0b1 << targetSquare;

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
			if (!Piece::isColor(board->squares[targetSquare], currentColor)) continue;
			opponentAttacks |= 0b1 << targetSquare;
		}
	}
}
void MoveGen::generateKingAttackData() {
	const int startSquare = enemyPieceLists[Piece::king][0];

	for (int dirIndex = 0; dirIndex < 8; dirIndex++) {
		if (preComp.directionDistances[startSquare].direction[dirIndex] <= 1) continue;
		const int targetSquare = slideDirections[dirIndex] + startSquare;
		if (Piece::isColor(board->squares[targetSquare], opposingColor)) continue;
		opponentAttacks |= 0b1 << targetSquare;
	}
}

void MoveGen::updateResults(Move moves[]) {
	for (int i = 0; i < currentMoves; i++) {
		if (moves[i].takenPiece != Piece::empty) {
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
		if (isCheck) {
			perftRes.checks++;
			continue;
		}
		if (doubleCheck) {
			perftRes.doubleChecks++;
			continue;
		}
		if (currentMoves == 0) {
			if (isCheck) perftRes.checkmates++;
			else perftRes.stalemates++;
			continue;
		}
	}
}

void MoveGen::generateAttackData() {
	opponentAttacks = 0ULL;
	generateSlideAttackData();
	generateKingAttackData();
	generateKnightAttackData();
	generatePawnAttackData();
}

void MoveGen::generateCheckData() {
	const int kingSquare = friendlyPieceLists[Piece::king][0];
	int targetSquare;
	int direction;
	int distance;
	for (int dirIndex = startOrthogonal; dirIndex < endDiagonal; dirIndex++) {
		bool isOrth = dirIndex < 4;
		bool foundFriendly = false;
		unsigned long long int dirBB = 0ULL;
		unsigned long long int dirRayBB = 0ULL;
		// bool dirCheck = false;
		direction = slideDirections[dirIndex];
		distance = preComp.directionDistances[kingSquare].direction[dirIndex];
		for (int it = 1; it < distance; it++) {
			targetSquare = direction * it + kingSquare;
			if (board->squares[targetSquare] == Piece::empty) {
				dirBB |= 0b1 << targetSquare;
				continue;
			}
			bool dangerPiece = isOrth ? Piece::isOrthogonal(board->squares[targetSquare]) : Piece::isDiagonal(board->squares[targetSquare]);
			// If targetsquare contains opposing sliding piece break
			if (Piece::isColor(board->squares[targetSquare], opposingColor) && dangerPiece) {
				dirBB |= 0b1 << targetSquare;
				if (foundFriendly) checkRayBB |= dirBB;
				else {
					checkBB |= dirBB;
					doubleCheck = isCheck;
					isCheck = true;
					//cout << "kingsquare: " << kingSquare << ", targetsquare: " << targetSquare << ", piece: " << board->squares[targetSquare] << endl;
				}
				break;
			} // If isnt sliding opposite piece
			else {
				if (foundFriendly) break;
				dirBB |= 0b1 << targetSquare;
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
		if (board->squares[targetSquare] != enemyKnight) continue;
		doubleCheck = isCheck;
		isCheck = true;
	}

	const int enemyPawn = whiteTurn ? Piece::blackPawn : Piece::whitePawn;
	const int* attackDirection = whiteTurn ? whitePawnAttackDirections : blackPawnAttackDirections;
	for (int i = 0; i < 2; i++) {
		if (preComp.directionDistances[kingSquare].direction[3 - i * 2] <= 1) continue;
		targetSquare = attackDirection[i] + kingSquare;
		if (board->squares[targetSquare] != enemyPawn) continue;
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
