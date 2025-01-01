#include "Bot.h"

#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;

int Bot::validateUserMove(Move moves[218], int startSquare, int targetSquare, int flag) {
    int numMoves = searcher->moveGenerator->generateMoves(moves);
    int moveIndex = -1;

    for (int i = 0; i < numMoves; i++) {
        if (moves[i].getStartSquare() == startSquare && moves[i].getTargetSquare() == targetSquare && 
            (moves[i].getFlag() < Move::promoteToQueenFlag || moves[i].getFlag() == flag)) {
            moveIndex = i;
            break;
        }
    }

    return moveIndex;
}

Bot::Bot() {
    board = new Board();
    searcher = new Searcher(board);
    board->setEvaluator(searcher->evaluator);
}

Bot::~Bot() {
    delete board;
    delete searcher;
}

void Bot::setPosition(std::string FEN) {
    board->loadPosition(FEN);
}

void Bot::makeMove(std::string movestr) {
    if (movestr.size() < 3 || movestr.size() > 5) {
        return;
    }
    int startSquare;
    int targetSquare;
    int flag;
    if (movestr == "O-O") {
        startSquare = board->state->whiteTurn ? 60 : 4;
        targetSquare = board->state->whiteTurn ? 62 : 6;
        flag = Move::castleFlag;
    } else if (movestr == "O-O-O") {
        startSquare = board->state->whiteTurn ? 60 : 4;
        targetSquare = board->state->whiteTurn ? 58 : 2;
        flag = Move::castleFlag;
    } else {
        startSquare = CoordHelper::stringToIndex(movestr.substr(0, 2));
        targetSquare = CoordHelper::stringToIndex(movestr.substr(2, 2));
        flag = 0;
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
    }

    Move positionMoves[218];
    int moveIndex = validateUserMove(positionMoves, startSquare, targetSquare, flag);

    if (moveIndex == -1) return;

    board->makeMove(positionMoves[moveIndex]);
}

string Bot::generateMove(int moveTimeMs) {
    searcher->startSearch(true, moveTimeMs);

    if (searcher->bestMove == Move()) {
        return "";
    }

    string startSquare = CoordHelper::indexToString(searcher->bestMove.getStartSquare());
    string targetSquare = CoordHelper::indexToString(searcher->bestMove.getTargetSquare());
    string flag = "";

    switch (searcher->bestMove.getFlag()) {
    case Move::promoteToQueenFlag:
        flag = "q";
        break;
    case Move::promoteToBishopFlag:
        flag = "b";
        break;
    case Move::promoteToKnightFlag:
        flag = "n";
        break;
    case Move::promoteToRookFlag:
        flag = "r";
        break;
    }

    cout << "bestmove " << startSquare << targetSquare << flag << endl;
    return startSquare + targetSquare;
}

void Bot::go() {
    searcher->startSearch(false);
}

int Bot::eval() {
    return searcher->eval();
}

void Bot::stopSearching() {
    searcher->endSearch();
}

uint64_t Bot::perft(int depth) {
    uint64_t movesgenerated = searcher->perft(depth);

    return movesgenerated;
}

void Bot::printBoard() {
    board->printBoard();
}

int Bot::chooseMoveTime(int whiteTimeMs, int blackTimeMs, int whiteIncMs, int blackIncMs) {
    int timeRemaining = board->state->whiteTurn ? whiteTimeMs : blackTimeMs;
    int incMs = board->state->whiteTurn ? whiteIncMs : blackIncMs;
    double minTimeMs = min(50, int(timeRemaining * 0.25));
    double moveTimeMs = float(timeRemaining) / 40.0f;

    if (timeRemaining > incMs * 2 && moveTimeMs > 50) moveTimeMs += timeRemaining * 0.1;

    if (timeRemaining > 45000) return maxMoveTime + 0.5 * incMs;



    int a = max(minTimeMs, moveTimeMs);

    return int(ceil(min(a, maxMoveTime)));
}

void Bot::changeHashSize(int sizeMB) {
    searcher->changeHashSize(sizeMB);
}

void Bot::clearHash() {
    searcher->clearHash();
}
