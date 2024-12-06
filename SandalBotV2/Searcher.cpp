#include "Searcher.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <string>

using namespace std;

// Perform iterative deepening
void Searcher::iterativeSearch() {
	// Initialise moves and statistics of search
	bestMove = Move();
	currentMove = Move();
	searchStatistics temp;

	// Perform search for each depth until maximum depth
	for (int depth = 1; depth < maxDeepening; depth++) {	
		// Peform negamax search of position and time it
		auto start = chrono::high_resolution_clock::now();
		stats = searchStatistics();
		int eval = negaMax(defaultAlpha, defaultBeta, 0, depth, 0);
		auto end = chrono::high_resolution_clock::now();
		chrono::duration<double> duration = end - start;

		// If search is not cancelled, update stats
		if (!cancelSearch.load()) {
			temp = stats;
			bestMove = currentMove;
			temp.maxDepth = depth;
			temp.eval = eval;
			temp.duration = duration.count();

			cout << "Time: " << duration.count() << ", Moves: " << StringUtil::commaSeparator(stats.nNodes) << ", Moves per second: " << StringUtil::commaSeparator((stats.nNodes) / duration.count()) << endl;
			cout << "Depth " << depth << ", Evaluation: " << eval << ", bestmove: " << currentMove << endl;
		}
		// If search is cancelled, stop iterative deepening
		if (cancelSearch.load()) 
			break;
	}
	// Update stats
	stats = temp;

	// Present search statistics
	cout << "Transpositions: " << StringUtil::commaSeparator(stats.transpositions) << ", nNodes: " << StringUtil::commaSeparator(stats.nNodes) << ", qNodes: " << StringUtil::commaSeparator(stats.qNodes) << endl;
	cout << "Time: " << stats.duration << ", Moves per second: " << StringUtil::commaSeparator((stats.nNodes) / stats.duration) << endl;
	cout << "Depth " << stats.maxDepth << ", Evaluation: " << stats.eval << endl;
	cout << endl;
}

// Quienscence search searches position by only considering moves which take
int Searcher::QuiescenceSearch(int alpha, int beta, int maxDepth) {
	stats.qNodes++;
	if (cancelSearch) {
		return Evaluator::cancelledScore;
	}

	// Check for threefold repetition
	if (board->history.contains(board->state->zobristHash)) {
		stats.repetitions++;
		return Evaluator::drawScore;
	}
	// If position has been previously stored, use its evaluation
	int tTableEval = tTable->lookup(0, maxDepth, alpha, beta, board->state->zobristHash);
	if (tTableEval != TranspositionTable::notFound) {
		stats.transpositions++;
		return tTableEval;
	}

	int score = 0;
	// Evaluate board
	score = evaluator->Evaluate();

	// If evaluation is too good, cut search
	if (score >= beta) {
		stats.cutoffs++;
		return beta;
	}

	if (score > alpha) {
		alpha = score;
	}

	// Initialise moves (218 is maximum number of moves)
	Move moves[218];

	// Generate moves which only take pieces
	int numMoves = moveGenerator->generateMoves(moves, true);

	// Order the moves
	if (numMoves > 1) 
		orderer->order(moves, bestMove, numMoves, 0, false, true);

	for (int i = 0; i < numMoves; i++) {
		// Make move
		board->makeMove(moves[i], false);
		// Recursively search
		score = -QuiescenceSearch(-beta, -alpha, maxDepth);
		// Undo move
		board->unMakeMove(moves[i]);

		// If search cancelled, dont store move and return
		if (cancelSearch) 
			return Evaluator::cancelledScore;

		// If move is too good, return premateruely
		if (score >= beta) {
			stats.cutoffs++;
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

	// If maximum depth is achieved, perform quiescence search
	if (depth >= maxDepth) {
		return QuiescenceSearch(alpha, beta, maxDepth);
	}

	// Lookup position to see if it has been searched and stored in hashtable before
	int tTableEval = tTable->lookup(maxDepth - depth, depth, alpha, beta, board->state->zobristHash);

	// Check for threefold repetition
	// If depth == 0 is included, it will provide move a8a8 since bot uses two fold repetition
	if (board->history.contains(board->state->zobristHash) && depth != 0) {
		stats.repetitions++;
		return Evaluator::drawScore;
	} 
	// Check for fifty move rule
	else if (board->state->fiftyMoveCounter >= 50) {
		stats.fiftyMoveDraws++;
		if (depth == 0) 
			currentMove = tTable->getBestMove();
		return Evaluator::drawScore;
	} 
	// If position found in transposition hash table, use previous evaluation
	else if (tTableEval != TranspositionTable::notFound) {
		stats.transpositions++;
		if (depth == 0) 
			currentMove = tTable->getBestMove();
		return tTableEval;
	}

	bool greaterAlpha = false;
	int score = 0;
	int evalBound = TranspositionTable::upperBound;

	// Initialise array for moves (218 is maximum number of moves)
	Move moves[218];
	// Generate moves and store them inside moves[]
	int numMoves = moveGenerator->generateMoves(moves);
	
	// Get best move (whether it be bestMove from iterative deepening or previous transpositions)
	Move& bestMove = depth == 0 ? this->bestMove : tTable->getBestMove();
	// Order moves to heuristically narrow search
	orderer->order(moves, bestMove, numMoves, depth, depth == 0, false);

	for (int i = 0; i < numMoves; i++) {
		// Make move
		board->makeMove(moves[i]);
		bool fullSearch = true;
		int extension = 0;
		// Reduce depth for moves late in move order as they are unlikely to be good
		if (i == reduceExtensionCutoff && (maxDepth - depth) >= 2) {
			score = -negaMax(-beta, -alpha, depth + 1, maxDepth - 1, numExtensions);
			// If move is good do full search
			fullSearch = score > alpha;
		}
		// If move is worth searching more, increase maxdepth for move
		if (worthSearching(moves[i]) && numExtensions < 16) {
			extension = 1;
		}
		// If reduced depth move is good, search it fully
		if (fullSearch) {
			score = -negaMax(-beta, -alpha, depth + 1, maxDepth - 1 + extension, numExtensions + extension);
		}
		
		// Undo move
		board->unMakeMove(moves[i]);

		// If search is cancelled, prevent processing move
		if (cancelSearch) 
			return Evaluator::cancelledScore;

		if (score > alpha) {
			alpha = score;
			evalBound = TranspositionTable::exact;
			bestMove = moves[i];
			greaterAlpha = true;
			if (depth == 0 && !cancelSearch) {
				currentMove = moves[i];
			}
		}

		if (alpha >= beta) {
			stats.cutoffs++;
			// Store position
			tTable->store(beta, maxDepth - depth, depth, TranspositionTable::lowerBound, moves[i], board->state->zobristHash);
			// Update killer moves
			orderer->addKiller(depth, moves[i]);
			return beta;
		}
	}

	// If no moves, either checkmate or stalemate
	if (numMoves == 0) {
		int eval = Evaluator::drawScore;
		if (moveGenerator->isCheck) {
			stats.checkmates++;
			eval = -Evaluator::checkMateScore * (maxDepth - depth);
		}
		stats.stalemates++;
		// Store move
		tTable->store(eval, maxDepth - depth, depth, TranspositionTable::exact, nullMove, board->state->zobristHash);
		return eval;
	}

	// Store move
	tTable->store(alpha, maxDepth - depth, depth, evalBound, greaterAlpha ? bestMove : nullMove, board->state->zobristHash);

	return alpha;
}

// Movesearch purely searches using recursion. Used for perft command to test move generation
uint64_t Searcher::moveSearch(bool isMaximising, int depth, int maxDepth) {
	if (depth == maxDepth) {
		return 1;
	}
	
	// Generate moves
	uint64_t movesGenerated = 0;
	Move moves[218];
	int numMoves = moveGenerator->generateMoves(moves);
	for (int i = 0; i < numMoves; i++) {
		// Simulate move
		board->makeMove(moves[i]);
		movesGenerated += moveSearch(!isMaximising, depth + 1, maxDepth);
		board->unMakeMove(moves[i]);

		// If at depth 0, print number of moves from branch, incredibly useful for debugging and
		// comparing to other engines
		if (depth == 0) {
			string promotionpiece = "";
			switch (moves[i].flag) {
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
			cout << CoordHelper::indexToString(moves[i].startSquare) << CoordHelper::indexToString(moves[i].targetSquare) << promotionpiece << ": " << movesGenerated - movesSince << endl;
			movesSince = movesGenerated;
		}
		
	}
	return movesGenerated;
}

// Default Constructor
Searcher::Searcher() {

}

// Constructor initialised with board
Searcher::Searcher(Board* board) {
	this->board = board;
	moveGenerator = new MoveGen(board);
	orderer = new MoveOrderer(board, moveGenerator, this);
	evaluator = new Evaluator(board, moveGenerator->preComp);
	this->tTable = new TranspositionTable(board, 1000);
}

// Starts the search on separate thread until time limit up
void Searcher::startSearch(int moveTimeMs) {
	cancelSearch.store(false);
	thread workerThread(&Searcher::iterativeSearch, this);
	chrono::milliseconds runDuration(moveTimeMs);
	this_thread::sleep_for(runDuration);
	cancelSearch.store(true);
	workerThread.join();
}

// Cancels search
void Searcher::endSearch() {
	cancelSearch = true;
}

// Destructor
Searcher::~Searcher() {
	delete moveGenerator;
	delete orderer;
	delete tTable;
	delete evaluator;
}

// Determines whether move is worth extending search for
bool Searcher::worthSearching(const Move& move) {
	return move.flag >= Move::promoteToQueenFlag || moveGenerator->isCheck;
}

// Performs perft test
uint64_t Searcher::perft(int depth) {
	movesSince = 0;
	return moveSearch(board->state->whiteTurn, 0, depth);
}
