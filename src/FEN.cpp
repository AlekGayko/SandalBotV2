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

	template <Color Us, bool Short>
	static bool verifyCastle(bool right, Piece squares[]) {
		if (!right) {
			return false;
		}
		constexpr Piece usRook = makePiece(ROOK, Us);
		constexpr Piece usKing = makePiece(KING, Us);
		constexpr Square kSq = Square(Us == WHITE ? W_KING_SQUARE : B_KING_SQUARE);
		constexpr Square rSq = Square(Short ? W_ROOK_SHORT_SQ : W_ROOK_LONG_SQ);
		constexpr Square usRSq = Us == WHITE ? rSq : flipRow(rSq);
		
		if (squares[kSq] != usKing) {
			return false;
		}
		
		if (squares[usRSq] != usRook) {
			return false;
		}
		
		return true;
	}

	// Given a board, and a parameter to include en passant square, returns a FEN string
	// which accurately respresents the board
	std::string FEN::generateFEN(const Board* board, bool includeEPSquare) {
		std::string FEN = "";
		// Iterate over board
		for (Row rank = ROW_8; rank <= ROW_1; ++rank) {
			int numEmptyFiles = 0; // Keep track of empty squares to include
			for (Column file = COL_A; file <= COL_H; ++file) {
				Square index = Square(rank * 8 + file);
				Piece piece = board->squares[index];
				// If non-empty piece, append non-zero empty squares, and then append piece symbol
				if (piece != NO_PIECE) {
					if (numEmptyFiles != 0) {
						FEN += to_string(numEmptyFiles);
						numEmptyFiles = 0;
					}
					char pieceChar = pieceToSymbol(piece);
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
		FEN += board->sideToMove() == WHITE ? 'w' : 'b';

		bool whiteShortCastle = bool(W_OO & board->state->cr);
		bool whiteLongCastle = bool(W_OOO & board->state->cr);
		bool blackShortCastle = bool(B_OO & board->state->cr);
		bool blackLongCastle = bool(B_OOO & board->state->cr);

		// Append castling availability
		FEN += ' ';
		FEN += whiteShortCastle ? "K" : "";
		FEN += whiteLongCastle ? "Q" : "";
		FEN += blackShortCastle ? "k" : "";
		FEN += blackLongCastle ? "q" : "";
		FEN += (board->state->cr == NO_RIGHTS) ? "-" : ""; // No castling rights, default to '-'

		// Append en passant target square
		FEN += ' ';

		if (includeEPSquare && board->state->enPassantSquare != NONE_SQUARE) {
			FEN += CoordHelper::indexToString(board->state->enPassantSquare);
		} else {
			FEN += '-';
		}

		// Append fifty move counter
		FEN += ' ';
		FEN += to_string(board->state->fiftyMoveCounter);
		// Append full move counter
		FEN += ' ';
		FEN += to_string((board->moveCounter() / 2) + 1);

		return FEN;
	}

	PositionInfo FEN::fenToPosition(std::string_view FEN) {
		PositionInfo newInfo{};
		Piece squarePieces[SQUARES_NB] {};
		std::fill(squarePieces, squarePieces + SQUARES_NB, NO_PIECE);
		// Split string via spaces
		vector<std::string> sections = StringUtil::splitString(FEN);

		// Cannot have FEN string with less than 3 parts
		if (sections.size() < 3) {
			throw runtime_error("FEN Parsing Error");
		}

		Column file = COL_START;
		Row rank = ROW_START;
		// Parse piece placement data
		for (char symbol : sections[0]) {
			if (rank >= ROW_NB || rank < ROW_START) {
				throw runtime_error("Invalid rank number");
			}
			// '/' represents new row/rank
			if (symbol == '/') {
				if (file < COL_NB) {
					throw runtime_error("Illegal '/' rank change");
				}
				file = COL_START;
				++rank;
			} else {
				if (file >= COL_NB || file < COL_START) {
					std::cout << file << std::endl;
					throw runtime_error("Invalid file number");
				}
				// If character is a digit, represents x empty squares on a row
				if (isdigit(symbol)) {
					Square square = Square(rank * 8 + file);
					file = Column(file + symbol - '0');
				} 
				// Symbol must represent a piece
				else {
					Piece piece = symbolToPiece(symbol);
					squarePieces[rank * 8 + file] = piece;
					++file;
				}
			}
		}
		// Copy calculated square pieces to squares member variables
		std::copy(squarePieces, squarePieces + SQUARES_NB, newInfo.squares);

		// If active color section is 'w', it is whites turn
		newInfo.sideToMove = sections[1] == "w" ? WHITE : BLACK;

		// Castling availability section of for 'KQkq', each representing ability 
		// to castle king or queen side for each color
		std::string castlingRights = sections[2];
		// Determine whether availability is in section
		bool whiteShortCastle = StringUtil::contains(castlingRights, 'K');
		bool whiteLongCastle = StringUtil::contains(castlingRights, 'Q');
		bool blackShortCastle = StringUtil::contains(castlingRights, 'k');
		bool blackLongCastle = StringUtil::contains(castlingRights, 'q');

		whiteShortCastle = verifyCastle<WHITE, true>(whiteShortCastle, newInfo.squares);
		whiteLongCastle = verifyCastle<WHITE, false>(whiteLongCastle, newInfo.squares);
		blackShortCastle = verifyCastle<BLACK, true>(blackShortCastle, newInfo.squares);
		blackLongCastle = verifyCastle<BLACK, false>(blackLongCastle, newInfo.squares);

		newInfo.cr = NO_RIGHTS;
		newInfo.cr |= whiteShortCastle ? W_OO : newInfo.cr;
		newInfo.cr |= whiteLongCastle ? W_OOO : newInfo.cr;
		newInfo.cr |= blackShortCastle ? B_OO : newInfo.cr;
		newInfo.cr |= blackLongCastle ? B_OOO : newInfo.cr;

		// Next variables
		newInfo.enPassantSquare = NONE_SQUARE;
		newInfo.fiftyMoveCount = 0;
		newInfo.moveCount = 0;

		// If en passant target square is of length two (e.g. 'e3') it is available
		if (sections.size() > 3 && sections[3].size() == 2) {
			std::string enPassantSquareName = sections[3];
			newInfo.enPassantSquare = Square(CoordHelper::stringToIndex(enPassantSquareName));
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