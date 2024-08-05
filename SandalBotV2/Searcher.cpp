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
	for (int i = 1; !cancelSearch.load(); i++) {	
		auto start = chrono::high_resolution_clock::now();
		stats = searchStatistics();
		int eval = negaMax(defaultAlpha, defaultBeta, 0, i);
		auto end = chrono::high_resolution_clock::now();
		chrono::duration<double> duration = end - start;
		if (!cancelSearch) {
			temp = stats;
			bestMove = currentMove;
			temp.maxDepth = i;
			temp.eval = eval;
			temp.duration = duration.count();
		}
		cout << "Time: " << stats.duration << ", Moves: " << stats.nNodes << ", Moves per second: " << (stats.nNodes) / stats.duration << endl;
		cout << "Depth " << i << ", Evaluation: " << eval << endl;
	}
	stats = temp;
	/**/
	cout << "Transpositions: " << stats.transpositions << ", Cutoffs: " << stats.cutoffs << ", nNodes: " << stats.nNodes << ", qNodes: " << stats.qNodes << endl;
	cout << "Checkmates: " << stats.checkmates << ", Stalemates: " << stats.stalemates << ", Repetitions: " << stats.repetitions << ", FiftyMoveDraws: " << stats.fiftyMoveDraws << endl;
	cout << "Time: " << stats.duration << ", Moves: " << stats.nNodes << ", Moves per second: " << (stats.nNodes) / stats.duration << endl;
	cout << "Depth " << stats.maxDepth << ", Evaluation: " << stats.eval << endl;
	cout << endl;
}

int Searcher::QuiescenceSearch(int alpha, int beta) {
	stats.qNodes++;
	if (cancelSearch) {
		return Evaluator::drawScore;
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

	if (numMoves > 1) orderer->order(moves, numMoves, false);

	for (int i = 0; i < numMoves; i++) {
		board->makeMove(moves[i], false);
		score = -QuiescenceSearch(-beta, -alpha);
		board->unMakeMove(moves[i]);
		if (score >= beta) {
			stats.cutoffs++;
			return beta;
		}
		if (score > alpha) {
			alpha = score;
		}
		if (cancelSearch) return Evaluator::drawScore;

	}

	return alpha;
}

int Searcher::negaMax(int alpha, int beta, int depth, int maxDepth) {
	stats.nNodes++;
	if (cancelSearch) {
		return Evaluator::drawScore;
	}

	if (depth == maxDepth) {
		return QuiescenceSearch(alpha, beta);
		//return evaluator.Evaluate(board);
	}
	int score = 0;
	int evalBound = TranspositionTable::upperBound;
	Move moves[218];
	int numMoves = moveGenerator->generateMoves(moves);
	Move bestMove = depth == 0 ? this->bestMove : tTable->getBestMove();
	if (numMoves > 1) orderer->order(moves, bestMove, numMoves, depth == 0);
	for (int i = 0; i < numMoves; i++) {
		board->makeMove(moves[i]);
		u64 hash = board->state->zobristHash;
		int storedEval = tTable->lookup(maxDepth - depth, alpha, beta, hash);
		if (storedEval != TranspositionTable::notFound) {
			score = -storedEval;
			stats.transpositions++;
		} else {
			if (board->history[hash]) {
				score = 0;
				stats.repetitions++;
			} else if (board->state->fiftyMoveCounter >= 50) {
				score = 0;
				stats.fiftyMoveDraws++;
			} else {
				score = -negaMax(-beta, -alpha, depth + 1, maxDepth);
			}
		}
		board->unMakeMove(moves[i]);
		//if (storedEval == TranspositionTable::notFound) tTable->store(score, maxDepth - depth, hash);
		if (score >= beta) {
			stats.cutoffs++;
			tTable->store(score, maxDepth - depth, TranspositionTable::lowerBound, moves[i], hash);
			return beta;
		}
		if (score > alpha) {
			alpha = score;
			evalBound = TranspositionTable::exact;
			bestMove = moves[i];
			if (depth == 0 && !cancelSearch) {
				currentMove = moves[i];
			}
		}
		if (cancelSearch) return Evaluator::drawScore;
	}

	if (!numMoves) {
		if (moveGenerator->isCheck) {
			stats.checkmates++;
			return Evaluator::checkMateScore * (maxDepth - depth);
		}
		stats.stalemates++;
		return Evaluator::drawScore;
	}

	tTable->store(alpha, maxDepth - depth, evalBound, bestMove, board->state->zobristHash);

	return alpha;
}

uint64_t Searcher::moveSearch(bool isMaximising, int depth, int maxDepth) {
	if (depth == maxDepth) {
		//cout << board->printBoard() << endl;
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

Searcher::Searcher()
{

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
