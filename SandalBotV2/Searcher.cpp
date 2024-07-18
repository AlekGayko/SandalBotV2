#include "Searcher.h"

#include <iostream>
#include <vector>
#include <string>

using namespace std;

void Searcher::iterativeSearch() {
	for (int i = 0; i < maxDeepening && !cancelSearch; i++) {
		moveSearch(board->state.whiteTurn, 0, i);
	}
}

void Searcher::moveSearch(bool isMaximising, int depth, int maxDepth) {
	if (depth == maxDepth) {
		//cout << board->printBoard() << endl;
		perftMoves++;
		return;
	}
	Move moves[218];
	int numMoves = moveGenerator->generateMoves(moves);
	for (int i = 0; i < numMoves; i++) {
		board->makeMove(moves[i]);
		moveSearch(!isMaximising, depth + 1, maxDepth);
		board->unMakeMove(moves[i]);
		
		if (depth == 0) {
			movesSince0[i] = perftMoves - movesSince;
			movesSince = perftMoves;
		}
		
	}
	
	if (depth == 0) { 
		for (int i = 0; i < numMoves - 4; i+=2) {
			cout << CoordHelper::indexToString(moves[i].startSquare) << CoordHelper::indexToString(moves[i].targetSquare) << ": " << movesSince0[i] << endl;
		}
		for (int i = 1; i < numMoves - 4; i += 2) {
			cout << CoordHelper::indexToString(moves[i].startSquare) << CoordHelper::indexToString(moves[i].targetSquare) << ": " << movesSince0[i] << endl;
		}
		for (int i = numMoves - 4; i < numMoves; i+=2) {
			cout << CoordHelper::indexToString(moves[i + 1].startSquare) << CoordHelper::indexToString(moves[i + 1].targetSquare) << ": " << movesSince0[i+1] << endl;
			cout << CoordHelper::indexToString(moves[i].startSquare) << CoordHelper::indexToString(moves[i].targetSquare) << ": " << movesSince0[i] << endl;
		}
	}
	
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
	moveSearch(board->state.whiteTurn, 0, depth);
	return perftMoves;
}
