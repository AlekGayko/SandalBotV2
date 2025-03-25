#ifndef FEN_H
#define FEN_H

#include "Board.h"
#include "Piece.h"
#include "StringUtil.h"

#include <string>

namespace SandalBot {

	class Board;

	// Utility class for FEN strings, can parse FEN strings and generate them
	class FEN {
	public:
		// PositionInfo stores information that can be extracted from FEN string
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
		static PositionInfo fenToPosition(std::string FEN);
	};

}

#endif