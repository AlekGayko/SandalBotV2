#include "FEN.h"

#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

namespace SandalBot {
	// Start Position FEN string
	const string FEN::startpos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

	// PositionInfo constructor from given FEN string
	FEN::PositionInfo::PositionInfo(string FEN) {
		this->FEN = FEN;
		int squarePieces[64];
		// Split string via spaces
		vector<string> sections = StringUtil::splitString(FEN);
		// Cannot have FEN string with less than 3 parts
		if (sections.size() < 3) {
			return;
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
		for (int i = 0; i < 64; i++) {
			squares[i] = squarePieces[i];
		}

		// If active color section is 'w', it is whites turn
		whiteTurn = sections[1] == "w";

		// Castling availability section of for 'KQkq', each representing ability 
		// to castle king or queen side for each color
		string castlingRights = sections[2];
		// Determine whether availability is in section
		this->whiteShortCastle = StringUtil::contains(castlingRights, 'K');
		this->whiteLongCastle = StringUtil::contains(castlingRights, 'Q');
		this->blackShortCastle = StringUtil::contains(castlingRights, 'k');
		this->blackLongCastle = StringUtil::contains(castlingRights, 'q');
		// Next variables
		this->enPassantSquare = -1;
		this->fiftyMoveCount = 0;
		this->moveCount = 0;

		// If en passant target square is of length two (e.g. 'e3') it is available
		if (sections.size() > 3 && sections[3].size() == 2) {
			string enPassantSquareName = sections[3];
			enPassantSquare = CoordHelper::stringToIndex(enPassantSquareName);
		}
		// fifty move counter
		if (sections.size() > 4) {
			fiftyMoveCount = stoi(sections[4]);
		}
		// Fullmove number
		if (sections.size() > 5) {
			moveCount = stoi(sections[5]);
		}
	}

	// Given a board, and a parameter to include en passant square, returns a FEN string
	// which accurately respresents the board
	std::string FEN::generateFEN(Board* board, bool includeEPSquare) {
		string FEN = "";
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
		int enPassantFileIndex = board->state->enPassantSquare;
		int enPassantRankIndex = board->state->whiteTurn ? 5 : 2;

		bool isEnPassant = enPassantFileIndex != -1;
		bool includeEnPassant = includeEPSquare;

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

	// Creates PositionInfo to parse FEN and return parsed data
	FEN::PositionInfo FEN::fenToPosition(std::string FEN) {
		PositionInfo position = PositionInfo(FEN);
		return position;
	}

}