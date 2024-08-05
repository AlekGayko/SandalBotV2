#include "Board.h"
#include "Piece.h"
#include "ZobristHash.h"

#include <iostream>
#include <functional>
#include <unordered_map>


using namespace std;

bool ZobristHash::initialised = false;

u64 ZobristHash::pieceHashes[2][7][64];
u64 ZobristHash::enPassantHash[64];
u64 ZobristHash::castlingRightsHash[17];
u64 ZobristHash::whiteMoveHash;



ZobristHash::ZobristHash() {
	if (!initialised) initHashes();
}

ZobristHash::ZobristHash(Board* board) : board(board) {
	if (!initialised) initHashes();
	initialised = true;
}


void ZobristHash::initHashes() {
	hash<u64> hash;
	for (int colorIndex = 0; colorIndex < 2; colorIndex++) {
		for (int piece = Piece::pawn; piece <= Piece::king; piece++) {
			for (int square = 0; square < 64; square++) {
				pieceHashes[colorIndex][piece][square] = hash(1000000 * colorIndex + 10000 * piece + square);
			}
		}
	}

	for (int square = 0; square < 64; square++) {
		enPassantHash[square] = hash(square);
	}

	for (int right = 0; right < 17; right++) {
		castlingRightsHash[right] = hash(right);
	}

	whiteMoveHash = hash(true);
}

u64 ZobristHash::hashBoard() {
	int numPieces;
	u64 boardHash = 0ULL;
	for (int colorIndex = 0; colorIndex < 2; colorIndex++) {
		for (int piece = Piece::pawn; piece <= Piece::king; piece++) {
			numPieces = board->pieceLists[colorIndex][piece].numPieces;
			for (int it = 0; it < numPieces; it++) {
				boardHash ^= pieceHashes[colorIndex][piece][board->pieceLists[colorIndex][piece][it]];
			}
		}
	}

	if (board->state->enPassantSquare != -1) boardHash ^= enPassantHash[board->state->enPassantSquare];

	boardHash ^= castlingRightsHash[board->state->castlingRights];

	if (board->state->whiteTurn) boardHash ^= whiteMoveHash;

	return boardHash;
}

u64 ZobristHash::hashBoard(Board* board) {
	if (!initialised) {
		initHashes();
		initialised = true;
	}
	int numPieces;
	u64 boardHash = 0ULL;
	for (int colorIndex = 0; colorIndex < 2; colorIndex++) {
		for (int piece = Piece::pawn; piece <= Piece::king; piece++) {
			numPieces = board->pieceLists[colorIndex][piece].numPieces;
			for (int it = 0; it < numPieces; it++) {
				boardHash ^= pieceHashes[colorIndex][piece][board->pieceLists[colorIndex][piece][it]];
			}
		}
	}

	if (board->state->enPassantSquare != -1) boardHash ^= enPassantHash[board->state->enPassantSquare];

	boardHash ^= castlingRightsHash[board->state->castlingRights];

	if (board->state->whiteTurn) boardHash ^= whiteMoveHash;

	return boardHash;
}