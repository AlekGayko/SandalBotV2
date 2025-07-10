#include "Searcher.h"

#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <string>
#include <sstream>

using namespace std;

namespace SandalBot {

	// Constructor initialised with board
	Searcher::Searcher(Board* board)
		: board(board) {
		// Allocate member variables
		this->moveGenerator = new MoveGen(board);
		this->orderer = new MoveOrderer(moveGenerator);
		this->evaluator = new Evaluator(board, moveGenerator->preComp);
		this->evaluator->generator = moveGenerator;
		this->tTable = new TranspositionTable();
		this->bestLine = new MoveLine(bestLineSize);
	}

	Searcher::~Searcher() {
		delete moveGenerator;
		delete orderer;
		delete tTable;
		delete evaluator;
		delete bestLine;
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
			int eval = negaMax(defaultAlpha, defaultBeta, 0, depth, 0);
			auto end = chrono::high_resolution_clock::now();
			chrono::duration<uint64_t, nano> duration = end - start;

			// If search is not cancelled, update stats
			if (!cancelSearch.load()) {
				bestLine->reset();
				generateBestLine(std::move(currentMove));
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
			else if (Evaluator::isMateScore(eval)) {
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
	int Searcher::quiescenceSearch(int alpha, int beta, int maxDepth) {
		stats.qNodes++; // Update stats
		if (cancelSearch) {
			return Evaluator::cancelledScore;
		}

		// Check for threefold repetition
		if (board->history.contains(board->state->zobristHash)) {
			return Evaluator::drawScore;
		} 
		// Check for draw by insufficient material
		else if (evaluator->insufficientMaterial()) {
			return Evaluator::drawScore;
		}

		// If position has been previously stored, use its evaluation
		int tTableEval = tTable->lookup(0, maxDepth, alpha, beta, board->state->zobristHash);
		if (tTableEval != TranspositionTable::notFound) {
			return tTableEval;
		}

		int score{ 0 };
		// Evaluate board
		score = evaluator->Evaluate();

		// If evaluation is too good, cut search
		if (score >= beta) {
			return beta;
		}

		if (score > alpha) {
			alpha = score;
		}

		// Initialise moves (218 is maximum number of moves)
		MovePoint moves[218];

		// Generate moves which only take pieces
		int numMoves = moveGenerator->generateMoves(moves, true);

		// Order the moves
		if (numMoves > 1) {
			orderer->order(board, moves, bestMove, numMoves, 0, true);
		}

		for (int i = 0; i < numMoves; i++) {
			// Make move
			board->makeMove(moves[i].move);
			// Recursively search
			score = -quiescenceSearch(-beta, -alpha, maxDepth + 1);
			// Undo move
			board->unMakeMove(moves[i].move);

			// If search cancelled, dont store move and return
			if (cancelSearch)
				return Evaluator::cancelledScore;

			// If move is too good, return premateruely
			if (score >= beta) {
				return beta;
			}
			// Update best score
			if (score > alpha) {
				alpha = score;
			}

		}

		return alpha;
	}

	// Negamax recursively searches future positions using alpha-beta pruning and
	// several heuristics to reduce search space
	int Searcher::negaMax(int alpha, int beta, int depth, int maxDepth, int numExtensions) {
		stats.nNodes++;
		if (cancelSearch) {
			return Evaluator::cancelledScore;
		}

		if (depth > 0) {
			// Check for threefold repetition
			// If depth == 0 is included, it will provide move a8a8 since bot uses two fold repetition
			if (board->history.contains(board->state->zobristHash)) {
				return Evaluator::drawScore;
			}
			// Check for fifty move rule
			else if (board->state->fiftyMoveCounter >= 100) {
				return Evaluator::drawScore;
			} else if (evaluator->insufficientMaterial()) {
				return Evaluator::drawScore;
			}

			alpha = max(alpha, -Evaluator::checkMateScore + depth);
			beta = min(beta, Evaluator::checkMateScore - depth);

			if (alpha >= beta) {
				return alpha;
			}
		}

		// Lookup position to see if it has been searched and stored in hashtable before
		int tTableEval = tTable->lookup(maxDepth - depth, depth, alpha, beta, board->state->zobristHash);
		// If position found in transposition hash table, use previous evaluation
		if (tTableEval != TranspositionTable::notFound) {
			int tTableDepth = tTable->getDepth(board->state->zobristHash);
			if (tTableDepth > stats.seldepth && tTableDepth != -1) {
				stats.seldepth = tTableDepth;
			}

			if (depth == 0) {
				currentMove = tTable->getBestMove(board->state->zobristHash);
			}

			return tTableEval;
		}

		// If maximum depth is achieved, perform quiescence search
		if (depth >= maxDepth) {
			if (maxDepth > stats.seldepth) {
				stats.seldepth = maxDepth;
			}

			return quiescenceSearch(alpha, beta, maxDepth);
		}

		bool greaterAlpha = false;
		int score = 0;
		int evalBound = TranspositionTable::upperBound;
		int bestDepth = maxDepth;
		// Initialise array for moves (218 is maximum number of moves)
		MovePoint moves[218];
		// Generate moves and store them inside moves[]
		int numMoves = moveGenerator->generateMoves(moves);
		bool isCheck = moveGenerator->isCheck;
		bool worthExtension = false;
		// Get best move (whether it be bestMove from iterative deepening or previous transpositions)
		Move currentBestMove = depth == 0 ? std::move(this->bestMove) : tTable->getBestMove(board->state->zobristHash);
		// Order moves to heuristically narrow search
		orderer->order(board, moves, currentBestMove, numMoves, depth, false);

		for (int i = 0; i < numMoves; ++i) {
			// Make move
			board->makeMove(moves[i].move);
			bool fullSearch = true;
			int extension = 0;
			worthExtension = worthSearching(moves[i].move, isCheck, numExtensions);
			// Reduce depth for moves late in move order as they are unlikely to be good
			/*
			if (i >= 4 * reduceExtensionCutoff && maxDepth - depth >= 3 && !worthExtension) {
				score = -negaMax(-beta, -alpha, depth + 1, maxDepth - 2, numExtensions);
				// If move is good do full search
				fullSearch = score > alpha;
			} else */if (i >= reduceExtensionCutoff && (maxDepth - depth) >= 2 && !worthExtension) {
				score = -negaMax(-beta, -alpha, depth + 1, maxDepth - 1, numExtensions);
				// If move is good do full search
				fullSearch = score > alpha;
			}
			// If move is worth searching more, increase maxdepth for move
			if (worthExtension) {
				extension = 1;
			}
			// If reduced depth move is good, search it fully
			if (fullSearch) {
				score = -negaMax(-beta, -alpha, depth + 1, maxDepth + extension, numExtensions + extension);
			}

			// Undo move
			board->unMakeMove(moves[i].move);

			// If search is cancelled, prevent processing move
			if (cancelSearch)
				return Evaluator::cancelledScore;

			if (score > alpha) {
				alpha = score;
				evalBound = TranspositionTable::exact;
				currentBestMove = moves[i].move;
				greaterAlpha = true;
				bestDepth = maxDepth + extension;
				if (depth == 0 && !cancelSearch) {
					currentMove = moves[i].move;
				}
			}

			if (alpha >= beta) {
				// Store position
				tTable->store(beta, maxDepth + extension - depth, depth, TranspositionTable::lowerBound, moves[i].move, board->state->zobristHash);
				// Update killer moves
				orderer->addKiller(depth, moves[i].move);
				return beta;
			}
		}

		// If no moves, either checkmate or stalemate
		if (numMoves == 0) {
			int eval = Evaluator::drawScore;
			if (moveGenerator->isCheck) {
				eval = -(Evaluator::checkMateScore - depth);
			}
			// Store move
			tTable->store(eval, maxDepth - depth, depth, TranspositionTable::exact, nullMove, board->state->zobristHash);
			return eval;
		}

		// Store move
		tTable->store(alpha, greaterAlpha ? bestDepth - depth : maxDepth - depth, depth, evalBound, greaterAlpha ? currentBestMove : nullMove, board->state->zobristHash);

		return alpha;
	}

	// Movesearch purely searches using recursion. Used for perft command to test move generation
	uint64_t Searcher::moveSearch(int depth, int maxDepth) {
		if (depth == maxDepth) {
			return 1ULL;
		}

		// Generate moves
		uint64_t movesGenerated = 0ULL;

		MovePoint moves[218];

		int numMoves = moveGenerator->generateMoves(moves);
		for (int i = 0; i < numMoves; ++i) {
			uint64_t numMoves{ 0ULL }; // Tracks number of nodes found in perft
			// Simulate move
			board->makeMove(moves[i].move);
			numMoves += moveSearch(depth + 1, maxDepth);
			board->unMakeMove(moves[i].move);

			// If at depth 0, print number of moves from branch, useful for debugging
			if (depth == 0) {
				string promotionpiece = "";
				switch (moves[i].move.getFlag()) {
				case Move::promoteToQueenFlag:
					promotionpiece = "q";
					break;
				case Move::promoteToRookFlag:
					promotionpiece = "r";
					break;
				case Move::promoteToBishopFlag:
					promotionpiece = "b";
					break;
				case Move::promoteToKnightFlag:
					promotionpiece = "n";
					break;
				}
				cout << CoordHelper::indexToString(moves[i].move.getStartSquare()) 
				<< CoordHelper::indexToString(moves[i].move.getTargetSquare()) 
				<< promotionpiece << ": " << numMoves << endl;
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
			this_thread::sleep_for(chrono::milliseconds(searchWaitPeriod));
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
	void Searcher::startSearch(bool isTimed, int moveTimeMs) {
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
	}

	// Cancels search
	void Searcher::endSearch() {
		searchCompleted.store(true);
		cancelSearch.store(true);
		searchStop.notify_all();
	}

	// Determines whether move is worth extending search for
	bool Searcher::worthSearching(Move move, const bool isCheck, const int numExtensions) {
		return (move.getFlag() >= Move::promoteToQueenFlag || isCheck) && numExtensions < maxExtensions;
	}

	// Generates best line
	void Searcher::generateBestLine(Move bestMove) {
		int depth = 0;
		enactBestLine(bestMove, depth); // Begin recursively generating
	}

	// Recursively searches the transposition table for best moves found (principal variation)
	// of current position
	void Searcher::enactBestLine(Move move, int depth) {
		if (move.moveValue == 0) {
			return;
		}

		bestLine->add(move); // Add move to bestline list

		// If threefold repetition, stop searching
		if (board->history.contains(board->state->zobristHash)) {
			return;
		}
		// Apply move to board
		board->makeMove(move);
		// Acquire next move from transposition table
		Move nextMove = tTable->getBestMove(board->state->zobristHash);

		enactBestLine(nextMove, depth + 1);

		board->unMakeMove(move); // Rollback changes to board
	}

	// Checks whether the board position is illegal, used to prevent malicious FEN inputs
	bool Searcher::isPositionIllegal() {
		// If more or less than one king on each side, its an illegal position
		if (board->pieceLists[Board::whiteIndex][Piece::king].numPieces != 1 
			|| board->pieceLists[Board::blackIndex][Piece::king].numPieces != 1) {
			return true;
		}

		// If piece can take a king, its an illegal position
		MovePoint moves[218];

		int numMoves = moveGenerator->generateMoves(moves); // Generate all legal moves

		// If any move attack king, it is illegal
		for (int i = 0; i < numMoves; i++) {
			if (Piece::type(board->squares[moves[i].move.getTargetSquare()]) == Piece::king) {
				return true;
			}
		}

		return false;
	}

	// Returns static evaluation of position
	int Searcher::eval() {
		return evaluator->Evaluate();
	}

	// Deletes old transposition table and creates new one of different size
	void Searcher::changeHashSize(int sizeMB) {
		if (tTable == nullptr) {
			return;
		}

		delete tTable;
		tTable = new TranspositionTable(sizeMB);
	}

	// Clears all transposition table entries
	void Searcher::clearHash() {
		if (tTable == nullptr) {
			return;
		}

		tTable->clear();
	}

	// Performs perft test
	uint64_t Searcher::perft(int depth) {
		return moveSearch(0, depth);
	}

	// Formats evaluation to string, accounting for checkmate scores as well
	std::string Searcher::SearchStatistics::prepareEval() {
		int movesRemaining = Evaluator::movesTilMate(eval);
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

		string pv = searcher->bestLine->str(); // Principal variation
		cout << "info depth " << to_string(depth) << " seldepth " << to_string(seldepth);
		cout << " score " << prepareEval() << " nodes " << to_string(nNodes + qNodes);
		cout << " nps " << to_string(uint64_t(1000000000ULL * (nNodes + qNodes) / duration));
		cout << " hashfull " << to_string((int)(1000 * (float)searcher->tTable->slotsFilled / (float)searcher->tTable->size));
		cout << " time " << to_string(duration / 1000000ULL);

		// If principal variation exists, print it
		if (pv.size() != 0) {
			cout << " pv " << pv;
		}

		cout << endl;
	}

}