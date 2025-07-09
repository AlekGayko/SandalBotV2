#include <stdexcept>
#include <vector>

#include <gtest/gtest.h>

#include "Board.h"
#include "Bot.h"
#include "Move.h"

using namespace SandalBot;

Move makeMove(Board* board, std::string start, std::string target, short int flag) {
	short int startSquare = CoordHelper::stringToIndex(start);
	short int targetSquare = CoordHelper::stringToIndex(target);

	Move move = Move(startSquare, targetSquare, flag);
	board->makeMove(move);

	return move;
}

void compareBitboards(std::vector<uint64_t>& bitboards1, std::vector<uint64_t>& bitboards2) {
	if (bitboards1.size() != bitboards2.size()) {
		throw std::exception();
	}
	for (int i = bitboards1.size() - 1; i >= 0; i--) {
		std::wstringstream message;
		message << i << "\n";
        EXPECT_EQ(bitboards1[i], bitboards2[i]) << message.str();
	}
}

TEST(BitBoardUpdate, MoveUpdate) {
	Board* board = new Board();

	makeMove(board, "e2", "e4", Move::pawnTwoSquaresFlag);

	delete board;
}

TEST(BitBoardUpdate, MoveUndo) {
	Board* board = new Board();
	std::vector<uint64_t> originalBitboards = board->getBitBoards();

	Move move = makeMove(board, "e2", "e4", Move::pawnTwoSquaresFlag);
	board->unMakeMove(move);

	std::vector<uint64_t> afterBitboards = board->getBitBoards();

	delete board;

	compareBitboards(originalBitboards, afterBitboards);
}

TEST(BitBoardUpdate, MoveUndoPromotion) {
	Board* board = new Board();
	board->loadPosition("8/1k3P2/8/8/8/8/2K5/8 w - - 0 1");
	std::vector<uint64_t> originalBitboards = board->getBitBoards();

	Move move = makeMove(board, "f7", "f8", Move::promoteToQueenFlag);
	board->unMakeMove(move);

	std::vector<uint64_t> afterBitboards = board->getBitBoards();

	delete board;

	compareBitboards(originalBitboards, afterBitboards);
}

TEST(BitBoardUpdate, MoveUndoTakeOnPromotion) {
	Board* board = new Board();
	board->loadPosition("6r1/1k3P2/8/8/8/8/2K5/8 w - - 0 1");
	std::vector<uint64_t> originalBitboards = board->getBitBoards();

	Move move = makeMove(board, "f7", "g8", Move::promoteToQueenFlag);
	board->unMakeMove(move);

	std::vector<uint64_t> afterBitboards = board->getBitBoards();

	delete board;

	compareBitboards(originalBitboards, afterBitboards);
}

TEST(BitBoardUpdate, MoveUndoShortCastle) {
	Board* board = new Board();
	board->loadPosition("8/1k6/8/8/8/8/8/R3K2R w KQ - 0 1");
	std::vector<uint64_t> originalBitboards = board->getBitBoards();

	Move move = makeMove(board, "e1", "g1", Move::castleFlag);
	board->unMakeMove(move);

	std::vector<uint64_t> afterBitboards = board->getBitBoards();

	delete board;

	compareBitboards(originalBitboards, afterBitboards);
}

TEST(BitBoardUpdate, MoveUndoLongCastle) {
	Board* board = new Board();
	board->loadPosition("8/1k6/8/8/8/8/8/R3K2R w KQ - 0 1");
	std::vector<uint64_t> originalBitboards = board->getBitBoards();

	Move move = makeMove(board, "e1", "c1", Move::castleFlag);
	board->unMakeMove(move);

	std::vector<uint64_t> afterBitboards = board->getBitBoards();

	delete board;

	compareBitboards(originalBitboards, afterBitboards);
}

TEST(BitBoardUpdate, MoveUndoEnPassant) {
	Board* board = new Board();
	board->loadPosition("8/1k6/8/5Pp1/8/8/8/R3K2R w KQ g6 0 1");
	std::vector<uint64_t> originalBitboards = board->getBitBoards();

	Move move = makeMove(board, "f5", "g6", Move::enPassantCaptureFlag);
	board->unMakeMove(move);

	std::vector<uint64_t> afterBitboards = board->getBitBoards();

	delete board;

	compareBitboards(originalBitboards, afterBitboards);
}