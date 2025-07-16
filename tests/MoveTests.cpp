#include <gtest/gtest.h>

#include "Move.h"
#include "Types.h"

using namespace SandalBot;

TEST(Move, SimpleMove) {
	Square startSquare = A8;
	Square targetSquare = H1;

	uint16_t expectedValue = 0b0000000000111111;

	EXPECT_EQ(expectedValue, Move(startSquare, targetSquare).moveValue);
}

TEST(Move, GetStartSquare) {
	Square from = G2;
	Square to = E1;

	EXPECT_EQ(from, Move(from, to).from());
}

TEST(Move, Getto) {
	Square from = G2;
	Square to = A6;

	Move move = Move(from, to);

	EXPECT_EQ(to, move.to());
}

TEST(Move, GetFlag) {
	Square from = A4;
	Square to = F3;
	Move::Flag flag = Move::Flag::QUEEN;

	Move move = Move(from, to, flag);

	EXPECT_EQ(flag, move.flag());
}

TEST(Move, GetSquaresAndFlag) {
	Square from = A4;
	Square to = F3;
	Move::Flag flag = Move::Flag::QUEEN;

	Move move = Move(from, to, flag);

	EXPECT_EQ(from, move.from());
	EXPECT_EQ(to, move.to());
	EXPECT_EQ(flag, move.flag());
}