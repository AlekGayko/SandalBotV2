#include "Searcher.h"

void Searcher::iterativeSearch() {
	for (int i = 0; i < maxDeepening && !cancelSearch; i++) {
		negaMax(0, i);
	}
}

int Searcher::negaMax(int depth, int maxDepth) {
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
}

void Searcher::startSearch() {
	iterativeSearch();
}

void Searcher::endSearch() {
	cancelSearch = true;
}
