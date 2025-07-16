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
		static constexpr int checkMateScore{ 100000 };
		static constexpr int drawScore{ 0 };
		static constexpr int cancelledScore{ 0 };

		Evaluator() {};

		int Evaluate(Board* board);
		bool insufficientMaterial();
		static bool isMateScore(int score);
		static int movesTilMate(int score);
	private:
		// Contains useful data which can be calculated on instantiation
		Board* board{ nullptr };

		static constexpr float endgameRequiredPieces{ 7.f }; // Number of pieces which define start of endgame phase
		static constexpr int startRow[2]{ 0, 7 }; // Start rows of [black, white]
		// Center manhattan-distance from https://www.chessprogramming.org/Center_Manhattan-Distance
		static constexpr uint8_t arrCenterManhattanDistance[64]{
			6, 5, 4, 3, 3, 4, 5, 6,
			5, 4, 3, 2, 2, 3, 4, 5,
			4, 3, 2, 1, 1, 2, 3, 4,
			3, 2, 1, 0, 0, 1, 2, 3,
			3, 2, 1, 0, 0, 1, 2, 3,
			4, 3, 2, 1, 1, 2, 3, 4,
			5, 4, 3, 2, 2, 3, 4, 5,
			6, 5, 4, 3, 3, 4, 5, 6
		};
		// Attack unit table for king safety from Stockfish
		static constexpr int kingZoneSafety[100]{
				0,   0,   1,   2,   3,   5,   7,   9,  12,  15,
			   18,  22,  26,  30,  35,  39,  44,  50,  56,  62,
			   68,  75,  82,  85,  89,  97, 105, 113, 122, 131,
			  140, 150, 169, 180, 191, 202, 213, 225, 237, 248,
			  260, 272, 283, 295, 307, 319, 330, 342, 354, 366,
			  377, 389, 401, 412, 424, 436, 448, 459, 471, 483,
			  494, 500, 500, 500, 500, 500, 500, 500, 500, 500,
			  500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
			  500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
			  500, 500, 500, 500, 500, 500, 500, 500, 500, 500
		};

		// Weightings of evaluation criteria
		static constexpr unsigned char passedPawnBonus[7]{ 0, 90, 60, 40, 25, 15, 15 };
		static constexpr float tropismWeightings[7]{ 0.f, 0.2f, 0.5f, 0.5f, 1.f, 2.f, 0.f };
		static constexpr float attackUnitScores[7]{ 0, 0.5f, 2, 2, 3, 5, 0 };
		static constexpr unsigned char pawnIslandPenalty{ 30 };
		static constexpr unsigned char pawnShieldColumnPenalty{ 30 };
		static constexpr unsigned char pawnShieldUndefendedPenalty{ 30 };
		static constexpr float kingSafetyCoefficient{ 0.04f };

		static constexpr unsigned char openFileBonus{ 20 };
		static constexpr unsigned char openFileNearKingBonus{ 40 };

		static constexpr unsigned char openDiagBonus{ 20 };
		static constexpr unsigned char openDiagNearKingBonus{ 40 };

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