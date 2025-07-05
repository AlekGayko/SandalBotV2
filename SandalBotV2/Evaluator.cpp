#include "Evaluator.h"
#include "BitBoardUtility.h"

#include <cassert>
#include <iostream>

using namespace std;

namespace SandalBot {

	int Evaluator::blackEvalSquare[64];

	Evaluator::Evaluator() {
		initBlackSquares();
	}

	Evaluator::Evaluator(Board* board, MovePrecomputation* precomputation) : precomputation(precomputation), board(board) {
		initBlackSquares();
		initStaticVariables();
	}

	// Returns true if insufficient material, false otherwise
	bool Evaluator::insufficientMaterial() {
		// Impossible for insufficient material if not endgame (many pieces on board)
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

		// If sum of material on both sides less than or equal to bishop, insufficient material
		// (Covers multiple cases)
		if (whiteMaterial <= PieceEvaluations::bishopVal && blackMaterial <= PieceEvaluations::bishopVal) {
			return true;
		}

		return false;
	}

	// Returns true if count of material is insufficient to provide checkmate, false otherwise
	bool Evaluator::insufficientMateMaterial(int material) {
		// If bishop or less, insufficient
		if (material <= PieceEvaluations::bishopVal) {
			return true;
		} 
		// If two knights, checkmate cannot be forced (but technically possible), therefore insufficient
		else if (material == 2 * PieceEvaluations::knightVal) {
			return true;
		}

		return false;
	}

	// Initialises blackEvalSquares by inverting row by keeping column value for index.
	// Allows for white and black to index same position based arrays
	void Evaluator::initBlackSquares() {
		for (int square = 0; square < 64; square++) {
			int row = square / 8;
			int col = square % 8;
			blackEvalSquare[square] = (7 - row) * 8 + col;
		}
	}

	// Initialise static variables which be kept track of when board makes moves
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

	// Initialise variables
	void Evaluator::initVariables() {
		evaluation = 0;
		whiteKingSquare = board->pieceLists[Board::whiteIndex][Piece::king][0];
		blackKingSquare = board->pieceLists[Board::blackIndex][Piece::king][0];
	}

	// Returns an integer representing the static evaluation of the board's position
	// Negative return value signifies losing position, zero is a draw, and positive is winning
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

		return board->state->whiteTurn ? evaluation : -evaluation;
	}

	// Returns true if score is a checkmate score
	bool Evaluator::isMateScore(int score) {
		return abs(score) >= checkMateScore / 2;
	}

	// Returns moves until checkmate from a given score
	int Evaluator::movesTilMate(int score) {
		if (!isMateScore(score))
			return 0;
		return max(1, ((checkMateScore - abs(score) - 1) / 2) + 1);
	}

	// Returns the evaluation regarding king safety
	int Evaluator::kingSafety() {
		int evaluation = 0;

		// If no major minor pieces, king safety is not too much of a factor
		if (whiteMMPieces == 0 || blackMMPieces == 0) {
			return evaluation;
		}

		evaluation += pawnShieldEvaluations();
		//evaluation += kingTropismEvaluations();

		evaluation = evaluation * kingSafetyCoefficient;

		//evaluation += kingAttackZones();

		return evaluation;
	}

	// Returns the evaluation for king tropism
	int Evaluator::kingTropismEvaluations() {
		int evaluation = blackMaterial * kingTropism(whiteKingSquare, board->pieceLists[Board::whiteIndex]);
		evaluation -= whiteMaterial * kingTropism(blackKingSquare, board->pieceLists[Board::blackIndex]);
		return evaluation;
	}

	// Calculate king tropism for one side
	int Evaluator::kingTropism(const int kingSquare, PieceList* enemyList) {
		int evaluation = 0;
		int numPieces;
		int square;

		for (int piece = Piece::pawn; piece < Piece::king; ++piece) {
			numPieces = enemyList[piece].numPieces;
			for (int it = 0; it < numPieces; ++it) {
				square = enemyList[piece][it];

				evaluation -= (7 - precomputation->getDistance(kingSquare, square)) * tropismWeightings[piece];
			}
		}

		return evaluation;
	}

	// Caluclate evaluation of pawn shield during opening/middle game phases
	int Evaluator::pawnShieldEvaluations() {
		if (endGameWeight >= 0.3f) {
			return 0;
		}

		// Mask to identify pawns near king
		uint64_t whiteShield = precomputation->getShieldMask(whiteKingSquare, Piece::white);
		uint64_t blackShield = precomputation->getShieldMask(blackKingSquare, Piece::black);

		whiteShield &= board->pawns & board->whitePieces;
		blackShield &= board->pawns & board->blackPieces;

		int evaluation = (blackMaterial / PieceEvaluations::pawnVal) * pawnShieldEvaluation(whiteKingSquare, whiteShield);
		evaluation -= (whiteMaterial / PieceEvaluations::pawnVal) * pawnShieldEvaluation(blackKingSquare, blackShield);

		evaluation *= (1 - endGameWeight); // Make pawn shields linearly less important as game approaches end game

		return evaluation;
	}

	// Calculate pawn shield evaluation for one side
	int Evaluator::pawnShieldEvaluation(const int square, uint64_t& pawns) {
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

		// Undefended pawns have penalty (undefended by king and/or other pawns)
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

	// Returns evaluation of passed pawns
	int Evaluator::passedPawnEvaluations() {
		uint64_t whitePawns = board->pawns & board->whitePieces;
		uint64_t blackPawns = board->pawns & board->blackPieces;

		int evaluation = passedPawnEvaluation(board->pieceLists[Board::whiteIndex][Piece::pawn], blackPawns, Piece::white);
		evaluation -= passedPawnEvaluation(board->pieceLists[Board::blackIndex][Piece::pawn], whitePawns, Piece::black);

		return evaluation;
	}

	// Evaluates the passed pawns of either side
	int Evaluator::passedPawnEvaluation(PieceList& pieceList, uint64_t opposingPawns, const int color) {
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
				// Squares from end of board
				promotionDistance = color == Piece::white ? square / 8 : 7 - (square / 8);

				evaluation += passedPawnBonus[promotionDistance];
			}
		}

		return evaluation;
	}

	// Returns evaluation of pawn islands
	int Evaluator::pawnIslandEvaluations() {
		uint64_t whitePawns = board->pawns & board->whitePieces;
		uint64_t blackPawns = board->pawns & ~whitePawns;

		int evaluation = pawnIslandEvaluation(board->pieceLists[Board::whiteIndex][Piece::pawn], whitePawns);
		evaluation -= pawnIslandEvaluation(board->pieceLists[Board::blackIndex][Piece::pawn], blackPawns);

		return evaluation;
	}

	// Returns evaluation of pawn islands from one side
	int Evaluator::pawnIslandEvaluation(PieceList& pieceList, uint64_t pawns) {
		int evaluation = 0;

		uint64_t islandMask;
		int numPawns = pieceList.numPieces;
		int square;

		// If no friendly pawns on either column of pawn, it is an island
		for (int i = 0; i < numPawns; i++) {
			square = pieceList[i];
			islandMask = precomputation->getPawnIslandMask(square % 8);
			if ((pawns & islandMask) != 0ULL)
				continue;

			evaluation -= pawnIslandPenalty;
		}

		return evaluation;
	}

	// Returns evaluation of king attack zones, measures frequency of attacks near king's area
	int Evaluator::kingAttackZones() {
		uint64_t friendlyKingZone;
		uint64_t enemyKingZone;

		// Choose king zone depending on whether it is an endgame or king is away from starting row
		if (abs(whiteKingSquare / 8 - startRow[Board::whiteIndex]) >= 2 || endGameWeight >= 0.2f) {
			friendlyKingZone = precomputation->getUnbiasKingAttackZone(whiteKingSquare);
		} else {
			friendlyKingZone = precomputation->getKingAttackSquare(whiteKingSquare, Piece::white);
		}
		// Choose king zone depending on whether it is an endgame or king is away from starting row
		if (abs(blackKingSquare / 8 - startRow[Board::blackIndex]) >= 2 || endGameWeight >= 0.2f) {
			enemyKingZone = precomputation->getUnbiasKingAttackZone(blackKingSquare);
		} else {
			enemyKingZone = precomputation->getKingAttackSquare(blackKingSquare, Piece::black);
		}

		int evaluation = kingAttackZone(friendlyKingZone, board->whitePieces, blackKingSquare, Piece::black);
		evaluation -= kingAttackZone(enemyKingZone, board->blackPieces, whiteKingSquare, Piece::white);

		return evaluation;
	}
	// Returns evaluation of king safety from an attack zone
	int Evaluator::kingAttackZone(uint64_t attackZone, uint64_t friendlyPieces, int enemyKingSquare, int opposingColor) {
		int evaluation = 0;
		uint64_t knights = friendlyPieces & board->knights;
		uint64_t pawns = friendlyPieces & board->pawns;
		uint64_t rooks = friendlyPieces & board->orthogonalPieces & ~board->diagonalPieces;
		uint64_t bishops = friendlyPieces & board->diagonalPieces & ~board->orthogonalPieces;
		uint64_t queens = rooks & bishops;

		// Sum number of moves in king attack zone multiplied by piece weightings
		evaluation += evalOrthMoves(attackZone, rooks, enemyKingSquare);
		evaluation += evalDiagMoves(attackZone, bishops, enemyKingSquare);
		evaluation += evalQueenMoves(attackZone, queens, enemyKingSquare);
		evaluation += evalPawnMoves(attackZone, pawns, opposingColor);
		evaluation += evalKnightMoves(attackZone, knights);

		evaluation = -kingZoneSafety[evaluation];

		return evaluation;
	}

	int Evaluator::incrementAttackZoneEval(uint64_t attackZone, uint64_t moves, const int piece) {
		uint64_t zoneMoves = attackZone & moves;
		int evaluation = 0;

		while (zoneMoves != 0ULL) {
			BitBoardUtility::popLSB(zoneMoves);
			evaluation += attackUnitScores[piece];
		}

		return evaluation;
	}

	int Evaluator::evalKnightMoves(uint64_t attackZone, uint64_t knights) {
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

	int Evaluator::evalPawnMoves(uint64_t attackZone, uint64_t pawns, const int opposingColor) {
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

	int Evaluator::evalOrthMoves(uint64_t attackZone, uint64_t orths, int enemyKingSquare) {
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

	int Evaluator::evalDiagMoves(uint64_t attackZone, uint64_t diags, int enemyKingSquare) {
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

	int Evaluator::evalQueenMoves(uint64_t attackZone, uint64_t queens, int enemyKingSquare) {
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

	// Return evaluation for open files
	int Evaluator::openFilesEvaluation() {
		// Endgame is less likely to require open files
		if (endGameWeight >= 0.3f)
			return 0;

		int evaluation = 0;

		// Iterate over each column to check for open file

		uint64_t orthogonalPieces = board->orthogonalPieces;

		while (orthogonalPieces != 0ULL) {
			int square = BitBoardUtility::LSB(orthogonalPieces);
			uint64_t colMask = precomputation->getColMask(square);

			if ((orthogonalPieces & colMask) == 0ULL)
				continue;

			uint64_t filePawns = board->pawns & colMask;
			int pawnCounter = 0;

			while (filePawns != 0ULL) {
				BitBoardUtility::popLSB(filePawns);
				pawnCounter++;
				if (pawnCounter > 1) {
					break;
				}
			}
			if (pawnCounter > 1) {
				orthogonalPieces &= ~colMask;
				continue;
			}
			evaluation += evaluateOpenFile(colMask, pawnCounter);

			orthogonalPieces &= ~colMask;
		}

		return evaluation;
	}
	// Evaluate an open file, returns evaluation of open file
	int Evaluator::evaluateOpenFile(uint64_t column, int pawnCounter) {
		int evaluation = 0;
		int square;
		// Start by analysing queens on file
		uint64_t OrthFile = column & board->orthogonalPieces & board->diagonalPieces;

		int whiteOrths = 0;
		int blackOrths = 0;

		while (OrthFile != 0ULL) {
			square = BitBoardUtility::popLSB(OrthFile);
			// White Queen
			if (board->whitePieces & (1ULL << square)) {
				evaluation += 0.7f * openFileBonus;
				whiteOrths += 1;
			}
			// Black Queen
			else {
				evaluation -= 0.7f * openFileBonus;
				blackOrths += 1;
			}
		}
		// Only rooks
		OrthFile = column & board->orthogonalPieces & ~board->diagonalPieces;

		while (OrthFile != 0ULL) {
			square = BitBoardUtility::popLSB(OrthFile);
			// White Rook
			if (board->whitePieces & (1ULL << square)) {
				evaluation += openFileBonus;
				whiteOrths += 1;
			}
			// Black Rook
			else {
				evaluation -= openFileBonus;
				blackOrths += 1;
			}
		}

		evaluation = pawnCounter == 0 ? evaluation : 0.2f * evaluation;

		if (openDiagFileNearKing(column, whiteKingSquare) && blackOrths != 0) {
			evaluation -= openFileNearKingBonus * blackOrths;
		}

		if (openDiagFileNearKing(column, blackKingSquare) && whiteOrths != 0) {
			evaluation += openFileNearKingBonus * whiteOrths;
		}

		return evaluation;
	}
	// Returns evaluation of open diagonals
	int Evaluator::openDiagEvaluation() {
		// Endgame is less likely to require open diags
		if (endGameWeight >= 0.3)
			return 0;
		// If open/semi-open diagonal has no diagonal pieces, unlikely to be advantageous
		if (board->diagonalPieces == 0ULL)
			return 0;

		int evaluation = 0;
		uint64_t forwardDiagMask;
		uint64_t backwardDiagMask;

		uint64_t forwardPawns;
		uint64_t backwardPawns;

		int pawnCounter;
		const int diagSize = 3;
		int forwardDiagSquares[3] = { 6, 7, 15 };
		int backwardDiagSquares[3] = { 8, 0, 1 };

		// Iterate over forward and backward diagonals and evaluate open diagonals
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

	// Evaluate an open diagonal, returns evaluation of the open diagonal
	int Evaluator::evaluateOpenDiag(uint64_t diag, int pawnCounter) {
		// If open/semi-open diagonal has no diagonal pieces, unlikely to be advantageous
		if ((diag & board->diagonalPieces) == 0ULL)
			return 0;

		int evaluation = 0;
		int square;
		// Start by analysing queens on file
		uint64_t diagPieces = diag & board->orthogonalPieces & board->diagonalPieces;

		int whiteDiags = 0;
		int blackDiags = 0;

		while (diagPieces != 0ULL) {
			square = BitBoardUtility::popLSB(diagPieces);
			// White Queen
			if (board->whitePieces & (1ULL << square)) {
				evaluation += 0.7f * openDiagBonus;
				whiteDiags += 1;
			}
			// Black Queen
			else {
				evaluation -= 0.7f * openDiagBonus;
				blackDiags += 1;
			}
		}
		// Only bishops
		diagPieces = diag & board->diagonalPieces & ~board->orthogonalPieces;

		while (diagPieces != 0ULL) {
			square = BitBoardUtility::popLSB(diagPieces);
			// White bishop
			if (board->whitePieces & (1ULL << square)) {
				evaluation += openDiagBonus;
				whiteDiags += 1;
			}
			// Black bishop
			else {
				evaluation -= openDiagBonus;
				blackDiags += 1;
			}
		}

		evaluation = pawnCounter == 0 ? evaluation : 0.2f * evaluation;
		// Evaluate whether open diagonal is near a king - impacts king safety
		if (openDiagFileNearKing(diag, whiteKingSquare) && blackDiags != 0) {
			evaluation -= openDiagNearKingBonus * blackDiags;
		}

		if (openDiagFileNearKing(diag, blackKingSquare) && whiteDiags != 0) {
			evaluation += openDiagNearKingBonus * whiteDiags;
		}

		return evaluation;
	}
	// Returns true if diagonal goes through area around king
	bool Evaluator::openDiagFileNearKing(uint64_t mask, int kingSquare) {
		return mask & (precomputation->getKingMoves(kingSquare));
	}
	// Initialises the value of pieces on the board. Calculating these initially allows progressive
	// incrementation of material value, instead of recalculating every evaluation
	void Evaluator::initPieceEvaluations() {
		whiteMaterial = 0;
		blackMaterial = 0;
		whiteStaticEval = staticPieceEvaluation(board->pieceLists[Board::whiteIndex], whiteMaterial);
		blackStaticEval = staticPieceEvaluation(board->pieceLists[Board::blackIndex], blackMaterial, true);
	}
	
	// Returns evaluation from static piece values. Need to do calculations for kings and pawns since
	// they are affected by endgame weight (which cannot be progressively applied like others)
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

		return whiteStaticEval + whiteEval - (blackStaticEval + blackEval);
	}
	// Calculate static evaluation of a certain side. Returns static evaluation of a side.
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

		int losingKingSquare = evaluation > 0 ? blackKingSquare : whiteKingSquare;

		int losingKingCMD = arrCenterManhattanDistance[losingKingSquare];

		int kingsMD = abs((whiteKingSquare / 8) - (blackKingSquare / 8)) + abs((whiteKingSquare % 8) - (blackKingSquare % 8));

		// From Chess 4.x
		mopUpScore = 4.7f * losingKingCMD + 1.6f * (14 - kingsMD);
		mopUpScore *= endGameWeight;

		// Discourage moving to outer edge of board if losing
		if (!board->state->whiteTurn) {
			mopUpScore *= -1;
		}

		return mopUpScore;
	}

	// Initialises end game weight by first calculating number of major and minor pieces
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

	// Calculates endgameweight using number of major and minor pieces on board
	void Evaluator::calculateEndgameWeight() {
		// Square min component to create smoother curve 
		// (pow function is inherently slower than variable * variable)
		endGameWeight = min(1.f, (whiteMMPieces + blackMMPieces) / endgameRequiredPieces);
		endGameWeight *= endGameWeight;
		endGameWeight = 1.f - endGameWeight;
	}

	// When applying a move to the board, compute the difference in static evaluation before the move is made
	void Evaluator::updateStaticEval(Board* board, Move move) {
		assert(board != nullptr);

		uint16_t startSquare = move.getStartSquare();
		uint16_t targetSquare = move.getTargetSquare();

		int piece = Piece::type(board->squares[startSquare]);
		staticPieceMove(piece, startSquare, targetSquare, board->state->whiteTurn);

		if (board->squares[targetSquare] != Piece::empty) {
			int deletedPiece = Piece::type(board->squares[targetSquare]);
			staticPieceDelete(deletedPiece, targetSquare, board->state->whiteTurn);
		}

		switch (move.getFlag()) {
			case Move::noFlag:
				return;
			case Move::enPassantCaptureFlag:
				break;
			case Move::castleFlag:
				break;
			default: // Promotion Moves
				staticPieceDelete(Piece::pawn, targetSquare, board->state->whiteTurn); // !whiteTurn?????????? see abt it
				switch (move.getFlag()) {
				case Move::promoteToBishopFlag:
					staticPieceSpawn(Piece::bishop, targetSquare, board->state->whiteTurn);
					break;
				case Move::promoteToKnightFlag:
					staticPieceSpawn(Piece::knight, targetSquare, board->state->whiteTurn);
					break;
				case Move::promoteToQueenFlag:
					staticPieceSpawn(Piece::queen, targetSquare, board->state->whiteTurn);
					break;
				case Move::promoteToRookFlag:
					staticPieceSpawn(Piece::rook, targetSquare, board->state->whiteTurn);
					break;
				}
				break;
		}
	}

	// When undoing a move on the board, compute the difference in static evaluation before the move is made
	void Evaluator::undoStaticEval(Board* board, Move move) {
		assert(board != nullptr);
		staticPieceMove()
	}

	// Adjusts the static evaluation for when a piece moves on the board
	void Evaluator::staticPieceMove(const int piece, int startSquare, int targetSquare, const bool whiteTurn) {
		int* movingSide = whiteTurn ? &whiteStaticEval : &blackStaticEval;
		if (!whiteTurn) {
			startSquare = blackEvalSquare[startSquare];
			targetSquare = blackEvalSquare[targetSquare];
		}
		// Remove old evaluation from piece, and add new evaluation from piece (as a result of new position)
		*movingSide += PieceEvaluations::pieceEvals[piece][targetSquare] - PieceEvaluations::pieceEvals[piece][startSquare];
	}
	// When a piece is captured on the board, delete its value and position value from the static evaluation
	void Evaluator::staticPieceDelete(const int piece, int square, const bool whiteTurn) {
		int* otherSide = whiteTurn ? &whiteStaticEval : &blackStaticEval;

		*otherSide -= PieceEvaluations::pieceVals[piece];
		if (!whiteTurn) {
			square = blackEvalSquare[square];
		}
		switch (piece) {
		// If piece is nothing, do nothing
		case Piece::empty:
			break;
		// Remove piece's positional value
		default:
			*otherSide -= PieceEvaluations::pieceEvals[piece][square];
			break;
		}
		// Remove piece's material value
		int* material = whiteTurn ? &whiteMaterial : &blackMaterial;
		*material -= PieceEvaluations::pieceVals[piece];

		// If piece to be deleted is a major or minor piece, update major minor pieces
		if (piece > Piece::pawn && piece < Piece::king) {
			int* MMMaterial = whiteTurn ? &whiteMMPieces : &blackMMPieces;
			*MMMaterial -= 1;
			calculateEndgameWeight();
		}
	}
	// Add static evaluation. Occurs when either captured piece comes back, or via promotion
	void Evaluator::staticPieceSpawn(const int piece, int square, const bool whiteTurn) {
		int* movingSide = whiteTurn ? &whiteStaticEval : &blackStaticEval;
		// Add value of piece to static eval
		*movingSide += PieceEvaluations::pieceVals[piece];
		if (!whiteTurn) {
			square = blackEvalSquare[square];
		}
		// Add positional value
		*movingSide += PieceEvaluations::pieceEvals[piece][square];
		// Add value to material
		int* material = whiteTurn ? &whiteMaterial : &blackMaterial;
		*material += PieceEvaluations::pieceVals[piece];

		// If piece is major or minor piece, update variables
		if (piece > Piece::pawn && piece < Piece::king) {
			int* MMMaterial = whiteTurn ? &whiteMMPieces : &blackMMPieces;
			*MMMaterial += 1;
			calculateEndgameWeight();
		}

	}

}