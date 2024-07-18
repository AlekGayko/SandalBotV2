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

	generatePawnMoves(moves);
	//cout << "moves after pawns: " << currentMoves << endl;
	generateKnightMoves(moves);
	//cout << "moves after knight: " << currentMoves << endl;
	generateKingMoves(moves);
	//cout << "moves after king: " << currentMoves << endl;
	generateOrthogonalMoves(moves);
	//cout << "moves after orthog: " << currentMoves << endl;
	generateDiagonalMoves(moves);
	//cout << "moves after diag: " << currentMoves << endl;

	//cout << board->printBoard() << endl;
	//cout << ".currentmoves: " << currentMoves << endl;
	short int maxstart = moves[0].startSquare;
	short int maxtarget = moves[0].targetSquare;

	return currentMoves;
}

void MoveGen::initVariables() {
	isCheck = board->state.check;
	whiteTurn = board->state.whiteTurn;
	enPassantFile = board->state.enPassantFile;
	castlingRights = board->state.castlingRights;
	fiftyMoveCounter = board->state.fiftyMoveCounter;
	moveCounter = board->state.moveCounter;

	colorIndex = whiteTurn ? Board::whiteIndex : Board::blackIndex;
	enemyColorIndex = !whiteTurn ? Board::whiteIndex : Board::blackIndex;
	currentColor = whiteTurn ? Piece::white : Piece::black;
	opposingColor = !whiteTurn ? Piece::white : Piece::black;
	currentMoves = 0;
	friendlyPieceLists = board->pieceLists[colorIndex];
	enemyPieceLists = board->pieceLists[!colorIndex];

	orthogonalSliders[0] = friendlyPieceLists[Piece::rook];
	orthogonalSliders[1] = friendlyPieceLists[Piece::queen];

	diagonalSliders[0] = friendlyPieceLists[Piece::bishop];
	diagonalSliders[1] = friendlyPieceLists[Piece::queen];

}

void MoveGen::generateSlideMoves(Move moves[]) {

}

void MoveGen::generateOrthogonalMoves(Move moves[]) {
	int a = currentMoves;
	for (int type = 0; type < 2; type++) {
		int numOrthoSliders = orthogonalSliders[type].numPieces;
		for (int it = 0; it < numOrthoSliders; it++) {
			const int startSquare = orthogonalSliders[type][it];
			for (int dirIndex = startOrthogonal; dirIndex < endOrthogonal; dirIndex++) {
				const int direction = slideDirections[dirIndex];
				const int distance = preComp.directionDistances[startSquare].direction[dirIndex];
				for (int it = 1; it < distance; it++) {
					int targetSquare = direction * it + startSquare;
					// If targetsquare contains friendly piece break
					if (Piece::isColor(board->squares[targetSquare], currentColor)) break;
					// Add move
					if (targetSquare > 63) {
						cout << dirIndex << ", " << slideDirections[dirIndex] << ", " << startSquare << endl;
						throw std::out_of_range("yo");
					}
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
	if (currentMoves - a > 1) {
		cout << "there are ortho moves: " << currentMoves - a << endl;
	}
}

void MoveGen::generateDiagonalMoves(Move moves[]) {
	int a = currentMoves;
	for (int type = 0; type < 2; type++) {
		int numOrthoSliders = orthogonalSliders[type].numPieces;
		for (int it = 0; it < numOrthoSliders; it++) {
			const int startSquare = orthogonalSliders[type][it];
			for (int dirIndex = startDiagonal; dirIndex < endDiagonal; dirIndex++) {
				int direction = slideDirections[dirIndex];
				int distance = preComp.directionDistances[startSquare].direction[dirIndex];
				for (int it = 1; it < distance; it++) {
					int targetSquare = direction * it + startSquare;
					if (targetSquare > 63) {
						cout << dirIndex << ", " << slideDirections[dirIndex] << ", " << startSquare << endl;
						throw std::out_of_range("yo");
					}
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
	if (currentMoves - a > 1) {
		cout << "there are diag moves: " << currentMoves - a << endl;
	}
}

void MoveGen::generateKnightMoves(Move moves[]) {
	int numKnights = friendlyPieceLists[Piece::knight].numPieces;
	//cout << "numknights: " << numKnights << endl;
	for (int it = 0; it < numKnights; it++) {
		//cout << "it: " << it << ", currentmoves: " << currentMoves << endl;
		const int startSquare = friendlyPieceLists[Piece::knight][it];
		//cout << "startsquare: " << startSquare << endl;
		for (int dirIndex = 0; dirIndex < 8; dirIndex++) {
			//cout << "dirIndex: " << dirIndex << endl;
			if (!preComp.directionDistances[startSquare].knightSquares[dirIndex]) continue;
			const int targetSquare = knightDirections[dirIndex] + startSquare;
			//cout << "targetsquare: " << targetSquare << endl;
			if (targetSquare > 63) {
				//cout << dirIndex << ", " << knightDirections[dirIndex] << ", " << startSquare << endl;
				throw std::out_of_range("yo");
			}
			if (Piece::isColor(board->squares[targetSquare], currentColor)) continue;
			//cout << "targetsquare: " << targetSquare << endl;
			//cout << "making move" << endl;
			moves[currentMoves++] = Move(startSquare, targetSquare);
			//cout << "made move: " << currentMoves << endl;

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
		if (Piece::isColor(board->squares[targetSquare], currentColor)) continue;
		moves[currentMoves++] = Move(startSquare, targetSquare);
		if (Piece::isColor(board->squares[targetSquare], opposingColor)) moves[currentMoves - 1].takenPiece = board->squares[targetSquare];
	}
}

void MoveGen::generatePawnMoves(Move moves[]) {
	int numPawns = friendlyPieceLists[Piece::pawn].numPieces;
	const int direction = whiteTurn ? whitePawnDirection : blackPawnDirection;
	const int* attackDirection = whiteTurn ? whitePawnAttackDirections : blackPawnAttackDirections;
	const int startRow = whiteTurn ? 6 : 1;
	const int enPassantRow = whiteTurn ? 3 : 4;
	const int promotionRow = whiteTurn ? 0 : 7;

	for (int it = 0; it < numPawns; it++) {
		const int startSquare = friendlyPieceLists[Piece::pawn][it];
		if (startSquare / 8 == 7) throw std::out_of_range("Pawn cannot exist on last rank.");
		
		for (int i = 0; i < 2; i++) {
			if (preComp.directionDistances[startSquare].direction[3 - i * 2] <= 1) continue;
			int targetSquare = attackDirection[i] + startSquare;
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

		if (board->squares[direction + startSquare] == Piece::empty) {
			moves[currentMoves++] = Move(startSquare, direction + startSquare);
			if (startSquare / 8 == promotionRow) promotionMoves(moves, direction + startSquare, startSquare);
		} else return;
		if (startSquare / 8 == startRow && board->squares[direction * 2 + startSquare] == Piece::empty) {
			moves[currentMoves++] = Move(startSquare, direction * 2 + startSquare);
		}
	}
}

void MoveGen::enPassantMoves(Move moves[], int targetSquare, int startSquare) {
	moves[currentMoves++] = Move(startSquare, targetSquare, Move::enPassantCaptureFlag);
}

void MoveGen::promotionMoves(Move moves[], int targetSquare, int startSquare) {
	for (int i = 0; i < 4; i++) {
		moves[currentMoves++] = Move(startSquare, targetSquare, promotionFlags[i]);
	}
}

void MoveGen::castlingMoves(Move moves[], int startSquare) {
	const int shortMask = whiteTurn ? BoardState::whiteShortCastleMask : BoardState::blackShortCastleMask;
	const int longMask = whiteTurn ? BoardState::whiteLongCastleMask : BoardState::blackLongCastleMask;
	const int friendlyRook = whiteTurn ? Piece::whiteRook : Piece::blackRook;


	if (castlingRights & shortMask && board->squares[shortCastleRookSquares[colorIndex]] == friendlyRook && board->squares[startSquare + 1] == Piece::empty && board->squares[startSquare + 2] == Piece::empty) {
		moves[currentMoves++] = Move(startingKingSquares[colorIndex], shortCastleKingSquares[colorIndex], Move::castleFlag);
	}
	if (castlingRights & longMask && board->squares[shortCastleRookSquares[colorIndex]] == friendlyRook && board->squares[startSquare - 1] == Piece::empty && board->squares[startSquare - 2] == Piece::empty && board->squares[startSquare - 3]) {
		moves[currentMoves++] = Move(startingKingSquares[colorIndex], longCastleKingSquares[colorIndex], Move::castleFlag);
	}
}

void MoveGen::generateCheckData() {

}
