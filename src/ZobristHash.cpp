#include "Board.h"
#include "ZobristHash.h"

#include <cassert>
#include <functional>
#include <iostream>
#include <random>


using namespace std;

namespace SandalBot {

	HashKey ZobristHash::pieceHashes[2][7][64];
	HashKey ZobristHash::enPassantHash[64];
	HashKey ZobristHash::castlingRightsHash[17];
	HashKey ZobristHash::whiteMoveHash;

	// Initialises hash member values
	void ZobristHash::initHashes() {
		std::mt19937_64 rng(0x1234);

		// Iterate over every color, piece and square to produce unique hash values
		for (PieceType type = PAWN; type <= KING; ++type) {
			for (Square sq = START_SQUARE; sq < SQUARES_NB; ++sq) {
				pieceHashes[BLACK][type][sq] = rng();
				pieceHashes[WHITE][type][sq] = rng();
			}
		}

		// Init en passant hashes
		for (Square sq = START_SQUARE; sq < SQUARES_NB; ++sq) {
			enPassantHash[sq] = rng();
		}
		// Init castling hashes
		for (CastlingRights cr = NO_RIGHTS; cr < RIGHTS_NB; ++cr) {
			castlingRightsHash[int(cr)] = rng();
		}

		whiteMoveHash = rng();
	}

	// Static function returns hash of a given board
	HashKey ZobristHash::hashBoard(Board* board) {
		assert(board != nullptr);

		HashKey boardHash = 0ULL;
		// Apply hashes for every piece on board, XOR'ing them together
		for (Square sq = START_SQUARE; sq < SQUARES_NB; ++sq) {
			PieceType type = typeOf(board->squares[sq]);
			Color color = colorOf(board->squares[sq]);

			if (type == NO_PIECE_TYPE) {
				continue;
			}

			boardHash ^= pieceHashes[color][type][sq];
		}

		if (board->state->enPassantSquare != NONE_SQUARE) 
			boardHash ^= enPassantHash[board->state->enPassantSquare];

		boardHash ^= castlingRightsHash[int(board->state->cr)];

		if (board->sideToMove() == WHITE) 
			boardHash ^= whiteMoveHash;

		return boardHash;
	}

}