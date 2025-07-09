#include <gtest/gtest.h>

#include "Move.h"

using namespace SandalBot;

TEST(Move, SimpleMove) {
	int startSquare = 0;
	int targetSquare = 63;

	uint16_t expectedValue = 0b0000000000111111;

	EXPECT_EQ(expectedValue, Move(startSquare, targetSquare).moveValue);
}

TEST(Move, GetStartSquare) {
	unsigned short int startSquare = 54;
	unsigned short int targetSquare = 60;

	EXPECT_EQ(startSquare, Move(startSquare, targetSquare).getStartSquare());
}

TEST(Move, GetTargetSquare) {
	unsigned short int startSquare = 54;
	unsigned short int targetSquare = 16;

	Move move = Move(startSquare, targetSquare);

	EXPECT_EQ(targetSquare, move.getTargetSquare());
}

TEST(Move, GetFlag) {
	unsigned short int startSquare = 32;
	unsigned short int targetSquare = 45;
	unsigned short int flag = Move::promoteToQueenFlag;

	Move move = Move(startSquare, targetSquare, flag);

	EXPECT_EQ(flag, move.getFlag());
}

TEST(Move, GetSquaresAndFlag) {
	unsigned short int startSquare = 32;
	unsigned short int targetSquare = 45;
	unsigned short int flag = Move::promoteToQueenFlag;

	Move move = Move(startSquare, targetSquare, flag);

	EXPECT_EQ(startSquare, move.getStartSquare());
	EXPECT_EQ(targetSquare, move.getTargetSquare());
	EXPECT_EQ(flag, move.getFlag());
}