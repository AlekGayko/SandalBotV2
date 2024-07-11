#pragma once

#ifndef PIECELIST_H
#define PIECELIST_H

class PieceList {
public:
	int* occupiedSquares = nullptr;
	int map[64];
	int numPieces = 0;

	PieceList(int maxNumPieces = 16);
	~PieceList();

	void addPiece(int square);
	void deletePiece(int square);
	void movePiece(int startSquare, int targetSquare);

	int& operator[](int index);
};

#endif // !PIECELIST_H
