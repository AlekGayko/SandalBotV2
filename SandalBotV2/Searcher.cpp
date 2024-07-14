#include "Searcher.h"

void Searcher::iterativeSearch() {
	for (int i = 0; i < maxDeepening && !cancelSearch; i++) {
		negaMax(board->state.whiteTurn, 0, i);
	}
}

int Searcher::negaMax(bool isMaximising, int depth, int maxDepth) {
	if (depth == maxDepth) {
		perftMoves++;
		return 0;
	}
	int bestScore = -1000000;
	int score = 0;
	Move moves[218];
	int numMoves = moveGenerator->generateMoves(moves, isMaximising);
	for (int i = 0; i < numMoves; i++) {
		board->makeMove(moves[i]);
		score = -negaMax(!isMaximising, depth, maxDepth);
		if (score > bestScore) bestScore = score;
		board->unMakeMove(moves[i]);
	}
	return 0;
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
}

void Searcher::startSearch() {
	iterativeSearch();
}

void Searcher::endSearch() {
	cancelSearch = true;
}

int Searcher::perft(int depth) {
	perftMoves = 0;
	negaMax(board->state.whiteTurn, 0, depth);
	return perftMoves;
}
