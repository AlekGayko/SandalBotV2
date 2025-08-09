#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "Bitboards.h"
#include "Board.h"
#include "PieceEvaluations.h"
#include "MoveGen.h"

namespace SandalBot {

	// Evaluator is capable of evaluating a static board position based
	// on several factors including piece value, position value, passed pawns,
	// king safety, and open files/diagonals
	class Evaluator {
	public:
		Evaluator() {};

		int Evaluate(Board* board);
		bool insufficientMaterial();
	private:
		// Contains useful data which can be calculated on instantiation
		Board* board{ nullptr };

		float endGameWeight{};

		void calculateEndgameWeight();

		template <Color Us>
		int evaluateSide();

		template <Color Us>
		int staticPieceEvaluation();
		template <Color Us>
		int kingSafety();
		template <Color Us>
		int kingTropism();
		template <Color Us>
		int pawnShieldEvaluation();
		template <Color Us>
		int passedPawnEvaluation();
		template <Color Us>
		int pawnIslandEvaluation();
		template <Color Us>
		int kingAttackZone();
		int incrementAttackZoneEval(Bitboard attackZone, Bitboard moves, PieceType piece);
		template <Color Us>
		int mobilityEvaluation();
		template <Color Us, PieceType Type>
		int pieceMobility();

		int kingDist(int currentEvaluation);

		int openFilesEvaluation();
		int evaluateOpenFile(Bitboard column, int pawnCounter);

		int openDiagEvaluation();
		int evaluateOpenDiag(Bitboard diag, int pawnCounter);

		bool openDiagFileNearKing(Bitboard mask, Square kingSquare);

		template <Color Us, PieceType Type>
		int evalMoves(Bitboard attackZone);
		template <Color Us>
		int evalPawnMoves(Bitboard attackZone);

		bool insufficientMateMaterial(int material);
	};

}

#endif