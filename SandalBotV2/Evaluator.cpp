#include "Evaluator.h"
#include "BitBoardUtility.h"

#include <iostream>

using namespace std;

int Evaluator::colorStart[2] = { 63, 0 };
int Evaluator::blackEvalSquare[64];

Evaluator::Evaluator() {
	initBlackSquares();
}

Evaluator::Evaluator(Board* board, MovePrecomputation* precomputation) : precomputation(precomputation), board(board) {
	initBlackSquares();
	initStaticVariables();
}

bool Evaluator::insufficientMaterial() {
	if (endGameWeight == 0.f) {
		return false;
	}

	// If there are only kings, insufficient
	if (whiteMaterial == 0 && blackMaterial == 0) {
		return true;
	}
	// If there are pawns on board, sufficient material
	if (board->pawns != 0ULL) {
		return false;
	}
	// If there is rook or queen, sufficient material
	if (board->orthogonalPieces != 0ULL) {
		return false;
	}

	if (whiteMaterial == 0) {
		return insufficientMateMaterial(blackMaterial);
	} else if (blackMaterial == 0) {
		return insufficientMateMaterial(whiteMaterial);
	}

	if (whiteMaterial <= PieceEvaluations::bishopVal && blackMaterial <= PieceEvaluations::bishopVal) {
		return true;
	}

	return false;
}

bool Evaluator::insufficientMateMaterial(int material) {
	if (material <= PieceEvaluations::bishopVal) {
		return true;
	} else if (material == 2 * PieceEvaluations::knightVal) {
		return true;
	}

	return false;
}

void Evaluator::initBlackSquares() {
	for (int square = 0; square < 64; square++) {
		int row = square / 8;
		int col = square % 8;
		blackEvalSquare[square] = (7 - row) * 8 + col;
	}
}

void Evaluator::initStaticVariables() {
	whiteStaticEval = 0;
	blackStaticEval = 0;
	whiteMaterial = 0;
	blackMaterial = 0;
	whiteMMPieces = 0;
	blackMMPieces = 0;

	initEndgameWeight();
	initPieceEvaluations();
}

void Evaluator::initVariables() {
	evaluation = 0;
	maximisingSide = board->state->whiteTurn ? 1 : -1;

	friendlyColor = board->state->whiteTurn ? Piece::white : Piece::black;
	enemyColor = !board->state->whiteTurn ? Piece::white : Piece::black;
	maximisingIndex = board->state->whiteTurn ? Board::whiteIndex : Board::blackIndex;
	minimisingIndex = !board->state->whiteTurn ? Board::whiteIndex : Board::blackIndex;

	friendlyKingSquare = board->pieceLists[maximisingIndex][Piece::king][0];
	enemyKingSquare = board->pieceLists[minimisingIndex][Piece::king][0];

	friendlyBoard = board->state->whiteTurn ? board->whitePieces : board->blackPieces;
	enemyBoard = !board->state->whiteTurn ? board->whitePieces : board->blackPieces;

	friendlyMaterial = board->state->whiteTurn ? whiteMaterial : blackMaterial;
	enemyMaterial = !board->state->whiteTurn ? whiteMaterial : blackMaterial;

	friendlyMajorMinorPieces = board->state->whiteTurn ? whiteMMPieces : blackMMPieces;
	enemyMajorMinorPieces = !board->state->whiteTurn ? whiteMMPieces : blackMMPieces;
}

int Evaluator::Evaluate() {
	initVariables();

	// If insufficient material its a draw
	if (insufficientMaterial()) {
		return drawScore;
	}

	evaluation += staticPieceEvaluations();
	evaluation += pawnIslandEvaluations();
	evaluation += passedPawnEvaluations();
	evaluation += kingSafety();
	evaluation += openFilesEvaluation();
	evaluation += openDiagEvaluation();
	evaluation += kingDist();

	return evaluation;
}

bool Evaluator::isMateScore(int score) {
	return abs(score) >= checkMateScore / 2;
}

int Evaluator::movesTilMate(int score) {
	if (!isMateScore(score))
		return 0;
	return max(1, ((checkMateScore - abs(score) - 1) / 2) + 1);
}

int Evaluator::kingSafety() {
	int evaluation = 0;

	if (enemyMaterial == 0 || enemyMajorMinorPieces == 0) {
		return evaluation;
	}

	evaluation += pawnShieldEvaluations();
	//evaluation += kingTropismEvaluations();

	evaluation *= kingSafetyCoefficient;

	//evaluation += kingAttackZones();

	return evaluation;
}

int Evaluator::kingTropismEvaluations() {
	int evaluation = enemyMaterial * kingTropism(friendlyKingSquare, board->pieceLists[maximisingIndex]);
	evaluation -= friendlyMaterial * kingTropism(enemyKingSquare, board->pieceLists[minimisingIndex]);
	return evaluation;
}

int Evaluator::kingTropism(const int& kingSquare, PieceList* enemyList) {
	int evaluation = 0;
	int numPieces;
	int square;

	for (int piece = Piece::pawn; piece < Piece::king; piece++) {
		numPieces = enemyList[piece].numPieces;
		for (int it = 0; it < numPieces; it++) {
			square = enemyList[piece][it];

			evaluation -= (7 - precomputation->getDistance(kingSquare, square)) * tropismWeightings[piece];
		}
	}

	return evaluation;
}

int Evaluator::pawnShieldEvaluations() {
	if (endGameWeight >= 0.3f) {
		return 0;
	}

	uint64_t friendlyShield = precomputation->getShieldMask(friendlyKingSquare, friendlyColor);
	uint64_t enemyShield = precomputation->getShieldMask(enemyKingSquare, enemyColor);

	friendlyShield &= board->pawns & friendlyBoard;
	enemyShield &= board->pawns & enemyBoard;

	int evaluation = (enemyMaterial / PieceEvaluations::pawnVal) * pawnShieldEvaluation(friendlyKingSquare, friendlyShield);
	evaluation -= (friendlyMaterial / PieceEvaluations::pawnVal) * pawnShieldEvaluation(enemyKingSquare, enemyShield);

	evaluation *= (1 - endGameWeight);

	return evaluation;
}

int Evaluator::pawnShieldEvaluation(const int& square, uint64_t& pawns) {
	int evaluation = 0;
	uint64_t attackMask;
	uint64_t pawnDefenders;
	uint64_t kingSquares = precomputation->getKingMoves(square);
	int targetSquare;
	int col = square % 8;

	// If there is no pawn shield on a column, add penalty
	if ((pawns & precomputation->getColMask(col)) == 0ULL) {
		evaluation -= pawnShieldColumnPenalty;
	}
	// Logic pertains to include edge file shields
	if (col == 7) {
		col = (square % 8) - 2;
	} else {
		col = col + 1;
	}


	if ((pawns & precomputation->getColMask(col)) == 0ULL) {
		evaluation -= pawnShieldColumnPenalty;
	}

	if (col == 0) {
		col = (square % 8) + 2;
	} else {
		col = (square % 8) - 1;
	}

	if ((pawns & precomputation->getColMask(col)) == 0ULL) {
		evaluation -= pawnShieldColumnPenalty;
	}

	// Undefended pawns have penalty
	while (pawns != 0ULL) {
		targetSquare = BitBoardUtility::popLSB(pawns);
		attackMask = precomputation->getPawnAttackMoves(targetSquare, Piece::white);
		attackMask |= precomputation->getPawnAttackMoves(targetSquare, Piece::black);

		pawnDefenders = pawns & attackMask;

		if (pawnDefenders == 0ULL && (kingSquares & (1ULL << targetSquare)) == 0ULL) {
			evaluation -= pawnShieldUndefendedPenalty;
		}
	}

	return evaluation;
}

int Evaluator::passedPawnEvaluations() {
	uint64_t friendlyPawns = board->pawns & friendlyBoard;
	uint64_t enemyPawns = board->pawns & enemyBoard;

	int evaluation = passedPawnEvaluation(board->pieceLists[maximisingIndex][Piece::pawn], friendlyPawns, enemyPawns, friendlyColor);
	evaluation -= passedPawnEvaluation(board->pieceLists[minimisingIndex][Piece::pawn], enemyPawns, friendlyPawns, enemyColor);

	return evaluation;
}

// Evaluates the passed pawns of either side
int Evaluator::passedPawnEvaluation(PieceList& pieceList, uint64_t& pawns, uint64_t& opposingPawns, const int& color) {
	int evaluation = 0;

	uint64_t passedMask;
	int numPawns = pieceList.numPieces;
	int square;
	int promotionDistance;

	for (int i = 0; i < numPawns; i++) {
		square = pieceList[i];
		passedMask = precomputation->getPassedPawnMask(square, color);

		// If no pawns in front of pawn's promotion path
		if ((passedMask & opposingPawns) == 0ULL) {
			promotionDistance = color == Piece::white ? square / 8 : 7 - (square / 8);

			evaluation += passedPawnBonus[promotionDistance];
		}
	}

	return evaluation;
}

int Evaluator::pawnIslandEvaluations() {
	uint64_t friendlyPawns = board->pawns & friendlyBoard;
	uint64_t enemyPawns = board->pawns & enemyBoard;

	int evaluation = pawnIslandEvaluation(board->pieceLists[maximisingIndex][Piece::pawn], friendlyPawns);
	evaluation -= pawnIslandEvaluation(board->pieceLists[minimisingIndex][Piece::pawn], enemyPawns);

	return evaluation;
}

int Evaluator::pawnIslandEvaluation(PieceList& pieceList, uint64_t& pawns) {
	int evaluation = 0;

	uint64_t islandMask;
	int numPawns = pieceList.numPieces;
	int square;

	for (int i = 0; i < numPawns; i++) {
		square = pieceList[i];
		islandMask = precomputation->getPawnIslandMask(square % 8);
		if ((pawns & islandMask) != 0ULL)
			continue;

		evaluation -= pawnIslandPenalty;
	}

	return evaluation;
}

int Evaluator::kingAttackZones() {
	uint64_t friendlyKingZone;
	uint64_t enemyKingZone;

	// Choose king zone depending on whether it is an endgame or king is away from starting row
	if (abs(friendlyKingSquare / 8 - startRow[maximisingIndex]) >= 2 || endGameWeight >= 0.2f) {
		friendlyKingZone = precomputation->getUnbiasKingAttackZone(friendlyKingSquare);
	} else {
		friendlyKingZone = precomputation->getKingAttackSquare(friendlyKingSquare, friendlyColor);
	}

	if (abs(enemyKingSquare / 8 - startRow[minimisingIndex]) >= 2 || endGameWeight >= 0.2f) {
		enemyKingZone = precomputation->getUnbiasKingAttackZone(enemyKingSquare);
	} else {
		enemyKingZone = precomputation->getKingAttackSquare(enemyKingSquare, enemyColor);
	}
	
	int evaluation = kingAttackZone(friendlyKingZone, friendlyBoard, enemyKingSquare, enemyColor);
	evaluation -= kingAttackZone(enemyKingZone, enemyBoard, friendlyKingSquare, friendlyColor);

	return evaluation;
}

int Evaluator::kingAttackZone(uint64_t& attackZone, uint64_t& friendlyPieces, int& enemyKingSquare, int& opposingColor) {
	int evaluation = 0;
	uint64_t knights = friendlyPieces & board->knights;
	uint64_t pawns = friendlyPieces & board->pawns;
	uint64_t rooks = friendlyPieces & board->orthogonalPieces & ~board->diagonalPieces;
	uint64_t bishops = friendlyPieces & board->diagonalPieces & ~board->orthogonalPieces;
	uint64_t queens = rooks & bishops;

	evaluation += evalOrthMoves(attackZone, rooks, enemyKingSquare);
	evaluation += evalDiagMoves(attackZone, bishops, enemyKingSquare);
	evaluation += evalQueenMoves(attackZone, queens, enemyKingSquare);
	evaluation += evalPawnMoves(attackZone, pawns, opposingColor);
	evaluation += evalKnightMoves(attackZone, knights);

	evaluation = -kingZoneSafety[evaluation];

	return evaluation;
}

int Evaluator::incrementAttackZoneEval(uint64_t& attackZone, uint64_t& moves, const int& piece) {
	uint64_t zoneMoves = attackZone & moves;
	int evaluation = 0;

	while (zoneMoves != 0ULL) {
		BitBoardUtility::popLSB(zoneMoves);
		evaluation += attackUnitScores[piece];
	}

	return evaluation;
}

int Evaluator::evalKnightMoves(uint64_t& attackZone, uint64_t& knights) {
	uint64_t moves;
	int startSquare;
	int evaluation = 0;

	while (knights != 0ULL) {
		startSquare = BitBoardUtility::popLSB(knights);
		moves = precomputation->getKnightBoard(startSquare);
		evaluation += incrementAttackZoneEval(attackZone, moves, Piece::knight);
	}

	return evaluation;
}

int Evaluator::evalPawnMoves(uint64_t& attackZone, uint64_t& pawns, const int& opposingColor) {
	uint64_t moves;
	int startSquare;
	int evaluation = 0;

	while (pawns != 0ULL) {
		startSquare = BitBoardUtility::popLSB(pawns);
		moves = precomputation->getPawnAttackMoves(startSquare, opposingColor);
		evaluation += incrementAttackZoneEval(attackZone, moves, Piece::pawn);
	}

	return evaluation;
}

int Evaluator::evalOrthMoves(uint64_t& attackZone, uint64_t& orths, int& enemyKingSquare) {
	uint64_t moves;
	int startSquare;
	int evaluation = 0;

	while (orths != 0ULL) {
		startSquare = BitBoardUtility::popLSB(orths);

		uint64_t blockers = board->allPieces & precomputation->getBlockerOrthogonalMask(startSquare);
		BitBoardUtility::deleteBit(blockers, enemyKingSquare);
		moves = precomputation->getOrthMovementBoard(startSquare, blockers);
		evaluation += incrementAttackZoneEval(attackZone, moves, Piece::rook);
	}

	return evaluation;
}

int Evaluator::evalDiagMoves(uint64_t& attackZone, uint64_t& diags, int& enemyKingSquare) {
	uint64_t moves;
	int startSquare;
	int evaluation = 0;

	while (diags != 0ULL) {
		startSquare = BitBoardUtility::popLSB(diags);

		uint64_t blockers = board->allPieces & precomputation->getBlockerDiagonalMask(startSquare);
		BitBoardUtility::deleteBit(blockers, enemyKingSquare);
		moves = precomputation->getDiagMovementBoard(startSquare, blockers);
		evaluation += incrementAttackZoneEval(attackZone, moves, Piece::bishop);
	}

	return evaluation;
}

int Evaluator::evalQueenMoves(uint64_t& attackZone, uint64_t& queens, int& enemyKingSquare) {
	uint64_t moves;
	int startSquare;
	int evaluation = 0;

	while (queens != 0ULL) {
		startSquare = BitBoardUtility::popLSB(queens);

		uint64_t orthBlockers = board->allPieces & precomputation->getBlockerOrthogonalMask(startSquare);
		uint64_t diagBlockers = board->allPieces & precomputation->getBlockerDiagonalMask(startSquare);

		BitBoardUtility::deleteBit(orthBlockers, enemyKingSquare);
		BitBoardUtility::deleteBit(diagBlockers, enemyKingSquare);

		moves = precomputation->getOrthMovementBoard(startSquare, orthBlockers);
		moves |= precomputation->getDiagMovementBoard(startSquare, diagBlockers);

		evaluation += incrementAttackZoneEval(attackZone, moves, Piece::queen);
	}

	return evaluation;
}

int Evaluator::openFilesEvaluation() {
	// Endgame is less likely to require open files
	if (endGameWeight >= 0.3)
		return 0;
	// If no orthogonal pieces, no use of open file
	if (board->orthogonalPieces == 0ULL) {
		return 0;
	}

	int evaluation = 0;
	uint64_t colMask;
	uint64_t filePieces;
	int pawnCounter;
	for (int col = 0; col < 8; col++) {
		colMask = precomputation->getColMask(col);
		filePieces = board->pawns & colMask;
		pawnCounter = 0;

		while (filePieces != 0ULL) {
			BitBoardUtility::popLSB(filePieces);
			pawnCounter++;
			if (pawnCounter > 1) {
				break;
			}
		}
		if (pawnCounter > 1) {
			continue;
		}
		evaluation += evaluateOpenFile(colMask, pawnCounter);
	}

	return evaluation;
}

int Evaluator::evaluateOpenFile(uint64_t column, int pawnCounter) {
	// If open/semi-open file has no orthogonal pieces, unlikely to be advantageous
	if ((column & board->orthogonalPieces) == 0ULL)
		return 0;
	int evaluation = 0;
	int square;
	// Start by analysing queens on file
	uint64_t OrthFile = column & board->orthogonalPieces & board->diagonalPieces;

	int friendlyOrths = 0;
	int enemyOrths = 0;

	while (OrthFile != 0ULL) {
		square = BitBoardUtility::popLSB(OrthFile);
		// Friendly Queen
		if (friendlyBoard & (1ULL << square)) {
			evaluation += 0.7f * openFileBonus;
			friendlyOrths += 1;
		}
		// Enemy Queen
		else {
			evaluation -= 0.7f * openFileBonus;
			enemyOrths += 1;
		}
	}
	// Only rooks
	OrthFile = column & board->orthogonalPieces & ~board->diagonalPieces;

	while (OrthFile != 0ULL) {
		square = BitBoardUtility::popLSB(OrthFile);
		// Friendly Rook
		if (friendlyBoard & (1ULL << square)) {
			evaluation += openFileBonus;
			friendlyOrths += 1;
		}
		// Enemy Rook
		else {
			evaluation -= openFileBonus;
			enemyOrths += 1;
		}
	}

	evaluation = pawnCounter == 0 ? evaluation : 0.2f * evaluation;

	if (openDiagFileNearKing(column, friendlyKingSquare) && enemyOrths != 0) {
		evaluation -= openFileNearKingBonus * enemyOrths;
	}

	if (openDiagFileNearKing(column, enemyKingSquare) && friendlyOrths != 0) {
		evaluation += openFileNearKingBonus * friendlyOrths;
	}

	return evaluation;
}

int Evaluator::openDiagEvaluation() {
	// Endgame is less likely to require open diags
	if (endGameWeight >= 0.3)
		return 0;
	// If no diagonal pieces, no use of open diag
	if (board->orthogonalPieces == 0ULL) {
		return 0;
	}

	int evaluation = 0;
	uint64_t forwardDiagMask;
	uint64_t backwardDiagMask;

	uint64_t forwardPawns;
	uint64_t backwardPawns;

	int pawnCounter;
	const int diagSize = 3;
	int forwardDiagSquares[3] = { 6, 7, 15 };
	int backwardDiagSquares[3] = { 8, 0, 1 };

	for (int it = 0; it < diagSize; it++) {
		forwardDiagMask = precomputation->getForwardMask(forwardDiagSquares[it]);

		forwardPawns = board->pawns & forwardDiagMask;

		pawnCounter = 0;

		while (forwardPawns != 0ULL) {
			BitBoardUtility::popLSB(forwardPawns);
			pawnCounter++;
			if (pawnCounter > 1) {
				break;
			}
		}
		if (pawnCounter > 1) {
			continue;
		}
		evaluation += evaluateOpenDiag(forwardDiagMask, pawnCounter);
	}

	for (int it = 0; it < diagSize; it++) {
		backwardDiagMask = precomputation->getBackwardMask(backwardDiagSquares[it]);

		backwardPawns = board->pawns & backwardDiagMask;

		pawnCounter = 0;

		while (backwardPawns != 0ULL) {
			BitBoardUtility::popLSB(backwardPawns);
			pawnCounter++;
			if (pawnCounter > 1) {
				break;
			}
		}
		if (pawnCounter > 1) {
			continue;
		}
		evaluation += evaluateOpenDiag(backwardDiagMask, pawnCounter);
	}

	return evaluation;
}

int Evaluator::evaluateOpenDiag(uint64_t diag, int pawnCounter) {
	// If open/semi-open diagonal has no diagonal pieces, unlikely to be advantageous
	if ((diag & board->diagonalPieces) == 0ULL)
		return 0;

	int evaluation = 0;
	int square;
	// Start by analysing queens on file
	uint64_t diagPieces = diag & board->orthogonalPieces & board->diagonalPieces;

	int friendlyDiags = 0;
	int enemyDiags = 0;

	while (diagPieces != 0ULL) {
		square = BitBoardUtility::popLSB(diagPieces);
		// Friendly Queen
		if (friendlyBoard & (1ULL << square)) {
			evaluation += 0.7f * openDiagBonus;
			friendlyDiags += 1;
		}
		// Enemy Queen
		else {
			evaluation -= 0.7f * openDiagBonus;
			enemyDiags += 1;
		}
	}
	// Only bishops
	diagPieces = diag & board->diagonalPieces & ~board->orthogonalPieces;

	while (diagPieces != 0ULL) {
		square = BitBoardUtility::popLSB(diagPieces);
		// Friendly bishop
		if (friendlyBoard & (1ULL << square)) {
			evaluation += openDiagBonus;
			friendlyDiags += 1;
		}
		// Enemy bishop
		else {
			evaluation -= openDiagBonus;
			enemyDiags += 1;
		}
	}

	evaluation = pawnCounter == 0 ? evaluation : 0.2f * evaluation;

	if (openDiagFileNearKing(diag, friendlyKingSquare) && enemyDiags != 0) {
		evaluation -= openDiagNearKingBonus * enemyDiags;
	}

	if (openDiagFileNearKing(diag, enemyKingSquare) && friendlyDiags != 0) {
		evaluation += openDiagNearKingBonus * friendlyDiags;
	}

	return evaluation;
}

bool Evaluator::openDiagFileNearKing(uint64_t mask, int kingSquare) {
	return mask & (precomputation->getKingMoves(kingSquare));
}

void Evaluator::initPieceEvaluations() {
	whiteMaterial = 0;
	blackMaterial = 0;
	whiteStaticEval = staticPieceEvaluation(board->pieceLists[Board::whiteIndex], whiteMaterial);
	blackStaticEval = staticPieceEvaluation(board->pieceLists[Board::blackIndex], blackMaterial, true);
}

int Evaluator::staticPieceEvaluations() {
	int whiteEval = 0;
	int blackEval = 0;
	// Since endgame-based evaluations cannot be progressively evaluated,
	// delete square evaluations of pawns and kings, then add interpolation of endgame square values

	if (endGameWeight > 0.f) {
		int square;
		uint64_t whitePawns = board->pawns & board->whitePieces;
		uint64_t blackPawns = board->pawns & board->blackPieces;

		while (whitePawns != 0ULL) {
			square = BitBoardUtility::popLSB(whitePawns);
			whiteEval -= PieceEvaluations::pawnEval[square];
			whiteEval += (1 - endGameWeight) * PieceEvaluations::pawnEval[square] + endGameWeight * PieceEvaluations::pawnEndgameEval[square];
		}

		while (blackPawns != 0ULL) {
			square = BitBoardUtility::popLSB(blackPawns);
			square = blackEvalSquare[square];
			blackEval -= PieceEvaluations::pawnEval[square];
			blackEval += (1 - endGameWeight) * PieceEvaluations::pawnEval[square] + endGameWeight * PieceEvaluations::pawnEndgameEval[square];
		}

		int whiteKingSquare = board->pieceLists[Board::whiteIndex][Piece::king][0];
		int blackKingSquare = board->pieceLists[Board::blackIndex][Piece::king][0];
		blackKingSquare = blackEvalSquare[blackKingSquare];

		whiteEval -= PieceEvaluations::kingEval[whiteKingSquare];
		whiteEval += (1 - endGameWeight) * PieceEvaluations::kingEval[whiteKingSquare] + endGameWeight * PieceEvaluations::kingEndgameEval[whiteKingSquare];

		blackEval -= PieceEvaluations::kingEval[blackKingSquare];
		blackEval += (1 - endGameWeight) * PieceEvaluations::kingEval[blackKingSquare] + endGameWeight * PieceEvaluations::kingEndgameEval[blackKingSquare];
	}
	//cout << "whitestaticeval: " << whiteStaticEval << endl;
	//cout << "blackstaticeval: " << blackStaticEval << endl;
	return maximisingSide * (whiteStaticEval + whiteEval - (blackStaticEval + blackEval));
}

int Evaluator::staticPieceEvaluation(PieceList* pieceList, int& material, bool useBlackSquares) {
	int evaluation = 0;
	int numPieces;
	int pieceEval;
	int square;

	for (int piece = Piece::pawn; piece <= Piece::king; piece++) {
		numPieces = pieceList[piece].numPieces;
		pieceEval = PieceEvaluations::pieceVals[piece];

		material += pieceEval * numPieces;

		for (int it = 0; it < numPieces; it++) {
			square = pieceList[piece][it];
			if (useBlackSquares)
				square = blackEvalSquare[square];

			evaluation += PieceEvaluations::pieceEvals[piece][square];
		}
	}

	evaluation += material;

	return evaluation;
}

// Calculates mopup evaluation
int Evaluator::kingDist() {
	// If not an endgame or evaluation is too tight, dont bother with mopup evaluation
	if (endGameWeight == 0.f || abs(evaluation) < 2 * PieceEvaluations::pieceVals[Piece::pawn]) {
		return 0;
	}

	int mopUpScore = 0;

	int losingKingSquare = evaluation > 0 ? enemyKingSquare : friendlyKingSquare;

	int losingKingCMD = arrCenterManhattanDistance[losingKingSquare];

	int kingsMD = abs((friendlyKingSquare / 8) - (enemyKingSquare / 8)) + abs((friendlyKingSquare % 8) - (enemyKingSquare % 8));

	// From Chess 4.x
	mopUpScore = 4.7f * losingKingCMD + 1.6f * (14 - kingsMD);
	mopUpScore *= endGameWeight;

	// Discourage moving to outer edge of board if losing
	if (evaluation < 0) {
		mopUpScore *= -1;
	}

	return mopUpScore;
}

void Evaluator::initEndgameWeight() {
	whiteMMPieces = 0;
	blackMMPieces = 0;

	for (int colorIndex = Board::blackIndex; colorIndex <= Board::whiteIndex; colorIndex++) {
		for (int piece = Piece::knight; piece < Piece::king; piece++) {
			if (colorIndex == Board::whiteIndex)
				whiteMMPieces += board->pieceLists[colorIndex][piece].numPieces;
			else {
				blackMMPieces += board->pieceLists[colorIndex][piece].numPieces;
			}
		}
	}

	calculateEndgameWeight();
}

void Evaluator::calculateEndgameWeight() {
	// Square min component to create smoother curve 
	// (pow function is inherently slower than variable * variable)
	endGameWeight = min(1.f, (whiteMMPieces + blackMMPieces) / endgameRequiredPieces);
	endGameWeight *= endGameWeight;
	endGameWeight = 1.f - endGameWeight;
}

void Evaluator::staticPieceMove(const int piece, int startSquare, int targetSquare, const bool whiteTurn) {
	int* movingSide = whiteTurn ? &whiteStaticEval : &blackStaticEval;
	if (!whiteTurn) {
		startSquare = blackEvalSquare[startSquare];
		targetSquare = blackEvalSquare[targetSquare];
	}
	*movingSide += PieceEvaluations::pieceEvals[piece][targetSquare] - PieceEvaluations::pieceEvals[piece][startSquare];
}

void Evaluator::staticPieceDelete(const int piece, int square, const bool whiteTurn) {
	int* otherSide = whiteTurn ? &whiteStaticEval : &blackStaticEval;

	*otherSide -= PieceEvaluations::pieceVals[piece];
	if (!whiteTurn) {
		square = blackEvalSquare[square];
	}
	switch (piece) {
	case Piece::empty:
		break;
	default:
		*otherSide -= PieceEvaluations::pieceEvals[piece][square];
		break;
	}

	int* material = whiteTurn ? &whiteMaterial : &blackMaterial;
	*material -= PieceEvaluations::pieceVals[piece];

	if (piece > Piece::pawn && piece < Piece::king) {
		int* MMMaterial = whiteTurn ? &whiteMMPieces : &blackMMPieces;
		*MMMaterial -= 1;
		calculateEndgameWeight();
	}
}

void Evaluator::staticPieceSpawn(const int piece, int square, const bool whiteTurn) {
	int* movingSide = whiteTurn ? &whiteStaticEval : &blackStaticEval;

	*movingSide += PieceEvaluations::pieceVals[piece];
	if (!whiteTurn) {
		square = blackEvalSquare[square];
	}

	*movingSide += PieceEvaluations::pieceEvals[piece][square];

	int* material = whiteTurn ? &whiteMaterial : &blackMaterial;
	*material += PieceEvaluations::pieceVals[piece];

	if (piece > Piece::pawn && piece < Piece::king) {
		int* MMMaterial = whiteTurn ? &whiteMMPieces : &blackMMPieces;
		*MMMaterial += 1;
		calculateEndgameWeight();
	}

}
