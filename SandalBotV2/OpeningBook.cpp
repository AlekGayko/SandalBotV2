#include "OpeningBook.h"
#include "StringUtil.h"

OpeningBook::OpeningBook() {
}

OpeningBook::OpeningBook(Board* book) : ZobristHash(board) {

}

std::vector<std::string> OpeningBook::obtainMoves(std::string line) {
	return std::vector<std::string>();
}

std::string OpeningBook::chooseMove(std::vector<std::string> moves) {
	return std::string();
}

Move OpeningBook::notationToMove(std::string move) {
	return Move();
}

bool OpeningBook::getMove() {
	return false;
}
