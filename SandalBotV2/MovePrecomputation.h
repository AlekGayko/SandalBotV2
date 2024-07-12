#pragma once

#ifndef MOVEPRECOMPUTATION_H
#define MOVEPRECOMPUTATION_H

class MovePrecomputation {
public:
	struct PrecompCalc {
		// Distances between piece and sides of board
		int top;
		int left;
		int right;
		int bottom;
		int minVertical;
		int minHorizontal;
		PrecompCalc(int top, int left, int right, int bottom);
	};
	
};

#endif // !MOVEPRECOMPUTATION_H
