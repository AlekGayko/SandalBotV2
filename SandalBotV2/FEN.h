#ifndef FEN_H
#define FEN_H

#include "Board.h"
#include "Piece.h"
#include "StringUtil.h"

#include <string>

namespace SandalBot {

	class Board;

	// Utility class for FEN strings
	class FEN {
	public:
		struct PositionInfo {
			std::string FEN{};
			int squares[64] = { 0 };
			bool whiteShortCastle{};
			bool whiteLongCastle{};
			bool blackShortCastle{};
			bool blackLongCastle{};
			int enPassantSquare{};
			bool whiteTurn{};
			int fiftyMoveCount{};
			int moveCount{};
			inline PositionInfo(std::string FEN);
		};
		static const std::string startpos;
		static std::string generateFEN(Board* board, bool includeEPSquare = true);
		static bool enPassantCapturable(Board* board, int epFileIndex, int epRankIndex);
		static std::string flipFEN(std::string FEN);
		static PositionInfo fenToPosition(std::string FEN);
	};

}

#endif