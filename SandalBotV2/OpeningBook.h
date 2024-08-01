#pragma once

#ifndef OPENINGBOOK_H
#define OPENINGBOOK_H

#include "Move.h"
#include "ZobristHash.h"

#include <string>
#include <vector>

class OpeningBook : public ZobristHash {
private:
	const std::string path = "";
	std::vector<std::string> obtainMoves(std::string line);
	std::string chooseMove(std::vector<std::string> moves);
	Move notationToMove(std::string move);
public:
	Move openingMove;

	OpeningBook();
	OpeningBook(Board* book);

	bool getMove();

};

#endif // !OPENINGBOOK_H
