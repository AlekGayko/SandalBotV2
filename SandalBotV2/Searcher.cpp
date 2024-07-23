#include "Searcher.h"

#include <iostream>
#include <vector>
#include <string>

using namespace std;

void Searcher::iterativeSearch() {
	for (int i = 0; i < maxDeepening && !cancelSearch; i++) {
		moveSearch(board->state->whiteTurn, 0, i);
	}
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
			movesSince0[i] = movesGenerated - movesSince;
			movesSince = movesGenerated;
		}
		
	}
	if (depth == 0) { 
		for (int i = 0; i < numMoves; i++) {
			cout << CoordHelper::indexToString(moves[i].startSquare) << CoordHelper::indexToString(moves[i].targetSquare) << ": " << movesSince0[i] << endl;
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
}

void Searcher::startSearch() {
	iterativeSearch();
}

void Searcher::endSearch() {
	cancelSearch = true;
}

unsigned long long int Searcher::perft(int depth) {
	movesSince = 0;
	return moveSearch(board->state->whiteTurn, 0, depth);
}
