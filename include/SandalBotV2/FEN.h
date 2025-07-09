#ifndef FEN_H
#define FEN_H

#include "Board.h"
#include "Piece.h"
#include "StringUtil.h"

#include <iostream>
#include <string>
#include <string_view>

namespace SandalBot {

	class Board;

	// PositionInfo stores information that can be extracted from FEN string
	struct PositionInfo {
		std::string FEN{};
		int8_t squares[64];
		uint8_t fiftyMoveCount{};
		uint16_t moveCount{};
		uint8_t enPassantSquare{};
		bool whiteShortCastle{};
		bool whiteLongCastle{};
		bool blackShortCastle{};
		bool blackLongCastle{};
		bool whiteTurn{};
		
		PositionInfo() = default;
	};

	// Utility class for FEN strings, can parse FEN strings and generate them
	namespace FEN {
		// Start Position FEN string
		constexpr std::string_view startpos { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" };

		std::string generateFEN(Board* board, bool includeEPSquare = true);

		// Creates PositionInfo to parse FEN and return parsed data
		PositionInfo fenToPosition(std::string_view FEN);
	};

}

#endif