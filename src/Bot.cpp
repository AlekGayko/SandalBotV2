#include "Bot.h"

#include "Types.h"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;

namespace SandalBot {

    // Validates a move on the current board position
    int Bot::validateUserMove(MovePoint moves[218], Square from, Square to, Move::Flag flag) {
        // Generate all possible moves in position
        int numMoves = searcher->moveGenerator->generate(moves);
        int moveIndex = -1;
        // If proposed move is in possible moves, it is a valid move
        for (int i = 0; i < numMoves; i++) {
            if (moves[i].move.from() == from && moves[i].move.to() == to &&
                (moves[i].move.flag() > Move::Flag::QUEEN || moves[i].move.flag() == flag)) {
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
        delete searcher;
    }

    // Set new board position
    void Bot::setPosition(std::string_view FEN) {
        board->loadPosition(FEN);
    }

    // Make a move from provided string. String must be in pure algebraic coordinate notation (per UCI)
    void Bot::makeMove(std::string movestr) {
        // UCI notation does not permit moves with length outside [3, 5]
        if (movestr.size() < 3 || movestr.size() > 5) {
            return;
        }
        // These variables define all moves
        Square from;
        Square to;
        Move::Flag flag;

        // Castle notation are edge cases
        if (movestr == "O-O") {
            from = Square(board->sideToMove() == WHITE ? W_KING_SQUARE : B_KING_SQUARE);
            to = board->sideToMove() == WHITE ? G1 : G8;
            flag = Move::Flag::CASTLE;
        } else if (movestr == "O-O-O") {
            from = Square(board->sideToMove() == WHITE ? W_KING_SQUARE : B_KING_SQUARE);
            to = board->sideToMove() == WHITE ? C1 : C8;
            flag = Move::Flag::CASTLE;
        } else {
            // Extract start and target square
            from = Square(CoordHelper::stringToIndex(movestr.substr(0, 2)));
            to = Square(CoordHelper::stringToIndex(movestr.substr(2, 2)));
            flag = Move::Flag::NO_FLAG;

            // Promotion moves have an extra character for promotion piece
            if (movestr.size() == 5) {
                switch (movestr[4]) {
                case 'q':
                    flag = Move::Flag::QUEEN;
                    break;
                case 'r':
                    flag = Move::Flag::ROOK;
                    break;
                case 'n':
                    flag = Move::Flag::KNIGHT;
                    break;
                case 'b':
                    flag = Move::Flag::BISHOP;
                    break;
                }
            }
        }

        // Validate move
        MovePoint positionMoves[218];
        int moveIndex = validateUserMove(positionMoves, from, to, flag);

        if (moveIndex == -1) 
            return;

        // If move is valid, enact move
        board->makeMove(positionMoves[moveIndex].move);
    }

    // Generate move within allotted time in milliseconds
    string Bot::generateMove(int moveTimeMs) {
        searcher->startSearch(true, moveTimeMs); // Generate move

        // If move is essentially null, either error, illegal position, or could not find move in time frame
        if (searcher->bestMove == Move()) {
            return "";
        }

        // Generate UCI notation from generated move
        string from = CoordHelper::indexToString(searcher->bestMove.from());
        string to = CoordHelper::indexToString(searcher->bestMove.to());
        string flag = "";

        switch (searcher->bestMove.flag()) {
        case Move::Flag::QUEEN:
            flag = "q";
            break;
        case Move::Flag::BISHOP:
            flag = "b";
            break;
        case Move::Flag::KNIGHT:
            flag = "n";
            break;
        case Move::Flag::ROOK:
            flag = "r";
            break;
        }

        cout << "bestmove " << from << to << flag << endl;
        return from + to;
    }

    // Search position asynchronously
    void Bot::go() {
        searcher->startSearch(false);
    }

    // Generate static evaluation from position
    int Bot::eval() {
        return searcher->eval();
    }

    // End asynchronous search
    void Bot::stopSearching() {
        searcher->endSearch();
    }

    // Perft stands for 'performance test', it returns the number of possible positions
    // at a given depth. Useful command to debug move generation issues.
    uint64_t Bot::perft(int depth) {
        if (depth <= 0) {
            return 1;
        }

        uint64_t movesgenerated = searcher->perft(depth);

        return movesgenerated;
    }

    // Print the board position, FEN string, and zobrist hash
    void Bot::printBoard() {
        board->printBoard();
    }

    // Returns the amount of thinking time the bot wishes to spend while generating a move
    int Bot::chooseMoveTime(int whiteTimeMs, int blackTimeMs, int whiteIncMs, int blackIncMs) {
        int timeRemaining = board->sideToMove() == WHITE ? whiteTimeMs : blackTimeMs;
        int incMs = board->sideToMove() == WHITE ? whiteIncMs : blackIncMs;
        double minTimeMs = min(50, int(timeRemaining * 0.25));
        double moveTimeMs = float(timeRemaining) / 40.0f;

        if (timeRemaining > incMs * 2 && moveTimeMs > 50) moveTimeMs += timeRemaining * 0.1;

        if (timeRemaining > 45000) return maxMoveTime + static_cast<int>(0.5 * static_cast<double>(incMs));



        int a = static_cast<int>(max(minTimeMs, moveTimeMs));

        return int(ceil(min(a, maxMoveTime)));
    }

    // Change the size of the transposition table
    void Bot::changeHashSize(int sizeMB) {
        searcher->changeHashSize(sizeMB);
    }

    // Clear the transposition table
    void Bot::clearHash() {
        searcher->clearHash();
    }

}