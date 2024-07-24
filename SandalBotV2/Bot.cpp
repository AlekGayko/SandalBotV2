#include "Bot.h"

#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;

int Bot::validateUserMove(Move moves[218], int startSquare, int targetSquare, int flag) {
    int numMoves = searcher->moveGenerator->generateMoves(moves);
    int moveIndex = -1;

    for (int i = 0; i < numMoves; i++) {
        if (moves[i].startSquare == startSquare && moves[i].targetSquare == targetSquare && (moves[i].flag < Move::promoteToQueenFlag || moves[i].flag == flag)) {
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
    if (movestr.size() != 4 && movestr.size() != 5) return;

    int startSquare = CoordHelper::stringToIndex(movestr.substr(0, 2));
    int targetSquare = CoordHelper::stringToIndex(movestr.substr(2, 2));
    int flag = 0;
    if (movestr.size() == 5) {
        switch (movestr[4]) {
        case 'q':
            flag = Move::promoteToQueenFlag;
            break;
        case 'r':
            flag = Move::promoteToRookFlag;
            break;
        case 'n':
            flag = Move::promoteToKnightFlag;
            break;
        case 'b':
            flag = Move::promoteToBishopFlag;
            break;
        }
    }

    Move positionMoves[218];
    int moveIndex = validateUserMove(positionMoves, startSquare, targetSquare, flag);

    if (moveIndex == -1) return;

    board->makeMove(positionMoves[moveIndex]);
}

void Bot::generateMove() {
}

void Bot::stopSearching() {
}

unsigned long long int Bot::perft(int depth) {
    searcher->moveGenerator->perftRes.reset();
    unsigned long long int movesgenerated = searcher->perft(depth);
    //cout << "test moves: " << board->testMoves << endl;
    cout << "Results: " << endl;
    cout << searcher->moveGenerator->perftRes << endl;
    return movesgenerated;
}

std::string Bot::printBoard() {
    return board->printBoard();
}
