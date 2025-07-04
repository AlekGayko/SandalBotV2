#ifndef PIECELIST_H
#define PIECELIST_H

namespace SandalBot {

	// PieceList class provides fast access to piece positions,
	// number of pieces on board
	class PieceList {
	public:
		static constexpr int occupiedSize = 64;
		static constexpr int empty = -1;
		// Stores the squares in which pieces are located
		int occupiedSquares[occupiedSize]; // array of 16 is enough, but 64 for safety
		int map[64]; // Maps squares to position in occupied squares
		int numPieces{ 0 }; // Tracks number of pieces in list

		PieceList();

		void addPiece(int square);
		void deletePiece(int square);
		void movePiece(int startSquare, int targetSquare);

		int& operator[](int index);
	};

}

#endif // !PIECELIST_H
