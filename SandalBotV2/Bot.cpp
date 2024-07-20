#include "Bot.h"

#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;

int Bot::validateUserMove(Move moves[218], int startSquare, int targetSquare) {
    int numMoves = searcher->moveGenerator->generateMoves(moves);
    int moveIndex = -1;

    for (int i = 0; i < numMoves; i++) {
        if (moves[i].startSquare == startSquare && moves[i].targetSquare == targetSquare) {
            moveIndex = i;
            break;
        }
    }

    return moveIndex;
}

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

    Move positionMoves[218];
    int moveIndex = validateUserMove(positionMoves, startSquare, targetSquare);

    if (moveIndex == -1) return;

    board->makeMove(positionMoves[moveIndex]);
}

void Bot::generateMove() {
}

void Bot::stopSearching() {
}

unsigned long long int Bot::perft(int depth) {
    searcher->moveGenerator->perftRes.reset();
    searcher->perft(depth);
    //cout << "test moves: " << board->testMoves << endl;
    cout << "Results: " << endl;
    cout << searcher->moveGenerator->perftRes << endl;
    return searcher->perftMoves;
}

std::string Bot::printBoard() {
    return board->printBoard();
}
