#include "Board.h"
#include "Piece.h"
#include "ZobristHash.h"

#include <iostream>
#include <functional>
#include <unordered_map>


using namespace std;

namespace SandalBot {

	bool ZobristHash::initialised{ false }; // Signifies whether hashes have been initialised

	// Hash values of each piece for both colors for every square
	uint64_t ZobristHash::pieceHashes[2][7][64];
	uint64_t ZobristHash::enPassantHash[64]; // Hash values for each en passant target square
	uint64_t ZobristHash::castlingRightsHash[17]; // Hashes for each castling right
	uint64_t ZobristHash::whiteMoveHash; // Hash for sides turn



	ZobristHash::ZobristHash() : ZobristHash(nullptr) {}

	ZobristHash::ZobristHash(Board* board) : board(board) {
		// Initialises all hash values
		if (!initialised) 
			initHashes();
		initialised = true;
	}

	// Initialises hash member values
	void ZobristHash::initHashes() {
		hash<uint64_t> hash;
		// Iterate over every color, piece and square to produce unique hash values
		for (uint64_t colorIndex = 0; colorIndex < 2; ++colorIndex) {
			for (uint64_t piece = Piece::pawn; piece <= Piece::king; ++piece) {
				for (uint64_t square = 0; square < 64; ++square) {
					pieceHashes[colorIndex][piece][square] = hash(1000000ULL * colorIndex + 10000ULL * piece + square);
				}
			}
		}

		// Init en passant hashes
		for (int square = 0; square < 64; square++) {
			enPassantHash[square] = hash(square);
		}
		// Init castling hashes
		for (int right = 0; right < 17; right++) {
			castlingRightsHash[right] = hash(right);
		}

		whiteMoveHash = hash(true);
	}

	// Returns hash from member board
	uint64_t ZobristHash::hashBoard() {
		return hashBoard(board);
	}

	// Static function returns hash of a given board
	uint64_t ZobristHash::hashBoard(Board* board) {
		// Initialise hashes if not
		if (!initialised) {
			initHashes();
			initialised = true;
		}

		int numPieces;
		uint64_t boardHash = 0ULL;
		// Apply hashes for every piece on board, XOR'ing them together
		for (int colorIndex = 0; colorIndex < 2; colorIndex++) {
			for (int piece = Piece::pawn; piece <= Piece::king; piece++) {
				numPieces = board->pieceLists[colorIndex][piece].numPieces;
				for (int it = 0; it < numPieces; it++) {
					boardHash ^= pieceHashes[colorIndex][piece][board->pieceLists[colorIndex][piece][it]];
				}
			}
		}

		if (board->state->enPassantSquare != -1) 
			boardHash ^= enPassantHash[board->state->enPassantSquare];

		boardHash ^= castlingRightsHash[board->state->castlingRights];

		if (board->state->whiteTurn) boardHash ^= whiteMoveHash;

		return boardHash;
	}

}