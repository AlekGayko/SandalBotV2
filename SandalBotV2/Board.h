#pragma once
#ifndef BOARD_H
#define BOARD_H

#include "Move.h"

class Board {
private:

public:
	Board();
	void MakeMove(Move move);
	void UnMakeMove(Move move);
};

#endif // !BOARD_H
