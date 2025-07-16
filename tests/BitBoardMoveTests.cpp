#include <stdexcept>
#include <vector>

#include <gtest/gtest.h>

#include "Board.h"
#include "Bot.h"
#include "Move.h"
#include "Types.h"

using namespace SandalBot;

Move makeMove(Board* board, std::string start, std::string target, Move::Flag flag) {
	Square from = Square(CoordHelper::stringToIndex(start));
	Square to = Square(CoordHelper::stringToIndex(target));

	Move move = Move(from, to, flag);
	board->makeMove(move);

	return move;
}

void compareBitboards(std::vector<Bitboard>& bitboards1, std::vector<Bitboard>& bitboards2) {
	if (bitboards1.size() != bitboards2.size()) {
		throw std::exception();
	}
	for (int i = bitboards1.size() - 1; i >= 0; i--) {
		std::wstringstream message;
		message << i << "\n";
        EXPECT_EQ(bitboards1[i], bitboards2[i]) << message.str();
	}
}

std::vector<Bitboard> getBitboards(Board* board) {
	std::vector<Bitboard> bitboards;
	for (PieceType type = ALL_PIECES; type < PIECE_TYPE_NB; ++type) {
		bitboards.push_back(board->typesBB[type]);
	}
	bitboards.push_back(board->colorsBB[WHITE]);
	bitboards.push_back(board->colorsBB[BLACK]);
	return bitboards;
}

TEST(BitBoardUpdate, MoveUpdate) {
	Board* board = new Board();

	makeMove(board, "e2", "e4", Move::Flag::PAWN_TWO_SQUARES);

	delete board;
}

TEST(BitBoardUpdate, MoveUndo) {
	Board* board = new Board();
	std::vector<Bitboard> originalBitboards = getBitboards(board);

	Move move = makeMove(board, "e2", "e4", Move::Flag::PAWN_TWO_SQUARES);
	board->unMakeMove();

	std::vector<Bitboard> afterBitboards = getBitboards(board);

	delete board;

	compareBitboards(originalBitboards, afterBitboards);
}

TEST(BitBoardUpdate, MoveUndoPromotion) {
	Board* board = new Board();
	board->loadPosition("8/1k3P2/8/8/8/8/2K5/8 w - - 0 1");
	std::vector<Bitboard> originalBitboards = getBitboards(board);

	Move move = makeMove(board, "f7", "f8", Move::Flag::QUEEN);
	board->unMakeMove();

	std::vector<Bitboard> afterBitboards = getBitboards(board);

	delete board;

	compareBitboards(originalBitboards, afterBitboards);
}

TEST(BitBoardUpdate, MoveUndoTakeOnPromotion) {
	Board* board = new Board();
	board->loadPosition("6r1/1k3P2/8/8/8/8/2K5/8 w - - 0 1");
	std::vector<Bitboard> originalBitboards = getBitboards(board);

	Move move = makeMove(board, "f7", "g8", Move::Flag::QUEEN);
	board->unMakeMove();

	std::vector<Bitboard> afterBitboards = getBitboards(board);

	delete board;

	compareBitboards(originalBitboards, afterBitboards);
}

TEST(BitBoardUpdate, MoveUndoShortCastle) {
	Board* board = new Board();
	board->loadPosition("8/1k6/8/8/8/8/8/R3K2R w KQ - 0 1");
	std::vector<Bitboard> originalBitboards = getBitboards(board);

	Move move = makeMove(board, "e1", "g1", Move::Flag::CASTLE);
	board->unMakeMove();

	std::vector<Bitboard> afterBitboards = getBitboards(board);

	delete board;

	compareBitboards(originalBitboards, afterBitboards);
}

TEST(BitBoardUpdate, MoveUndoLongCastle) {
	Board* board = new Board();
	board->loadPosition("8/1k6/8/8/8/8/8/R3K2R w KQ - 0 1");
	std::vector<Bitboard> originalBitboards = getBitboards(board);

	Move move = makeMove(board, "e1", "c1", Move::Flag::CASTLE);
	board->unMakeMove();

	std::vector<Bitboard> afterBitboards = getBitboards(board);

	delete board;

	compareBitboards(originalBitboards, afterBitboards);
}

TEST(BitBoardUpdate, MoveUndoEnPassant) {
	Board* board = new Board();
	board->loadPosition("8/1k6/8/5Pp1/8/8/8/R3K2R w KQ g6 0 1");
	std::vector<Bitboard> originalBitboards = getBitboards(board);

	Move move = makeMove(board, "f5", "g6", Move::Flag::EN_PASSANT);
	board->unMakeMove();

	std::vector<Bitboard> afterBitboards = getBitboards(board);

	delete board;

	compareBitboards(originalBitboards, afterBitboards);
}