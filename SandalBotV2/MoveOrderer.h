#pragma once

#ifndef MOVEORDERER_H
#define MOVEORDERER_H

#include "Board.h"

class MoveOrderer {
private:
	Board* board = nullptr;
	void quickSort(Move moves[]);
public:
	MoveOrderer();
	MoveOrderer(Board* board);
	void order(Move moves[], int numMoves);
};

#endif