#include "PieceList.h"

#include <stdexcept>

PieceList::PieceList(int numPieces = 16) {
	occupiedSquares = new int[numPieces];
	for (int i = 0; i < numPieces; i++) {
		occupiedSquares[i] = -1;
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
	const int occupiedIndex = map[startSquare];

	occupiedSquares[occupiedIndex] = targetSquare;
	map[targetSquare] = occupiedIndex;
	map[startSquare] = -1;
	
}

int PieceList::count() const {
	return numPieces;
}

int& PieceList::operator[](int index) {
	if (index < 0 || index >= numPieces) {
		throw std::out_of_range("Index out of range.");
	}
	return occupiedSquares[index];
}
