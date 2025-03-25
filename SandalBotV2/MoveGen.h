#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "BitBoardUtility.h"
#include "Board.h"
#include "MovePrecomputation.h"
#include "CoordHelper.h"
#include "Piece.h"

namespace SandalBot {
	// MoveGen class generates all possible legalmoves in a given position
	class MoveGen {
		friend class MoveOrderer;
		friend class MovePrecomputation;
		friend class Searcher;
	private:
		Board* board = nullptr;

		const int whitePawnDirection{ -8 };
		const int blackPawnDirection{ 8 };

		const int promotionFlags[4] { Move::promoteToQueenFlag, Move::promoteToRookFlag, 
			Move::promoteToKnightFlag, Move::promoteToBishopFlag };

		uint64_t currentMoves{};
		int colorIndex{};
		int enemyColorIndex{};
		int currentColor{};
		int opposingColor{};
		bool doubleCheck{};
		bool generateCaptures{};
		int friendlyKingSquare{};
		int enemyKingSquare{};
		int* squares{ nullptr };

		bool whiteTurn{};
		int enPassantSquare{};
		int castlingRights{};
		int fiftyMoveCounter{};
		int moveCounter{};

		// Utility bitboards
		uint64_t opponentAttacks{};
		uint64_t checkBB{};
		uint64_t checkRayBB{};
		uint64_t friendlyBoard{};
		uint64_t enemyBoard{};

	public:
		MovePrecomputation* preComp{ nullptr };

		// Castling information
		static constexpr int startingKingSquares[2] { 4, 60 };
		static constexpr int shortCastleKingSquares[2] { 6, 62 };
		static constexpr int longCastleKingSquares[2] { 2, 58 };
		static constexpr int shortCastleRookSquares[2] { 7, 63 };
		static constexpr int longCastleRookSquares[2] { 0, 56 };
		static constexpr int shortCastleRookSpawn[2] { 5, 61 };
		static constexpr int longCastleRookSpawn[2] { 3, 59 };
		static constexpr int maxMoves{ 218 };

		bool isCheck{}; // Tracks whether there is check in position

		MoveGen();
		MoveGen(Board* board);
		~MoveGen();
		int generateMoves(Move moves[], bool capturesOnly = false);
		void initVariables(bool capturesOnly);
		void generateOrthogonalMoves(Move moves[]);
		void generateDiagonalMoves(Move moves[]);
		void generateKnightMoves(Move moves[]);
		void generateKingMoves(Move moves[]);
		void generatePawnMoves(Move moves[]);
		void enPassantMoves(Move moves[], int targetSquare, int startSquare);
		void promotionMoves(Move moves[], int targetSquare, int startSquare);
		void castlingMoves(Move moves[], int startSquare);

		uint64_t generateKnightAttackData(const uint64_t enemyBoard);
		uint64_t generatePawnAttackData(const uint64_t enemyBoard, const int opposingColor);
		uint64_t generateKingAttackData(const int enemyKingSquare);
		uint64_t generateOrthogonalAttackData(const uint64_t orthogonalPieces, const uint64_t enemyBoard, int friendlyKingSquare);
		uint64_t generateDiagonalAttackData(const uint64_t diagonalPieces, const uint64_t enemyBoard, int friendlyKingSquare);
		void generateAttackData();
		void generateCheckData();

		bool enPassantPin(int friendlyPawnSquare, int enemyPawnSquare);

		void updateResults(Move moves[]);
	};

}

#endif
