#pragma once

#ifndef BOARD_H
#define BOARD_H

#include "BoardHistory.h"
#include "BoardState.h"
#include "CoordHelper.h"
#include "Move.h"
#include "PieceList.h"

#include <stack>
#include <vector>

class FEN;

class Board {
private:
	
public:
	
	int squares[64];

	PieceList pieceLists[2][7]; // index [i][0] is redundant since 0 = empty
	//int kings[2];

	BoardHistory history;
	std::vector<BoardState> boardStateHistory;
	BoardState* state = nullptr;

	// Bitboards
	uint64_t allPieces;
	uint64_t whitePieces;
	uint64_t blackPieces;
	uint64_t orthogonalPieces;
	uint64_t diagonalPieces;
	uint64_t pawns;
	uint64_t knights;

	static const int blackIndex = 0;
	static const int whiteIndex = 1;

	int testMoves = 0;

	Board();
	~Board();
	void loadPieceLists();
	void loadBitBoards();
	void loadPosition(std::string fen);
	void makeMove(Move& move, bool hashBoard = true);
	void makeEnPassantChanges(Move& move);
	void makeCastlingChanges(Move& move, int& castlingRights);
	void makePromotionChanges(Move& move, int& piece);
	void unMakeMove(Move& move);
	void undoEnPassantChanges(Move& move);
	void undoCastlingChanges(Move& move);
	void updateBitBoards(Move& move, int pieceType, int takenPiece);
	void undoBitBoards(Move& move, int pieceType, int takenPiece);
	void printBoard();
	std::vector<uint64_t> getBitBoards();
};

#endif // !BOARD_H
