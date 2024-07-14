#include "Bot.h"

#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;

Bot::Bot() {
    board = new Board();
    searcher = new Searcher(board);
}

Bot::~Bot() {
    delete board;
}

void Bot::makeMove(std::string move) {
    if (move.size() != 4) return;
    int startSquare = CoordHelper::stringToIndex(move.substr(0, 2));
    int targetSquare = CoordHelper::stringToIndex(move.substr(2, 2));
    board->makeMove(Move(startSquare, targetSquare));
}

void Bot::generateMove() {
}

void Bot::stopSearching() {
}

int Bot::perft(int depth) {
    return searcher->perft(depth);
}

std::string Bot::printBoard() {
    return board->printBoard();
}
