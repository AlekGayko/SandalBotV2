#include "Searcher.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <string>

using namespace std;

void Searcher::iterativeSearch() {
	bestMove = Move();
	currentMove = Move();
	searchStatistics temp;
	for (int depth = 1; depth < maxDeepening; depth++) {	
		auto start = chrono::high_resolution_clock::now();
		stats = searchStatistics();
		int eval = negaMax(defaultAlpha, defaultBeta, 0, depth, 0);
		auto end = chrono::high_resolution_clock::now();
		chrono::duration<double> duration = end - start;

		if (!cancelSearch.load()) {
			temp = stats;
			bestMove = currentMove;
			temp.maxDepth = depth;
			temp.eval = eval;
			temp.duration = duration.count();

			cout << "Time: " << duration.count() << ", Moves: " << StringUtil::commaSeparator(stats.nNodes) << ", Moves per second: " << StringUtil::commaSeparator((stats.nNodes) / duration.count()) << endl;
			cout << "Depth " << depth << ", Evaluation: " << eval << ", bestmove: " << currentMove << endl;
		}
		if (cancelSearch.load()) break;
	}
	stats = temp;

	cout << "Transpositions: " << StringUtil::commaSeparator(stats.transpositions) << ", nNodes: " << StringUtil::commaSeparator(stats.nNodes) << ", qNodes: " << StringUtil::commaSeparator(stats.qNodes) << endl;
	//cout << "Checkmates: " << stats.checkmates << ", Stalemates: " << stats.stalemates << ", Repetitions: " << stats.repetitions << ", FiftyMoveDraws: " << stats.fiftyMoveDraws << endl;
	cout << "Time: " << stats.duration << ", Moves per second: " << StringUtil::commaSeparator((stats.nNodes) / stats.duration) << endl;
	cout << "Depth " << stats.maxDepth << ", Evaluation: " << stats.eval << endl;
	cout << endl;
}

int Searcher::QuiescenceSearch(int alpha, int beta, int maxDepth) {
	stats.qNodes++;
	if (cancelSearch) {
		return Evaluator::cancelledScore;
	}

	int tTableEval = tTable->lookup(0, maxDepth, alpha, beta, board->state->zobristHash);
	if (tTableEval != TranspositionTable::notFound) {
		stats.transpositions++;
		return tTableEval;
	}

	int score = 0;

	score = evaluator.Evaluate(board);

	if (score >= beta) {
		stats.cutoffs++;
		return beta;
	}

	if (score > alpha) {
		alpha = score;
	}

	Move moves[218];

	int numMoves = moveGenerator->generateMoves(moves, true);

	if (numMoves > 1) orderer->order(moves, bestMove, numMoves, false, true);

	for (int i = 0; i < numMoves; i++) {
		board->makeMove(moves[i], false);
		score = -QuiescenceSearch(-beta, -alpha, maxDepth);
		board->unMakeMove(moves[i]);
		if (cancelSearch) return Evaluator::cancelledScore;
		if (score >= beta) {
			stats.cutoffs++;
			return beta;
		}
		if (score > alpha) {
			alpha = score;
		}

	}

	return alpha;
}

int Searcher::negaMax(int alpha, int beta, int depth, int maxDepth, int numExtensions) {
	stats.nNodes++;
	if (cancelSearch) {
		return Evaluator::cancelledScore;
	}

	if (depth == maxDepth) {
		return QuiescenceSearch(alpha, beta, maxDepth);
		//return evaluator.Evaluate(board);
	}

	int tTableEval = tTable->lookup(maxDepth - depth, depth, alpha, beta, board->state->zobristHash);

	if (board->history.contains(board->state->zobristHash)) {
		stats.repetitions++;
		if (depth == 0) currentMove = tTable->getBestMove();
		return Evaluator::drawScore;
	} else if (board->state->fiftyMoveCounter >= 50) {
		stats.fiftyMoveDraws++;
		if (depth == 0) currentMove = tTable->getBestMove();
		return Evaluator::drawScore;
	} else if (tTableEval != TranspositionTable::notFound) {
		stats.transpositions++;
		if (depth == 0) currentMove = tTable->getBestMove();
		return tTableEval;
	}

	bool greaterAlpha = false;
	int score = 0;
	int evalBound = TranspositionTable::upperBound;

	Move moves[218];
	int numMoves = moveGenerator->generateMoves(moves);
	int reduceExtensionCutoff = 10;
	int reducedMaxDepth = maxDepth - 1;
	if (reducedMaxDepth - depth <= 0) reducedMaxDepth++;
	//cout << maxDepth << ", " << reducedMaxDepth << endl;
	Move bestMove = depth == 0 ? this->bestMove : tTable->getBestMove();
	if (moveGenerator->isCheck && numExtensions < 16) {
		numExtensions++;
		maxDepth++;
	}
	if (numMoves > 1) orderer->order(moves, bestMove, numMoves, depth == 0, false);
	for (int i = 0; i < numMoves; i++) {
		board->makeMove(moves[i]);

		score = -negaMax(-beta, -alpha, depth + 1, i > reduceExtensionCutoff ? reducedMaxDepth : maxDepth, numExtensions);

		board->unMakeMove(moves[i]);

		if (cancelSearch) return Evaluator::cancelledScore;

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
			tTable->store(beta, maxDepth - depth, depth, TranspositionTable::lowerBound, moves[i], board->state->zobristHash);
			return beta;
		}
	}

	if (!numMoves) {
		int eval = Evaluator::drawScore;
		if (moveGenerator->isCheck) {
			stats.checkmates++;
			eval = -Evaluator::checkMateScore * (maxDepth - depth);
		}
		stats.stalemates++;
		tTable->store(eval, maxDepth - depth, depth, TranspositionTable::exact, Move(), board->state->zobristHash);
		return eval;
	}

	tTable->store(alpha, maxDepth - depth, depth, evalBound, greaterAlpha ? bestMove : Move(), board->state->zobristHash);

	return alpha;
}

uint64_t Searcher::moveSearch(bool isMaximising, int depth, int maxDepth) {
	if (depth == maxDepth) {
		return 1;
	}
	uint64_t movesGenerated = 0;
	Move moves[218];
	int numMoves = moveGenerator->generateMoves(moves);
	for (int i = 0; i < numMoves; i++) {
		board->makeMove(moves[i]);
		movesGenerated += moveSearch(!isMaximising, depth + 1, maxDepth);
		board->unMakeMove(moves[i]);
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

Searcher::Searcher() {

}

Searcher::Searcher(Board* board) {
	this->board = board;
	moveGenerator = new MoveGen(board);
	orderer = new MoveOrderer(board, moveGenerator, this);
	this->tTable = new TranspositionTable(board, 1000);
}

void Searcher::startSearch(int moveTimeMs) {
	cancelSearch.store(false);
	thread workerThread(&Searcher::iterativeSearch, this);
	chrono::milliseconds runDuration(moveTimeMs);
	this_thread::sleep_for(runDuration);
	cancelSearch.store(true);
	workerThread.join();
}

void Searcher::endSearch() {
	cancelSearch = true;
}

Searcher::~Searcher() {
	delete moveGenerator;
	delete orderer;
	delete tTable;
}

uint64_t Searcher::perft(int depth) {
	movesSince = 0;
	return moveSearch(board->state->whiteTurn, 0, depth);
}
