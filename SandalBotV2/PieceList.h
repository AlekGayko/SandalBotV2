#pragma once

#ifndef PIECELIST_H
#define PIECELIST_H

class PieceList {
public:
	int occupiedSquares[64]; // array of 16 is enough, but 64 for potential
	int map[64];
	int numPieces = 0;

	PieceList(int maxNumPieces = 64);
	~PieceList();

	void addPiece(int square);
	void deletePiece(int square);
	void movePiece(int startSquare, int targetSquare);

	int& operator[](int index);
};

#endif // !PIECELIST_H
