#include "FEN.h"

#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

const string FEN::startpos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

FEN::PositionInfo::PositionInfo(string FEN) {
	this->FEN = FEN;
	int squarePieces[64];
	vector<string> sections = StringUtil::splitString(FEN);

	if (sections.size() < 3) {
		return;
	}

	int file = 0;
	int rank = 0;

	for (char symbol : sections[0]) {
		if (symbol == '/') {
			file = 0;
			rank++;
		} else {
			if (isdigit(symbol)) {
				int square = rank * 8 + file;
				file += symbol - '0';
				for (int i = 0; i < std::stoi(std::string(1, symbol)); i++) {
					squarePieces[square + i] = Piece::empty;
				}
			} else {
				int piece = Piece::symbolToPiece(symbol);
				squarePieces[rank * 8 + file] = piece;
				file++;
			}
		}
	}

	for (int i = 0; i < 64; i++) {
		squares[i] = squarePieces[i];
	}

	whiteTurn = sections[1] == "w";

	string castlingRights = sections[2];
	whiteShortCastle = StringUtil::contains(castlingRights, 'K');
	whiteLongCastle = StringUtil::contains(castlingRights, 'Q');
	blackShortCastle = StringUtil::contains(castlingRights, 'k');
	blackLongCastle = StringUtil::contains(castlingRights, 'q');

	enPassantFile = 0;
	fiftyMoveCount = 0;
	moveCount = 0;

	if (sections.size() > 3 && sections[3].size() == 1) {
		string enPassantFileName = sections[3];
		// get boardhelper to convert char to string of enpassantfile
	}

	if (sections.size() > 4) {
		fiftyMoveCount = stoi(sections[4]);
	}

	if (sections.size() > 5) {
		moveCount = stoi(sections[5]);
	}
}

std::string FEN::generateFEN(Board* board, bool includeEPSquare) {
	string FEN = "";
	for (int rank = 0; rank < 8; rank++) {
		int numEmptyFiles = 0;
		for (int file = 0; file < 8; file++) {
			int index = rank * 8 + file;
			int piece = board->squares[index];
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
		if (numEmptyFiles != 0) {
			FEN += to_string(numEmptyFiles);
		}
		if (rank != 0) {
			FEN += '/';
		}
	}

	FEN += ' ';
	FEN += board->state.whiteTurn ? 'w' : 'b';

	bool whiteShortCastle = (board->state.whiteShortCastleMask & board->state.castlingRights);
	bool whiteLongCastle = (board->state.whiteLongCastleMask & board->state.castlingRights);
	bool blackShortCastle = (board->state.blackShortCastleMask & board->state.castlingRights);
	bool blackLongCastle = (board->state.blackLongCastleMask & board->state.castlingRights);

	FEN += ' ';
	FEN += whiteShortCastle ? "K" : "";
	FEN += whiteLongCastle ? "Q" : "";
	FEN += blackShortCastle ? "k" : "";
	FEN += blackLongCastle ? "q" : "";
	FEN += (board->state.castlingRights == 0) ? "-" : "";

	FEN += ' ';
	int enPassantFileIndex = board->state.enPassantFile;
	int enPassantRankIndex = board->state.whiteTurn ? 5 : 2;

	bool isEnPassant = enPassantFileIndex != -1;
	bool includeEnPassant = includeEPSquare || enPassantCapturable(board, enPassantFileIndex, enPassantRankIndex);

	if (isEnPassant && includeEnPassant) {
		// need boardhelper (squarename from coordinate)
		
	} else {
		FEN += '-';
	}

	FEN += ' ';
	FEN += to_string(board->state.fiftyMoveCounter);

	FEN += ' ';
	FEN += to_string((board->state.moveCounter / 2) + 1);

	return FEN;
}

bool FEN::enPassantCapturable(Board* board, int epFileIndex, int epRankIndex) {
	return false;
}

std::string FEN::flipFEN(std::string FEN) {
	return std::string();
}

FEN::PositionInfo FEN::fenToPosition(std::string FEN) {
	PositionInfo position = PositionInfo(FEN);
	return position;
}