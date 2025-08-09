#include "Searcher.h"

#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <string>
#include <string_view>

using namespace std;

namespace SandalBot {

	using namespace std::literals::string_view_literals;

	// Constructor initialised with board
	Searcher::Searcher(Board* board) : board(board) {
		// Allocate member variables
		this->evaluator = Evaluator();
		this->tTable = TranspositionTable();
		this->bestLine = MoveLine(bestLineSize);

		for (int i = 0; i < stack.size(); ++i) {
			SearchStack& ss = stack[i];
			ss.ply = i;
		}
	}

	// Performs iterative deepening, iteratively searches deeper and deeper for more intelligent
	// searches. Information gained and stored in transposition table aid further searches tremendously
	// and allow timed search instead of fixed depth/time search
	void Searcher::iterativeSearch() {
		// Initialise moves and statistics of search
		bestMove = Move();
		currentMove = Move();
		SearchStatistics temp;

		// If board position is illegal, do not search
		if (isPositionIllegal()) {
			return;
		}

		// Perform search for each depth until maximum depth
		for (int depth = 1; depth < maxDeepening; depth++) {
			// Peform negamax search of position and time it
			auto start = chrono::high_resolution_clock::now();
			stats = SearchStatistics();

			int eval = negaMax(&stack[0], defaultAlpha, defaultBeta, depth, 0);

			auto end = chrono::high_resolution_clock::now();
			chrono::duration<uint64_t, nano> duration = end - start;

			// If search is not cancelled, update stats
			if (!cancelSearch.load()) {
				bestLine.reset();
				generateBestLine(currentMove);
				bestMove = currentMove;
				temp = stats;
				temp.bestMove = currentMove;
				temp.depth = depth;
				temp.eval = eval;
				temp.duration = duration.count();

				temp.print(this);
			}
			// If search is cancelled, stop iterative deepening
			if (cancelSearch.load()) {
				break;
			} 
			// If checkmate has been found, stop search early
			else if (isMateScore(eval)) {
				searchCompleted.store(true);
				searchStop.notify_all();
				break;
			}
		}
		// If search ended before it was cancelled, notify other threads
		if (!cancelSearch.load() && !searchCompleted.load()) {
			searchCompleted.store(true);
			searchStop.notify_all();
		}

		// Update stats
		stats = temp;
	}

	// Quienscence search searches position by only considering moves which capture,
	// reduces horizon effect by preventing incredibly inaccurate evaluations from capture
	// sequences
	int Searcher::quiescenceSearch(SearchStack* const ss, int alpha, int beta, int depth) {
		stats.qNodes++; // Update stats
		if (cancelSearch) {
			return SCORE_NULL;
		}

		// Check for threefold repetition
		if (board->history.contains(board->state->zobristHash)) {
			return SCORE_DRAW;
		}
		// Check for draw by insufficient material
		else if (evaluator.insufficientMaterial()) {
			return SCORE_DRAW;
		}

		// If position has been previously stored, use its evaluation
		Entry* ttEntry = tTable.getEntry(board->state->zobristHash, ss->ttHit);
		Move ttMove = ss->ttHit ? ttEntry->move : Move();
		int ttEval = TranspositionTable::ttToEval(ttEntry->eval, ss->ply);

		if (ss->ttHit 
			&& (ttEntry->bound == EXACT 
			|| (ttEntry->bound == UPPER_BOUND && ttEval <= alpha)
			|| (ttEntry->bound == LOWER_BOUND && ttEval >= beta))) {
			return ttEval;
		}

		int bestScore{ 0 };
		Move bestMove = Move();

		// Evaluate board
		bestScore = evaluator.Evaluate(board);

		// If evaluation is too good, cut search
		if (bestScore >= beta) {
			return beta;
		}

		if (bestScore > alpha) {
			alpha = bestScore;
		}

		MoveGen moveGenerator = MoveGen(board, Move(), depth);
		Move move;

		while (!(move = moveGenerator.getMove()).isNull()) {
			if (!board->legalMove(move)) {
				continue;
			}

			bool givesCheck = board->givesCheck(move);

			// Make move
			board->makeMove(move, givesCheck);
			// Recursively search
			int score = -quiescenceSearch(ss + 1, -beta, -alpha, depth - 1);
			// Undo move
			board->unMakeMove();

			// If search cancelled, dont store move and return
			if (cancelSearch)
				return SCORE_NULL;

			if (score > bestScore) {
				bestScore = score;
				bestMove = move;
				// Update best bestScore
				if (bestScore > alpha) {
					alpha = bestScore;
				}
			}
			// If move is too good, return premateruely
			if (bestScore >= beta) {
				bestScore = beta;
				break;
			}

		}

		// Store move
		tTable.store(bestScore, depth, ss->ply, bestScore >= beta ? LOWER_BOUND : !bestMove.isNull() ? EXACT : UPPER_BOUND, bestMove, board->state->zobristHash);

		return bestScore;
	}

	// Negamax recursively searches future positions using alpha-beta pruning and
	// several heuristics to reduce search space
	int Searcher::negaMax(SearchStack* const ss, int alpha, int beta, int depth, int numExtensions) {
		stats.nNodes++;
		if (cancelSearch) {
			return SCORE_NULL;
		}

		if (depth > 0) {
			// Check for threefold repetition
			// If depth == 0 is included, it will provide move a8a8 since bot uses two fold repetition
			if (board->history.contains(board->state->zobristHash)) {
				return SCORE_DRAW;
			}
			// Check for fifty move rule
			else if (board->state->fiftyMoveCounter >= 100) {
				return SCORE_DRAW;
			} else if (evaluator.insufficientMaterial()) {
				return SCORE_DRAW;
			}

			alpha = max(alpha, -checkmateScore(ss->ply));
			beta = min(beta, checkmateScore(ss->ply - 1));

			if (alpha >= beta) {
				return alpha;
			}
		}

		Entry* ttEntry = tTable.getEntry(board->state->zobristHash, ss->ttHit);
		Move ttMove = ss->ttHit ? ttEntry->move : Move();
		int ttEval = TranspositionTable::ttToEval(ttEntry->eval, ss->ply);

		// If position found in transposition hash table, use previous evaluation
		if (ss->ttHit && (ttEntry->depth >= depth || isMateScore(ttEntry->eval)) 
			&& (ttEntry->bound == EXACT || (ttEntry->bound == UPPER_BOUND && ttEval <= alpha) 
				|| (ttEntry->bound == LOWER_BOUND && ttEval >= beta))) {
			int tTableDepth = ttEntry->depth;
			if (tTableDepth > stats.seldepth && tTableDepth != -1) {
				stats.seldepth = tTableDepth;
			}

			if (ss->ply == 0) {
				currentMove = ttMove;
			}

			return ttEval;
		}
		
		// If maximum depth is achieved, perform quiescence search
		if (depth <= 0) {
			if (ss->ply > stats.seldepth) {
				stats.seldepth = ss->ply;
			}
			return quiescenceSearch(ss, alpha, beta, depth);
		}

		//ss->staticEval = evaluator.Evaluate(board);

		int score = 0;
		int bestDepth = depth;
		int bestScore = SCORE_NEGATIVE_INFINITY;

		// Get best move (whether it be bestMove from iterative deepening or previous transpositions)
		Move bestMove = depth == 0 ? std::move(this->bestMove) : Move();

		// If position not in transposition table, reduce search depth
		if (ttMove.isNull() && depth >= 8 && !board->checkBB()) {
			--depth;
		}

		MoveGen moveGenerator = MoveGen(board, &ss->kMove, ttMove, depth);

		bool worthExtension = false;

		Move move;
		int moveNum = 0;

		while (!(move = moveGenerator.getMove()).isNull()) {
			if (!board->legalMove(move)) {
				moveNum++;
				continue;
			}

			bool givesCheck = board->givesCheck(move);

			int newDepth = depth - 1;
			int extension = 0;

			// Make move
			board->makeMove(move, givesCheck);
			bool fullSearch = true;
			
			worthExtension = worthSearching(move, givesCheck, numExtensions);

			// Reduce depth for moves late in move order as they are unlikely to be good
			if (moveNum >= 4 * reduceExtensionCutoff && depth >= 3 && !worthExtension) {
				score = -negaMax(ss + 1, -beta, -alpha, newDepth - 2, numExtensions);
				// If move is good do full search
				fullSearch = score > alpha;
			} else if (moveNum >= reduceExtensionCutoff && depth >= 2 && !worthExtension) {
				score = -negaMax(ss + 1, -beta, -alpha, newDepth - 1, numExtensions);
				// If move is good do full search
				fullSearch = score > alpha;
			}
			// If move is worth searching more, increase maxdepth for move
			if (worthExtension) {
				extension = 1;
			}
			// If reduced depth move is good, search it fully
			if (fullSearch) {
				newDepth += extension;
				score = -negaMax(ss + 1, -beta, -alpha, newDepth, numExtensions + extension);
			}

			// Undo move
			board->unMakeMove();

			moveNum++;

			// If search is cancelled, prevent processing move
			if (cancelSearch)
				return SCORE_NULL;

			if (score > bestScore) {
				bestScore = score;
				bestDepth = newDepth;
				bestMove = move;
				if (score > alpha) {
					alpha = score;
					if (ss->ply == 0 && !cancelSearch) {
						currentMove = move;
					}
				}
			}
			

			if (alpha >= beta) {
				// Store position
				bestScore = beta;
				// Update killer moves
				addKiller(ss, move);
				break;
			}
		}

		// If no moves, either checkmate or stalemate
		if (moveNum == 0) {
			if (board->checkBB() == 0ULL) {
				bestScore = SCORE_DRAW;
			} else {
				bestScore = -checkmateScore(ss->ply);
			}
		}

		// Store move
		tTable.store(bestScore, bestDepth, ss->ply, bestScore >= beta ? LOWER_BOUND : !bestMove.isNull() ? EXACT : UPPER_BOUND, bestMove, board->state->zobristHash);

		return bestScore;
	}

	// Movesearch purely searches using recursion. Used for perft command to test move generation
	uint64_t Searcher::moveSearch(int depth, int maxDepth) {
		if (depth == maxDepth) {
			return 1ULL;
		}

		// Generate moves
		uint64_t movesGenerated = 0ULL;

		MoveGen moveGenerator = MoveGen(board);
		Move move;

		while (!(move = moveGenerator.getMove()).isNull()) {
			bool givesCheck = board->givesCheck(move);
			if (!board->legalMove(move)) {
				continue;
			}
			uint64_t numMoves{ 0ULL }; // Tracks number of nodes found in perft
			// Simulate move
			board->makeMove(move, givesCheck);
			numMoves += moveSearch(depth + 1, maxDepth);
			board->unMakeMove();

			// If at depth 0, print number of moves from branch, useful for debugging
			if (depth == 0) {
				std::string_view promotionpiece = "";
				switch (move.flag()) {
				case Move::Flag::QUEEN:
					promotionpiece = "q"sv;
					break;
				case Move::Flag::ROOK:
					promotionpiece = "r"sv;
					break;
				case Move::Flag::BISHOP:
					promotionpiece = "b"sv;
					break;
				case Move::Flag::KNIGHT:
					promotionpiece = "n"sv;
					break;
				}
				cout << move.uciStr() << ": " << numMoves << endl;
			}
			movesGenerated += numMoves;
		}
		return movesGenerated;
	}

	// Sleeps recursively until search has completed or until moveTimeMs is up
	void Searcher::moveSleep(int moveTimeMs) {
		chrono::milliseconds runDurationMs{ moveTimeMs };
		auto stopTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
		stopTime += runDurationMs.count();

		// While search is ongoing, sleep until search completed or current time is exceeded stop time
		while (!searchCompleted.load()) {
			this_thread::sleep_for(searchWaitPeriod);
			auto now = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
			if (now > stopTime) {
				break;
			}
		}

		// Notify other threads that search is over
		cancelSearch.store(true);
		searchStop.notify_all();
	}

	// Starts the search on separate thread until time limit up
	Move Searcher::startSearch(bool isTimed, int moveTimeMs) {
		cancelSearch.store(false);
		searchCompleted.store(false);
		unique_lock<mutex> lock{ searchMutex }; // Lock for searchStop

		thread searchThread(&Searcher::iterativeSearch, this); // Begin search
		thread timerThread;
		// If search is timed, create thread which interrupts upon time limit
		if (isTimed) {
			timerThread = thread(&Searcher::moveSleep, this, moveTimeMs);
		}
		// Wait until search has completed (notified by either previous thread)
		searchStop.wait(lock, [this] { return this->cancelSearch.load(); });

		// Join previous threads
		if (isTimed) {
			timerThread.join();
		}

		searchThread.join();

		return bestMove;
	}

	// Cancels search
	void Searcher::endSearch() {
		searchCompleted.store(true);
		cancelSearch.store(true);
		searchStop.notify_all();
	}

	// Determines whether move is worth extending search for
	bool Searcher::worthSearching(Move move, const bool checkGiven, const int numExtensions) {
		return (move.isPromotion() || checkGiven) && numExtensions < maxExtensions;
	}

	// Generates best line
	void Searcher::generateBestLine(Move bestMove) {
		enactBestLine(bestMove); // Begin recursively generating
	}

	// Recursively searches the transposition table for best moves found (principal variation)
	// of current position
	void Searcher::enactBestLine(Move move) {
		if (move.moveValue == 0) {
			return;
		}

		bestLine.add(move); // Add move to bestline list

		// If threefold repetition, stop searching
		if (board->history.contains(board->state->zobristHash)) {
			return;
		}
		// Apply move to board
		board->makeMove(move);

		// Acquire next move from transposition table
		bool ttHit;
		Entry* ttEntry = tTable.getEntry(board->state->zobristHash, ttHit);

		if (ttHit) {
			enactBestLine(ttEntry->move);
		}

		board->unMakeMove(); // Rollback changes to board
	}

	// Checks whether the board position is illegal, used to prevent malicious FEN inputs
	bool Searcher::isPositionIllegal() {
		// If more or less than one king on each side, its an illegal position
		if (board->pieceCount[W_KING] != 1 || board->pieceCount[B_KING] != 1) {
			return true;
		}

		MoveGen moveGenerator = MoveGen(board);
		Move move;

		// If any move attack king, it is illegal
		while (!(move = moveGenerator.getMove()).isNull()) {
			if (typeOf(board->squares[move.to()]) == KING) {
				return true;
			}
		}

		return false;
	}

	// Returns static evaluation of position
	int Searcher::eval() {
		return evaluator.Evaluate(board);
	}

	// Deletes old transposition table and creates new one of different size
	void Searcher::changeHashSize(int sizeMB) {
		tTable = TranspositionTable(sizeMB);
	}

	// Clears all transposition table entries
	void Searcher::clearHash() {
		tTable.clear();
	}

	// Performs perft test
	uint64_t Searcher::perft(int depth) {
		return moveSearch(0, depth);
	}

	// Formats evaluation to string, accounting for checkmate scores as well
	std::string Searcher::SearchStatistics::prepareEval() {
		int movesRemaining = movesTilMate(eval);
		string sign = eval >= 0 ? "" : "-";
		// If checkmate
		if (movesRemaining != 0)
			return "mate " + sign + to_string(movesRemaining);

		return "cp " + to_string(eval); // centipawn eval
	}

	// Prints statistics of search. Prints in standard UCI format
	void Searcher::SearchStatistics::print(Searcher* searcher) {
		// Prevent division by zero
		if (duration == 0)
			duration = 1;

		string pv = searcher->bestLine.str(); // Principal variation
		cout << "info depth " << to_string(depth) << " seldepth " << to_string(seldepth);
		cout << " score " << prepareEval() << " nodes " << to_string(nNodes + qNodes);
		cout << " nps " << to_string(uint64_t(1000000000ULL * (nNodes + qNodes) / duration));
		cout << " hashfull " << to_string((int)(1000 * (float)searcher->tTable.getSlotsFilled() / (float)searcher->tTable.getSize()));
		cout << " time " << to_string(duration / 1000000ULL);

		// If principal variation exists, print it
		if (pv.size() != 0) {
			cout << " pv " << pv;
		}

		cout << endl;
	}

	// Add move to killer moves
	void Searcher::addKiller(SearchStack* ss, Move move) {
		ss->kMove.add(move);
	}

}