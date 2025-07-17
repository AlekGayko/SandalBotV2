#include <gtest/gtest.h>

#include "TestFileReader.h"

#include "Board.h"
#include "Bot.h"
#include "InitGlobals.h"
#include "Move.h"

#include <iostream>
#include <random>
#include <vector>

using namespace SandalBot;

std::string performPuzzleTest(std::string FEN, std::string correctMove, int moveTime) {
	Bot* bot = new Bot();
	bot->setPosition(FEN);
	std::string bestMove = bot->generateMove(moveTime);

	delete bot;

	return bestMove;
}

class PuzzleTest : public ::testing::TestWithParam<std::string> {};

TEST_P(PuzzleTest, FindBestMove) {
    GlobalInit::SetUpTestSuite();
    std::string fileName = GetParam();
    std::string inputFileName = "puzzles/inputs/" + fileName;
    std::string expectedFileName = "puzzles/expected/" + fileName;

    std::vector<std::string> inputLines = TestFileReader::readFile(inputFileName);
    std::vector<std::string> expectedLines = TestFileReader::readFile(expectedFileName);
    
    if (inputLines.size() != expectedLines.size()) {
        ASSERT_FALSE(false);
    }

    std::stringstream ss;

    for (int i = 0; i < inputLines.size(); ++i) {
        int time;
        std::string FEN, bestMove, category;
        ss.clear();
        ss << inputLines[i];
        ss >> time;
        std::getline(ss >> std::ws, FEN);

        ss.clear();

        ss << expectedLines[i];
        ss >> bestMove;
        ss >> category;

        std::string botMove = performPuzzleTest(FEN, bestMove, time);

        std::string errorMessage = "Expected: " + bestMove + ", Received: " + botMove;
	    errorMessage += ", FEN: " + FEN + ", Description: " + category + ".";

        EXPECT_EQ(bestMove, botMove) << errorMessage;
    }
}

INSTANTIATE_TEST_SUITE_P(PuzzlePerformance, PuzzleTest, ::testing::Values(
    "EasyPuzzles.txt",
    "MediumPuzzles.txt",
    "HardPuzzles.txt",
    "OtherPuzzles.txt"
));