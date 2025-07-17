#include <algorithm>
#include <random>

#include <gtest/gtest.h>

#include "InitGlobals.h"
#include "Move.h"
#include "MoveOrderer.h"

using namespace SandalBot;

// Tests the sorting of moves
constexpr int movesSize = 218;

void checkSortedMoves(MovePoint moves[], int numMoves) {
	if (numMoves <= 1) {
		return;
	}

	for (int i{ 1 }; i < numMoves; ++i) {
		if (moves[i - 1].value < moves[i].value) {
            ASSERT_FALSE(false);
		}
	}

    ASSERT_TRUE(true);
}

void generateMoves(MovePoint moves[], int numMoves) {
    static std::mt19937 gen(0x1234);
	static std::uniform_int_distribution<int> dist(-500, 500);

	for (int i = 0; i < numMoves; ++i) {
		moves[i].value = dist(gen);
	}
}

TEST(MoveSorting, QuickSort) {
	GlobalInit::SetUpTestSuite();
	MovePoint moves[movesSize];
	int moveSizes[]{ 5, 10, 20, 40, 60 };

	for (int size : moveSizes) {
		generateMoves(moves, size);

		MoveOrderer::quickSort(moves, 0, size);
		std::sort(moves, moves + size, [](MovePoint& mp1, MovePoint& mp2) { return mp1.value > mp2.value; });

		checkSortedMoves(moves, size);
	}
}

TEST(MoveSorting, BubbleSort) {
	GlobalInit::SetUpTestSuite();
	MovePoint moves[movesSize];
	int moveSizes[]{ 5, 10, 20, 40, 60 };

	for (int size : moveSizes) {
		generateMoves(moves, size);

		MoveOrderer::bubbleSort(moves, size);

		checkSortedMoves(moves, size);
	}
}

TEST(MoveSorting, InsertionSort) {
	GlobalInit::SetUpTestSuite();
	MovePoint moves[movesSize];
	int moveSizes[]{ 5, 10, 20, 40, 60 };

	for (int size : moveSizes) {
		generateMoves(moves, size);

		MoveOrderer::insertionSort(moves, size);

		checkSortedMoves(moves, size);
	}
}

TEST(MoveSorting, SelectionSort) {
	GlobalInit::SetUpTestSuite();
	MovePoint moves[movesSize];
	int moveSizes[]{ 5, 10, 20, 40, 60 };

	for (int size : moveSizes) {
		generateMoves(moves, size);

		MoveOrderer::selectionSort(moves, size);

		checkSortedMoves(moves, size);
	}
}

TEST(MoveSorting, MergeSort) {
	GlobalInit::SetUpTestSuite();
	MovePoint moves[movesSize];
	int moveSizes[]{ 5, 10, 20, 40, 60 };

	for (int size : moveSizes) {
		generateMoves(moves, size);

		MoveOrderer::mergeSort(moves, 0, size);

		checkSortedMoves(moves, size);
	}
}

TEST(MoveSorting, StdSort) {
	GlobalInit::SetUpTestSuite();
	MovePoint moves[movesSize];
	int moveSizes[] { 5, 10, 20, 40, 60 };

	for (int size : moveSizes) {
		generateMoves(moves, size);

		std::sort(moves, moves + size, [](MovePoint& mp1, MovePoint& mp2) { return mp1.value > mp2.value; });

		checkSortedMoves(moves, size);
	}
}