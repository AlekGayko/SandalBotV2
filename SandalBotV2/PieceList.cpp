#include "PieceList.h"

#include <stdexcept>

PieceList::PieceList(int maxNumPieces) {
	occupiedSquares = new int[maxNumPieces];
	for (int i = 0; i < maxNumPieces; i++) {
		occupiedSquares[i] = -1;
	}
	for (int i = 0; i < 64; i++) {
		map[i] = -1;
	}
}

PieceList::~PieceList() {
	if (occupiedSquares != nullptr) {
		delete[] occupiedSquares;
	}
}

void PieceList::addPiece(int square) {
	occupiedSquares[numPieces] = square;
	map[square] = numPieces;
	numPieces++;
}

void PieceList::deletePiece(int square) {
	const int occupiedIndex = map[square];
	const int lastOccupiedIndex = numPieces - 1;
	const int lastSquare = occupiedSquares[lastOccupiedIndex];

	occupiedSquares[map[square]] = occupiedSquares[lastOccupiedIndex];
	map[lastSquare] = occupiedIndex;
	map[square] = -1;

	numPieces--;
}

void PieceList::movePiece(int startSquare, int targetSquare) {
	const int occupiedStartIndex = map[startSquare];

	occupiedSquares[occupiedStartIndex] = targetSquare;
	map[targetSquare] = occupiedStartIndex;
	map[startSquare] = -1;
}


int& PieceList::operator[](int index) {
	if (index < 0 || index >= numPieces) {
		throw std::out_of_range("Index out of range.");
	}
	return occupiedSquares[index];
}
