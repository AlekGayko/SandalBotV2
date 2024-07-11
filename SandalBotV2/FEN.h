#pragma once

#ifndef FEN_H
#define FEN_H

#include "Piece.h"
#include "Board.h"

#include <string>

// Utility class for FEN strings
class FEN {
	struct PositionInfo {
		std::string FEN;
		int squares[64];
		bool whiteShortCastle;
		bool whiteLongCastle;
		bool blackShortCastle;
		bool blackLongCastle;
		int enPassantFile;
		bool whiteTurn;
		int fiftyMoveCount;
		int moveCount;
		PositionInfo(std::string FEN);
	};
public:
	static const std::string startpos;
	std::string generateFEN(Board board, bool includeEPSquare = true);
	bool enPassantCapturable(Board board, int epFileIndex, int epRankIndex);
	std::string flipFEN(std::string FEN);
};

#endif