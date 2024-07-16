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

void Bot::setPosition(std::string FEN) {
    board->loadPosition(FEN);
}

void Bot::makeMove(std::string movestr) {
    if (movestr.size() != 4) return;
    int startSquare = CoordHelper::stringToIndex(movestr.substr(0, 2));
    int targetSquare = CoordHelper::stringToIndex(movestr.substr(2, 2));
    Move move = Move(startSquare, targetSquare);
    board->makeMove(move);
}

void Bot::generateMove() {
}

void Bot::stopSearching() {
}

int Bot::perft(int depth) {
    searcher->perft(depth);
    //cout << "test moves: " << board->testMoves << endl;
    return searcher->perftMoves;
}

std::string Bot::printBoard() {
    return board->printBoard();
}
