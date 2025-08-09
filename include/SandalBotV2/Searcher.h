#ifndef SEARCHER_H
#define SEARCHER_H

#include "Board.h"
#include "Evaluator.h"
#include "MoveGen.h"
#include "MoveLine.h"
#include "MoveOrderer.h"
#include "TranspositionTable.h"
#include "Types.h"

#include <array>
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
		struct SearchStack {
			int ply{ 0 };
			int staticEval{ SCORE_NULL };
			bool ttHit{ false };
			Killer kMove;
		};
	public:
		Evaluator evaluator{};

		Searcher() {};
		Searcher(Board* board);
		~Searcher() {}
		Move startSearch(bool isTimed, int moveTimeMs = 0);
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

		SearchStatistics stats{}; // Statistics of most recent search

		std::atomic<bool> cancelSearch{ false }; // Atomic boolean indicates if search has been cancelled
		// Atomic boolean indicates if search has completed prematurely (checkmate)
		std::atomic<bool> searchCompleted{ false };
		std::mutex searchMutex; // Used to lock searchStop
		std::condition_variable searchStop; // Conditional variable waits to synchronise class during search

		static constexpr std::chrono::milliseconds searchWaitPeriod{ 100 }; // Sleep time for sleeping thread
		static constexpr int maxDeepening{ 256 }; // Maximum iterative deepening depth
		static constexpr int reduceExtensionCutoff{ 3 }; // Move array index where depth is reduced
		static constexpr int maxExtensions{ 16 }; // Maximum number of extensions during search
		static constexpr int bestLineSize{ maxDeepening + maxExtensions + 1 };

		// Using min cannot be negated due to two complement range
		static constexpr int defaultAlpha{ SCORE_NEGATIVE_INFINITY };
		static constexpr int defaultBeta{ SCORE_INFINITY };

		std::array<SearchStack, maxDeepening + maxExtensions + 1> stack;

		MoveLine bestLine{};

		Board* board{ nullptr };

		TranspositionTable tTable{}; // Store previously evaluated positions

		Move currentMove{};
		Move bestMove{};

		void iterativeSearch();
		int negaMax(SearchStack* const ss, int alpha, int beta, int depth, int numExtensions);
		uint64_t moveSearch(int depth, int maxDepth);
		int quiescenceSearch(SearchStack* const ss, int alpha, int beta, int depth);
		bool worthSearching(Move move, const bool isCheck, const int numExtensions);
		void moveSleep(int moveTimeMs);
		void generateBestLine(Move bestMove);
		void enactBestLine(Move move);
		bool isPositionIllegal();

		void addKiller(SearchStack* ss, Move move);
	};

}

#endif // !SEARCHER_H
