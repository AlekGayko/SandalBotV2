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
	for (int i = 1; !cancelSearch.load(); i++) {
		moves = 0;
		auto start = chrono::high_resolution_clock::now();
		int eval = negaMax(defaultAlpha, defaultBeta, 0, i);
		auto end = chrono::high_resolution_clock::now();
		chrono::duration<double> duration = end - start;
		cout << "time: " << duration.count() << ", moves: " << moves << ", moves per second: " << moves / duration.count() << endl;
		moves = 0;
		if (!cancelSearch) bestMove = currentMove;
		cout << "Depth " << i << " completed. Evaluation: " << eval << ",  Bestmove: " << bestMove << endl;
		cout << endl;
	}
}

int Searcher::negaMax(int alpha, int beta, int depth, int maxDepth) {
	moves++;
	if (cancelSearch) {
		return 0;
	}

	if (depth == maxDepth) {
		return -evaluator.Evaluate(board);
	}
	int score = 0;
	Move moves[218];
	int numMoves = moveGenerator->generateMoves(moves);
	orderer->order(moves, numMoves, depth == 0);

	for (int i = 0; i < numMoves; i++) {
		board->makeMove(moves[i]);
		score = -negaMax(-beta, -alpha, depth + 1, maxDepth);
		board->unMakeMove(moves[i]);
		if (score > alpha) {
			alpha = score;
			if (depth == 0 && !cancelSearch) {
				currentMove = moves[i];
			}
		}
		if (alpha >= beta) return alpha;
		if (cancelSearch) return 0;

	}

	if (!numMoves) return moveGenerator->isCheck ? Evaluator::checkMateScore * (maxDepth - depth) : 0;

	return alpha;
}

unsigned long long int Searcher::moveSearch(bool isMaximising, int depth, int maxDepth) {
	if (depth == maxDepth) {
		//cout << board->printBoard() << endl;
		return 1;
	}
	unsigned long long int movesGenerated = 0;
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

int Searcher::QuiescenceSearch() {
	return 0;
}

Searcher::Searcher()
{

}

Searcher::Searcher(Board* board) {
	this->board = board;
	moveGenerator = new MoveGen(board);
	orderer = new MoveOrderer(board, moveGenerator, this);
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
}

unsigned long long int Searcher::perft(int depth) {
	movesSince = 0;
	return moveSearch(board->state->whiteTurn, 0, depth);
}
