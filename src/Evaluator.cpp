#include "Evaluator.h"

#include "Bitboards.h"

#include <cassert>
#include <iostream>

namespace SandalBot {

	// Returns true if insufficient material, false otherwise
	bool Evaluator::insufficientMaterial() {
		// Impossible for insufficient material if not endgame (many pieces on board)
		if (endGameWeight <= 0.05f) {
			return false;
		}

		int whiteMaterial = board->sideValues[WHITE];
		int blackMaterial = board->sideValues[BLACK];

		// If there are only kings, insufficient
		if (whiteMaterial == 0 && blackMaterial == 0) {
			return true;
		}
		// If there are pawns on board, sufficient material
		if (board->typesBB[PAWN] != 0ULL) {
			return false;
		}
		// If there is rook or queen, sufficient material
		if ((board->typesBB[ROOK] | board->typesBB[QUEEN]) != 0ULL) {
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

	// Calculates endgameweight using number of major and minor pieces on board
	void Evaluator::calculateEndgameWeight() {
		// Square min component to create smoother curve 
		endGameWeight = std::min(1.f, (board->MMPieces[WHITE] + board->MMPieces[BLACK]) / endgameRequiredPieces);
		endGameWeight *= endGameWeight;
		endGameWeight = 1.f - endGameWeight;
	}

	// Returns an integer representing the static evaluation of the board's position
	// Negative return value signifies losing position, zero is a draw, and positive is winning
	int Evaluator::Evaluate(Board* board) {
		assert(board != nullptr);
		this->board = board;

		calculateEndgameWeight();

		int evaluation{ 0 };

		// If insufficient material its a draw
		if (insufficientMaterial()) {
			return drawScore;
		}

		evaluation += evaluateSide<WHITE>();
		evaluation -= evaluateSide<BLACK>();

		evaluation += openFilesEvaluation();
		evaluation += openDiagEvaluation();
		
		evaluation += kingDist(evaluation);

		return board->sideToMove() ? evaluation : -evaluation;
	}

	template <Color Us>
	int Evaluator::evaluateSide() {
		int evaluation{ 0 };

		evaluation += staticPieceEvaluation<Us>();
		evaluation += pawnIslandEvaluation<Us>();
		evaluation += passedPawnEvaluation<Us>();
		evaluation += kingSafety<Us>();

		return evaluation;
	}

	// Returns true if score is a checkmate score
	bool Evaluator::isMateScore(int score) {
		return abs(score) >= checkMateScore / 2;
	}

	// Returns moves until checkmate from a given score
	int Evaluator::movesTilMate(int score) {
		if (!isMateScore(score))
			return 0;
		return std::max(1, ((checkMateScore - abs(score) - 1) / 2) + 1);
	}

	// Returns the evaluation regarding king safety
	template <Color Us>
	int Evaluator::kingSafety() {
		int evaluation{ 0 };

		// If no major minor pieces, king safety is not too much of a factor
		if (board->MMPieces[WHITE] == 0 || board->MMPieces[BLACK] == 0) {
			return evaluation;
		}

		evaluation += pawnShieldEvaluation<Us>();
		//evaluation += kingTropismEvaluation<Us>();

		evaluation = evaluation * kingSafetyCoefficient;

		//evaluation += kingAttackZone<Us>();

		return evaluation;
	}

	// Calculate king tropism for one side
	template <Color Us>
	int Evaluator::kingTropism() {
		Square kingSq = board->kingSquares[Us];
		Bitboard otherSideBB = board->colorsBB[~Us];
		int evaluation = 0;

		while (otherSideBB != 0ULL) {
			Square sq = popLSB(otherSideBB);
			PieceType type = typeOf(board->squares[sq]);

			evaluation -= (7 - getDistance(kingSq, sq)) * tropismWeightings[type];
		}

		return board->sideValues[~Us] * evaluation;
	}

	// Calculate pawn shield evaluation for one side
	template <Color Us>
	int Evaluator::pawnShieldEvaluation() {
		if (endGameWeight >= 0.3f) {
			return 0;
		}

		Square kSq = board->kingSquares[Us];
		Bitboard shieldMask = getShieldMask<Us>(kSq);
		Bitboard pawns = board->typesBB[PAWN] & board->colorsBB[Us] & shieldMask;

		int evaluation{ 0 };
		Bitboard kingSquares = getMovementBoard<KING>(kSq, 0ULL);

		Square col = kSq;

		// If there is no pawn shield on a column, add penalty
		if ((pawns & getColMask(col)) == 0ULL) {
			evaluation -= pawnShieldColumnPenalty;
		}
		// Logic pertains to include edge file shields
		if (col == 7) {
			col = Square(kSq - 2);
		} else {
			col = Square(col + 1);
		}


		if ((pawns & getColMask(col)) == 0ULL) {
			evaluation -= pawnShieldColumnPenalty;
		}

		if (col == 0) {
			col = Square(kSq + 2);
		} else {
			col = Square(kSq - 1);
		}

		if ((pawns & getColMask(col)) == 0ULL) {
			evaluation -= pawnShieldColumnPenalty;
		}

		// Undefended pawns have penalty (undefended by king and/or other pawns)
		while (pawns != 0ULL) {
			Square to = popLSB(pawns);
			Bitboard attackMask = getPawnAttackMoves<WHITE>(to);
			attackMask |= getPawnAttackMoves<BLACK>(to);

			Bitboard pawnDefenders = pawns & attackMask;

			if (pawnDefenders == 0ULL && (kingSquares & (1ULL << to)) == 0ULL) {
				evaluation -= pawnShieldUndefendedPenalty;
			}
		}

		evaluation *= (board->sideValues[~Us] / PieceEvaluations::pawnVal);
		evaluation *= (1.f - endGameWeight);

		return evaluation;
	}

	// Evaluates the passed pawns of either side
	template <Color Us>
	int Evaluator::passedPawnEvaluation() {
		Bitboard usPawns = board->typesBB[PAWN] & board->colorsBB[Us];
		Bitboard opposingPawns = board->typesBB[PAWN] & board->colorsBB[~Us];
		int evaluation = 0;

		while (usPawns != 0ULL) {
			Square sq = popLSB(usPawns);
			Bitboard passedMask = getPassedPawnMask<Us>(sq);

			// If no pawns in front of pawn's promotion path
			if ((passedMask & opposingPawns) == 0ULL) {
				// Squares from end of board
				int promotionDistance = Us == WHITE ? toRow(sq) : 7 - toRow(sq);

				evaluation += passedPawnBonus[promotionDistance];
			}
		}

		return evaluation;
	}

	// Returns evaluation of pawn islands from one side
	template <Color Us>
	int Evaluator::pawnIslandEvaluation() {
		Bitboard pawns = board->typesBB[PAWN] & board->colorsBB[Us];
		int evaluation = 0;

		Bitboard tempPawns = pawns;
		while (tempPawns != 0ULL) {
			Square sq = popLSB(tempPawns);
			Bitboard islandMask = getPawnIslandMask(toCol(sq));

			if ((pawns & islandMask) != 0ULL)
				continue;

			evaluation -= pawnIslandPenalty;
		}

		return evaluation;
	}

	// Returns evaluation of king safety from an attack zone
	template <Color Us>
	int Evaluator::kingAttackZone() {
		Bitboard usPieces = board->colorsBB[Us];
		Square usKSq = board->kingSquares[Us];
		
		Bitboard attackZone = (abs(toRow(usKSq) - startRow[WHITE]) >= 2 || endGameWeight >= 0.2f)
			? getUnbiasKingAttackZone(usKSq) : getKingAttackSquare<Us>(usKSq);

		int evaluation = 0;

		Bitboard knights = usPieces & board->typesBB[KNIGHT];
		Bitboard pawns = usPieces & board->typesBB[PAWN];
		Bitboard rooks = usPieces & board->typesBB[ROOK];
		Bitboard bishops = usPieces & board->typesBB[BISHOP];
		Bitboard queens = rooks & bishops;

		// Sum number of moves in king attack zone multiplied by piece weightings
		evaluation += evalMoves<Us, QUEEN>(attackZone);
		evaluation += evalMoves<Us, ROOK>(attackZone);
		evaluation += evalMoves<Us, BISHOP>(attackZone);
		evaluation += evalMoves<Us, KNIGHT>(attackZone);
		evaluation += evalPawnMoves<Us>(attackZone);

		evaluation = -kingZoneSafety[evaluation];

		return evaluation;
	}

	int Evaluator::incrementAttackZoneEval(Bitboard attackZone, Bitboard moves, PieceType piece) {
		Bitboard zoneMoves = attackZone & moves;
		int evaluation = 0;

		while (zoneMoves != 0ULL) {
			popLSB(zoneMoves);
			evaluation += attackUnitScores[piece];
		}

		return evaluation;
	}

	template <Color Us, PieceType Type>
	int Evaluator::evalMoves(Bitboard attackZone) {
		Bitboard pieces = board->typesBB[Type] & board->colorsBB[Us];
		Square themKSq = board->kingSquares[~Us];
		Bitboard allPieces = board->typesBB[ALL_PIECES];
		allPieces &= ~(1ULL << themKSq);

		int evaluation = 0;

		while (pieces != 0ULL) {
			Square from = popLSB(pieces);
			Bitboard movementBB = getMovementBoard<Type>(from, allPieces);
			evaluation += incrementAttackZoneEval(attackZone, movementBB, Type);
		}

		return evaluation;
	}

	template <Color Us>
	int Evaluator::evalPawnMoves(Bitboard attackZone) {
		Bitboard pawns = board->typesBB[PAWN] & board->colorsBB[Us];
		int evaluation = 0;

		while (pawns != 0ULL) {
			Square from = popLSB(pawns);
			Bitboard movementBB = getPawnAttackMoves<~Us>(from);
			evaluation += incrementAttackZoneEval(attackZone, movementBB, PAWN);
		}

		return evaluation;
	}

	// Return evaluation for open files
	int Evaluator::openFilesEvaluation() {
		// Endgame is less likely to require open files
		if (endGameWeight >= 0.3f)
			return 0;

		if ((board->typesBB[ROOK] | board->typesBB[QUEEN]) == 0ULL)
			return 0;

		int evaluation = 0;

		// Iterate over each column to check for open file

		Bitboard orthogonalPieces = board->typesBB[ROOK] | board->typesBB[QUEEN];

		while (orthogonalPieces != 0ULL) {
			Square square = LSB(orthogonalPieces);
			Bitboard colMask = getColMask(square);

			if ((orthogonalPieces & colMask) == 0ULL)
				continue;

			Bitboard filePawns = board->typesBB[PAWN] & colMask;
			int pawnCounter = 0;

			while (filePawns != 0ULL) {
				popLSB(filePawns);
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
	int Evaluator::evaluateOpenFile(Bitboard column, int pawnCounter) {
		int evaluation = 0;
		Square square;
		// Start by analysing queens on file
		Bitboard OrthFile = column & board->typesBB[QUEEN];

		int whiteOrths = 0;
		int blackOrths = 0;

		while (OrthFile != 0ULL) {
			square = popLSB(OrthFile);
			// White Queen
			if (board->colorsBB[WHITE] & (1ULL << square)) {
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
		OrthFile = column & board->typesBB[ROOK];

		while (OrthFile != 0ULL) {
			square = popLSB(OrthFile);
			// White Rook
			if (board->colorsBB[WHITE] & (1ULL << square)) {
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

		if (openDiagFileNearKing(column, board->kingSquares[WHITE]) && blackOrths != 0) {
			evaluation -= openFileNearKingBonus * blackOrths;
		}

		if (openDiagFileNearKing(column, board->kingSquares[WHITE]) && whiteOrths != 0) {
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
		if ((board->typesBB[BISHOP] | board->typesBB[QUEEN]) == 0ULL)
			return 0;

		int evaluation = 0;

		
		constexpr int diagSize = 3;
		constexpr Square forwardDiagSquares[diagSize] = { G8, H8, H7 };
		constexpr Square backwardDiagSquares[diagSize] = { A7, A8, B8 };

		// Iterate over forward and backward diagonals and evaluate open diagonals
		for (int it = 0; it < diagSize; it++) {
			Bitboard forwardDiagMask = getForwardMask(forwardDiagSquares[it]);

			Bitboard forwardPawns = board->typesBB[PAWN] & forwardDiagMask;

			int pawnCounter = 0;

			while (forwardPawns != 0ULL) {
				popLSB(forwardPawns);
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
			Bitboard backwardDiagMask = getBackwardMask(backwardDiagSquares[it]);

			Bitboard backwardPawns = board->typesBB[PAWN] & backwardDiagMask;

			int pawnCounter = 0;

			while (backwardPawns != 0ULL) {
				popLSB(backwardPawns);
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
	int Evaluator::evaluateOpenDiag(Bitboard diag, int pawnCounter) {
		// If open/semi-open diagonal has no diagonal pieces, unlikely to be advantageous
		if ((diag & (board->typesBB[BISHOP] | board->typesBB[QUEEN])) == 0ULL)
			return 0;

		int evaluation = 0;
		Square square;
		// Start by analysing queens on file
		Bitboard diagPieces = diag & board->typesBB[QUEEN];

		int whiteDiags = 0;
		int blackDiags = 0;

		while (diagPieces != 0ULL) {
			square = popLSB(diagPieces);
			// White Queen
			if (board->colorsBB[WHITE] & (1ULL << square)) {
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
		diagPieces = diag & board->typesBB[BISHOP];

		while (diagPieces != 0ULL) {
			square = popLSB(diagPieces);
			// White bishop
			if (board->colorsBB[WHITE] & (1ULL << square)) {
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
		if (openDiagFileNearKing(diag, board->kingSquares[WHITE]) && blackDiags != 0) {
			evaluation -= openDiagNearKingBonus * blackDiags;
		}

		if (openDiagFileNearKing(diag, board->kingSquares[BLACK]) && whiteDiags != 0) {
			evaluation += openDiagNearKingBonus * whiteDiags;
		}

		return evaluation;
	}

	// Returns true if diagonal goes through area around king
	bool Evaluator::openDiagFileNearKing(Bitboard mask, Square kingSquare) {
		return mask & (getMovementBoard<KING>(kingSquare, 0ULL));
	}
	
	// Returns evaluation from static piece values. Need to do calculations for kings and pawns since
	// they are affected by endgame weight (which cannot be progressively applied like others)
	template<Color Us>
	int Evaluator::staticPieceEvaluation() {
		int evaluation = 0;
		// Since endgame-based evaluations cannot be progressively evaluated,
		// delete square evaluations of pawns and kings, then add interpolation of endgame square values

		if (endGameWeight > 0.f) {
			Bitboard pawns = board->typesBB[PAWN] & board->colorsBB[Us];

			while (pawns != 0ULL) {
				Square square = popLSB(pawns);
				if (Us == BLACK) {
					square = flipRow(square);
				}
				evaluation -= PieceEvaluations::pawnEval[square];
				evaluation += (1 - endGameWeight) * PieceEvaluations::pawnEval[square] + endGameWeight * PieceEvaluations::pawnEndgameEval[square];
			}

			Square kSq = board->kingSquares[Us];
			if (Us == BLACK) {
				kSq = flipRow(kSq);
			}

			evaluation -= PieceEvaluations::kingEval[kSq];
			evaluation += (1 - endGameWeight) * PieceEvaluations::kingEval[kSq] + endGameWeight * PieceEvaluations::kingEndgameEval[kSq];
		}

		return evaluation + board->sideValues[Us] + board->pieceSquareValues[Us];
	}

	// Calculates mopup evaluation
	int Evaluator::kingDist(int currentEvaluation) {
		// If not an endgame or evaluation is too tight, dont bother with mopup evaluation
		if (endGameWeight == 0.f || abs(currentEvaluation) < 2 * PieceEvaluations::pieceVals[PAWN]) {
			return 0;
		}

		int mopUpScore = 0;

		Square wKSq = board->kingSquares[WHITE];
		Square bKSq = board->kingSquares[BLACK];

		Square losingKingSquare = currentEvaluation > 0 ? bKSq : wKSq;

		int losingKingCMD = arrCenterManhattanDistance[losingKingSquare];

		int kingsMD = abs(toRow(wKSq) - toRow(bKSq)) + abs(toCol(wKSq) - toCol(bKSq));

		// From Chess 4.x
		mopUpScore = 4.7f * losingKingCMD + 1.6f * (14 - kingsMD);
		mopUpScore *= endGameWeight;

		// Discourage moving to outer edge of board if losing
		if (board->sideToMove() == BLACK) {
			mopUpScore *= -1;
		}

		return mopUpScore;
	}

}