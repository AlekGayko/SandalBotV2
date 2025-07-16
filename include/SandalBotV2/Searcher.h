#ifndef SEARCHER_H
#define SEARCHER_H

#include "Board.h"
#include "Evaluator.h"
#include "MoveGen.h"
#include "MoveLine.h"
#include "MoveOrderer.h"
#include "TranspositionTable.h"
#include "Types.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <limits>
#include <mutex>

namespace SandalBot {

	// Searcher is responsible for searching game tree of chess positions, and encapsulates
	// the algorithmic design of the bot. Supports asynchronous searching, and synchronous
	// searching.
	class Searcher {
	public:
		Evaluator* evaluator{ nullptr };
		MoveGen* moveGenerator{ nullptr };
		MoveOrderer* orderer{ nullptr };
		Move bestMove{};

		Searcher() {};
		Searcher(Board* board);
		~Searcher();
		void startSearch(bool isTimed, int moveTimeMs = 0);
		void endSearch();
		int eval();
		uint64_t perft(int depth);
		void clearHash();
		void changeHashSize(int sizeMB);
	private:
		// SearchStatistics encapsulates the statistics from a search iteration
		struct SearchStatistics {
			Move bestMove{}; // Best found move
			uint64_t qNodes{}; // Number of quiescence search nodes
			uint64_t nNodes{}; // Number of Regular search nodes
			int depth{}; // Standard depth of search
			int seldepth{}; // Maximum selective depth of search
			int eval{}; // Evaluation of position
			uint64_t duration{}; // Duration of search

			std::string prepareEval();
			void printIteration();
			void print(Searcher* searcher);
		};
		const Move nullMove{}; // 'Null' move, represents uninitialised move to compare to

		SearchStatistics stats{}; // Statistics of most recent search

		std::atomic<bool> cancelSearch{ false }; // Atomic boolean indicates if search has been cancelled
		// Atomic boolean indicates if search has completed prematurely (checkmate)
		std::atomic<bool> searchCompleted{ false };
		std::mutex searchMutex; // Used to lock searchStop
		std::condition_variable searchStop; // Conditional variable waits to synchronise class during search

		static constexpr int searchWaitPeriod{ 100 }; // Sleep time for sleeping thread
		static constexpr int maxDeepening{ 256 }; // Maximum iterative deepening depth
		static constexpr int reduceExtensionCutoff{ 3 }; // Move array index where depth is reduced
		static constexpr int maxExtensions{ 16 }; // Maximum number of extensions during search
		static constexpr int bestLineSize{ maxDeepening + maxExtensions + 1 };

		MoveLine* bestLine{ nullptr };

		Board* board{ nullptr };

		TranspositionTable* tTable{ nullptr }; // Store previously evaluated positions

		Move currentMove{};

		// Using min cannot be negated due to two complement range
		static constexpr int defaultAlpha{ std::numeric_limits<int>::min() + 1 };
		static constexpr int defaultBeta{ std::numeric_limits<int>::max() };

		void iterativeSearch();
		int negaMax(int alpha, int beta, int depth, int maxDepth, int numExtensions);
		uint64_t moveSearch(int depth, int maxDepth);
		int quiescenceSearch(int alpha, int beta, int maxDepth);
		bool worthSearching(Move move, const bool isCheck, const int numExtensions);
		void moveSleep(int moveTimeMs);
		void generateBestLine(Move bestMove);
		void enactBestLine(Move move, int depth);
		bool isPositionIllegal();
	};

}

#endif // !SEARCHER_H
