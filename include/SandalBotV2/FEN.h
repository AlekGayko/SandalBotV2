#ifndef FEN_H
#define FEN_H

#include "Board.h"
#include "StringUtil.h"
#include "Types.h"

#include <iostream>
#include <string>
#include <string_view>

namespace SandalBot {

	using namespace std::literals::string_view_literals;

	// PositionInfo stores information that can be extracted from FEN string
	struct PositionInfo {
		std::string FEN{};
		Piece squares[64];
		int fiftyMoveCount{};
		int moveCount{};
		Square enPassantSquare{};
		CastlingRights cr{};
		Color sideToMove{};
		
		PositionInfo() = default;
	};

	// Utility class for FEN strings, can parse FEN strings and generate them
	namespace FEN {
		// Start Position FEN string
		constexpr std::string_view startpos { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"sv };

		std::string generateFEN(const Board* board, bool includeEPSquare = true);

		// Creates PositionInfo to parse FEN and return parsed data
		PositionInfo fenToPosition(std::string_view FEN);
	};

}

#endif