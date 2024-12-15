#include "Searcher.h"

#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <string>
#include <sstream>

using namespace std;
using namespace StringUtil;

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
		chrono::duration<u64, nano> duration = end - start;

		// If search is not cancelled, update stats
		if (!cancelSearch.load()) {
			bestLine->reset();
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
		} else if (Evaluator::isMateScore(eval)) {
			searchCompleted.store(true);
			searchStop.notify_all();
			break;
		}
	}
	if (!cancelSearch.load() && !searchCompleted.load()) {
		searchCompleted.store(true);
		searchStop.notify_all();
	}
	// Update stats
	stats = temp;
}

// Quienscence search searches position by only considering moves which take
int Searcher::quiescenceSearch(int alpha, int beta, int maxDepth) {
	stats.qNodes++;
	if (cancelSearch) {
		return Evaluator::cancelledScore;
	}

	// Check for threefold repetition
	if (board->history.contains(board->state->zobristHash)) {
		return Evaluator::drawScore;
	}

	// If position has been previously stored, use its evaluation
	int tTableEval = tTable->lookup(0, maxDepth, alpha, beta, board->state->zobristHash);
	if (tTableEval != TranspositionTable::notFound) {
		return tTableEval;
	}

	int score = 0;
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
	Move moves[218];

	// Generate moves which only take pieces
	int numMoves = moveGenerator->generateMoves(moves, true);

	// Order the moves
	if (numMoves > 1) {
		orderer->order(moves, bestMove, numMoves, 0, false, true);
	}

	for (int i = 0; i < numMoves; i++) {
		// Make move
		board->makeMove(moves[i], false);
		// Recursively search
		score = -quiescenceSearch(-beta, -alpha, maxDepth + 1);
		// Undo move
		board->unMakeMove(moves[i]);

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
		/*
		* Not sure if this is necessarily beneficial
		alpha = max(alpha, -Evaluator::checkMateScore + depth);
		beta = min(beta, Evaluator::checkMateScore - depth);

		if (alpha >= beta) {
			return alpha;
		}
		*/
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
	if (board->state->zobristHash == 3868631796917292577ULL) {
		cout << "bestMove: " << tTable->getBestMove(board->state->zobristHash) << endl;
		cout << "prev Hash: " << board->stateHistory.getSecondLast().zobristHash << endl;
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
	Move moves[218];
	// Generate moves and store them inside moves[]
	int numMoves = moveGenerator->generateMoves(moves);
	bool isCheck = moveGenerator->isCheck;
	bool worthExtension = false;
	// Get best move (whether it be bestMove from iterative deepening or previous transpositions)
	Move currentBestMove = depth == 0 ? this->bestMove : tTable->getBestMove(board->state->zobristHash);
	// Order moves to heuristically narrow search
	orderer->order(moves, currentBestMove, numMoves, depth, depth == 0, false);

	for (int i = 0; i < numMoves; i++) {
		// Make move
		board->makeMove(moves[i]);
		bool fullSearch = true;
		int extension = 0;
		worthExtension = worthSearching(moves[i], isCheck, numExtensions);
		// Reduce depth for moves late in move order as they are unlikely to be good
		/*
		if (i >= 4 * reduceExtensionCutoff && maxDepth - depth >= 3 && !worthExtension) {
			score = -negaMax(-beta, -alpha, depth + 1, maxDepth - 2, numExtensions);
			// If move is good do full search
			fullSearch = score > alpha;
		} else */if (i >= reduceExtensionCutoff && maxDepth - depth >= 2 && !worthExtension) {
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
		board->unMakeMove(moves[i]);

		// If search is cancelled, prevent processing move
		if (cancelSearch) 
			return Evaluator::cancelledScore;

		if (score > alpha) {
			alpha = score;
			evalBound = TranspositionTable::exact;
			currentBestMove = moves[i];
			greaterAlpha = true;
			bestDepth = maxDepth + extension;
			if (depth == 0 && !cancelSearch) {
				currentMove = moves[i];
			}
		}

		if (alpha >= beta) {
			// Store position
			tTable->store(beta, maxDepth + extension - depth, depth, TranspositionTable::lowerBound, moves[i], board->state->zobristHash);
			// Update killer moves
			orderer->addKiller(depth, moves[i]);
			return beta;
		}
	}

	// If no moves, either checkmate or stalemate
	if (numMoves == 0) {
		int eval = Evaluator::drawScore;
		if (moveGenerator->isCheck) {
			eval = -Evaluator::checkMateScore + depth;
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
			switch (moves[i].getFlag()) {
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
			cout << CoordHelper::indexToString(moves[i].getStartSquare()) << CoordHelper::indexToString(moves[i].getTargetSquare()) << promotionpiece << ": " << movesGenerated - movesSince << endl;
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
	evaluator->generator = moveGenerator;
	this->tTable = new TranspositionTable(board, 1000);
	cancelSearch.store(false);
	searchCompleted.store(false);

	bestLineSize = maxDeepening + maxExtensions + 1;
	
	bestLine = new MoveLine(bestLineSize);
}

void Searcher::moveSleep(int moveTimeMs) {
	chrono::milliseconds runDurationMs(moveTimeMs);
	auto stopTime = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
	stopTime += runDurationMs.count();

	while (!searchCompleted.load()) {
		this_thread::sleep_for(chrono::milliseconds(searchWaitPeriod));
		auto now = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
		if (now > stopTime) {
			break;
		}
	}

	cancelSearch.store(true);
	searchStop.notify_all();
}

// Starts the search on separate thread until time limit up
void Searcher::startSearch(bool isTimed, int moveTimeMs) {
	cancelSearch.store(false);
	searchCompleted.store(false);
	unique_lock<mutex> lock(searchMutex);

	thread searchThread(&Searcher::iterativeSearch, this);
	thread timerThread;

	if (isTimed) {
		timerThread = thread(&Searcher::moveSleep, this, moveTimeMs);
	}

	searchStop.wait(lock, [this] { return this->cancelSearch.load(); });

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

// Destructor
Searcher::~Searcher() {
	delete moveGenerator;
	delete orderer;
	delete tTable;
	delete evaluator;
	delete bestLine;
}

// Determines whether move is worth extending search for
bool Searcher::worthSearching(Move& move, const bool isCheck, const int numExtensions) {
	return (move.getFlag() >= Move::promoteToQueenFlag || isCheck) && numExtensions < maxExtensions;
}

void Searcher::generateBestLine(Move bestMove) {
	int depth = 0;
	enactBestLine(bestMove, depth);
}

void Searcher::enactBestLine(Move& move, int depth) {
	if (move.moveValue == 0) {
		return;
	}
	//cout << "move: " << move << endl;
	//cout << "hash: " << board->state->zobristHash << endl;
	//cout << "hash % tableSize: " << board->state->zobristHash % 65536000ULL << endl;
	//board->printBoard();
	/*
	if (board->state->zobristHash == 6552879911533713140ULL) {
		cout << "seldepth: " << stats.seldepth << endl;
		cout << "depth: " << depth << endl;
		cout << "move: " << move << endl;
		cout << "movevalue: " << move.moveValue << endl;
		void* rsp = _AddressOfReturnAddress();
		std::cout << "Stack pointer (RSP): " << rsp << std::endl;
		board->printBoard();
	}
	*/

	bestLine->add(move);

	if (board->history.contains(board->state->zobristHash)) {
		return;
	}
	//cout << board->state->zobristHash << endl;
	board->makeMove(move);
	//cout << board->state->zobristHash << endl;
	Move nextMove = tTable->getBestMove(board->state->zobristHash);
	if (board->state->zobristHash == 3868631796917292577ULL) {
		board->printBoard();
		cout << "NextMove: " << nextMove << endl;
	}
	enactBestLine(nextMove, depth + 1);

	board->unMakeMove(move);
}

int Searcher::eval() {
	return quiescenceSearch(defaultAlpha, defaultBeta, 0);
}

// Performs perft test
uint64_t Searcher::perft(int depth) {
	movesSince = 0;
	return moveSearch(board->state->whiteTurn, 0, depth);
}

std::string Searcher::searchStatistics::prepareEval() {
	int movesRemaining = Evaluator::movesTilMate(eval);
	if (movesRemaining != 0)
		return "mate " + to_string(movesRemaining);

	return "cp " + to_string(eval);
}

void Searcher::searchStatistics::print(Searcher* searcher) {
	if (duration == 0)
		duration = 1;
	string pv = searcher->bestLine->str();
	cout << "info depth " << to_string(depth) << " seldepth " << to_string(seldepth);
	cout << " score " << prepareEval() << " nodes " << to_string(nNodes + qNodes);
	cout << " nps " << to_string(u64(1000000000ULL * (nNodes + qNodes) / duration));
	cout << " hashfull " << to_string((int) (10000 * (float) searcher->tTable->slotsFilled / (float) searcher->tTable->size));
	cout << " time " << to_string(duration / 1000000ULL);
	if (pv.size() != 0) {
		cout << " pv " << pv;
	}
	cout << endl;
}