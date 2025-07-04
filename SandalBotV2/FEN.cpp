#include "FEN.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace SandalBot {
	// Given a board, and a parameter to include en passant square, returns a FEN string
	// which accurately respresents the board
	std::string FEN::generateFEN(Board* board, bool includeEPSquare) {
		std::string FEN = "";
		// Iterate over board
		for (int rank = 0; rank < 8; rank++) {
			int numEmptyFiles = 0; // Keep track of empty squares to include
			for (int file = 0; file < 8; file++) {
				int index = rank * 8 + file;
				int piece = board->squares[index];
				// If non-empty piece, append non-zero empty squares, and then append piece symbol
				if (piece != Piece::empty) {
					if (numEmptyFiles != 0) {
						FEN += to_string(numEmptyFiles);
						numEmptyFiles = 0;
					}
					char pieceChar = Piece::pieceToSymbol(piece);
					FEN += pieceChar;
				} else {
					numEmptyFiles++;
				}
			}
			// If remaining empty squares, append to FEN string
			if (numEmptyFiles != 0) {
				FEN += to_string(numEmptyFiles);
			}
			// If not end of board, signify rank/row change with '/'
			if (rank != 7) {
				FEN += '/';
			}
		}
		// Append active color
		FEN += ' ';
		FEN += board->state->whiteTurn ? 'w' : 'b';

		bool whiteShortCastle = (board->state->whiteShortCastleMask & board->state->castlingRights);
		bool whiteLongCastle = (board->state->whiteLongCastleMask & board->state->castlingRights);
		bool blackShortCastle = (board->state->blackShortCastleMask & board->state->castlingRights);
		bool blackLongCastle = (board->state->blackLongCastleMask & board->state->castlingRights);

		// Append castling availability
		FEN += ' ';
		FEN += whiteShortCastle ? "K" : "";
		FEN += whiteLongCastle ? "Q" : "";
		FEN += blackShortCastle ? "k" : "";
		FEN += blackLongCastle ? "q" : "";
		FEN += (board->state->castlingRights == 0) ? "-" : ""; // No castling rights, default to '-'

		// Append en passant target square
		FEN += ' ';

		if (includeEPSquare) {
			FEN += CoordHelper::indexToString(board->state->enPassantSquare);
		} else {
			FEN += '-';
		}

		// Append fifty move counter
		FEN += ' ';
		FEN += to_string(board->state->fiftyMoveCounter);
		// Append full move counter
		FEN += ' ';
		FEN += to_string((board->state->moveCounter / 2) + 1);

		return FEN;
	}

	PositionInfo FEN::fenToPosition(std::string_view FEN) {
		PositionInfo newInfo;
		int squarePieces[64] {};
		// Split string via spaces
		vector<std::string> sections = StringUtil::splitString(FEN);
		// Cannot have FEN string with less than 3 parts
		if (sections.size() < 3) {
			throw runtime_error("FEN Parsing Error");
		}

		int file = 0;
		int rank = 0;
		// Parse piece placement data
		for (char symbol : sections[0]) {
			// '/' represents new row/rank
			if (symbol == '/') {
				file = 0;
				rank++;
			} else {
				// If character is a digit, represents x empty squares on a row
				if (isdigit(symbol)) {
					int square = rank * 8 + file;
					file += symbol - '0';
					for (int i = 0; i < std::stoi(std::string(1, symbol)); i++) {
						squarePieces[square + i] = Piece::empty;
					}
				} 
				// Symbol must represent a piece
				else {
					int piece = Piece::symbolToPiece(symbol);
					squarePieces[rank * 8 + file] = piece;
					file++;
				}
			}
		}
		// Copy calculated square pieces to squares member variables
		std::copy(squarePieces, squarePieces + 64, newInfo.squares);

		// If active color section is 'w', it is whites turn
		newInfo.whiteTurn = sections[1] == "w";

		// Castling availability section of for 'KQkq', each representing ability 
		// to castle king or queen side for each color
		std::string castlingRights = sections[2];
		// Determine whether availability is in section
		newInfo.whiteShortCastle = StringUtil::contains(castlingRights, 'K');
		newInfo.whiteLongCastle = StringUtil::contains(castlingRights, 'Q');
		newInfo.blackShortCastle = StringUtil::contains(castlingRights, 'k');
		newInfo.blackLongCastle = StringUtil::contains(castlingRights, 'q');
		// Next variables
		newInfo.enPassantSquare = -1;
		newInfo.fiftyMoveCount = 0;
		newInfo.moveCount = 0;

		// If en passant target square is of length two (e.g. 'e3') it is available
		if (sections.size() > 3 && sections[3].size() == 2) {
			string enPassantSquareName = sections[3];
			newInfo.enPassantSquare = CoordHelper::stringToIndex(enPassantSquareName);
		}
		// fifty move counter
		if (sections.size() > 4) {
			newInfo.fiftyMoveCount = stoi(sections[4]);
		}
		// Fullmove number
		if (sections.size() > 5) {
			newInfo.moveCount = stoi(sections[5]);
		}

		return newInfo;
	}
}