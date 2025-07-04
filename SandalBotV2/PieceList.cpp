#include "PieceList.h"

#include <algorithm>
#include <cassert>
#include <stdexcept>

namespace SandalBot {

	// Fill arrays with sentinel value empty
	PieceList::PieceList() {
		std::fill(occupiedSquares, occupiedSquares + occupiedSize, empty);
		std::fill(map, map + 64, empty);
	}

	// Add piece to the list from a given square
	void PieceList::addPiece(int square) {
		assert(square >= 0 && square < 64);
		assert(numPieces < 64);

		occupiedSquares[numPieces] = square; // Add to list of squares of pieces
		map[square] = numPieces; // Map square to index
		++numPieces;
	}

	// Remove piece from list from a given square
	void PieceList::deletePiece(int square) {
		assert(square >= 0 && square < 64);
		assert(numPieces > 0);

		const int occupiedIndex = map[square]; // Get index mapped from square
		const int lastOccupiedIndex = numPieces - 1;
		const int lastSquare = occupiedSquares[lastOccupiedIndex];
		// Replace to be deleted piece with piece at end of list
		occupiedSquares[occupiedIndex] = occupiedSquares[lastOccupiedIndex];
		// Update map
		map[lastSquare] = occupiedIndex;
		map[square] = -1;

		--numPieces;
	}

	// Move piece from one square to another
	void PieceList::movePiece(int startSquare, int targetSquare) {
		const int occupiedStartIndex = map[startSquare];

		occupiedSquares[occupiedStartIndex] = targetSquare;
		map[targetSquare] = occupiedStartIndex;
		map[startSquare] = -1;
	}

	// Access occupiedSquares via indexing to get squares of pieces
	int& PieceList::operator[](int index) {
		assert(index >= 0 && index < numPieces);

		return occupiedSquares[index];
	}

}