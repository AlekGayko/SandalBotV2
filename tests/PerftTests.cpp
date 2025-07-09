#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

#include <gtest/gtest.h>

#include "TestFileReader.h"
#include "Bot.h"

using namespace SandalBot;

uint64_t performPerftTest(std::string FEN, int depth) {
	Bot* bot = new Bot();
	bot->setPosition(FEN);
	uint64_t moves = bot->perft(depth);

	delete bot;

	return moves;
}

class PerftTest : public ::testing::TestWithParam<std::string> {};

TEST_P(PerftTest, CorrectMoveGeneration) {
    std::string fileName = GetParam();
    std::string inputFileName = "perft/inputs/" + fileName;
    std::string expectedFileName = "perft/expected/" + fileName;

    std::vector<std::string> inputLines = TestFileReader::readFile(inputFileName);
    std::vector<std::string> expectedLines = TestFileReader::readFile(expectedFileName);
    
    if (inputLines.size() != 1) {
        ASSERT_FALSE(false);
    }

    std::stringstream ss;
    // Extract maximum depth to test and the FEN position
    int maxDepth;
    std::string FEN;
    ss.clear();
    ss << inputLines[0];
    ss >> maxDepth;

    std::getline(ss >> std::ws, FEN);

    std::unordered_map<int, uint64_t> expectedMoveCounts; // Store depth and movecounts in hashmap
    // Populate hashmap
    for (std::string& line : expectedLines) {
        int exDepth;
        uint64_t moveCount;
        ss.clear();
        ss << line;
        ss >> exDepth;
        ss >> moveCount;
        std::cout << "moveCount: " << moveCount << std::endl;
        expectedMoveCounts.insert({ exDepth, moveCount });
    }

    // Test for movecount until maxdepth is reached
    for (int depth = 1; depth <= maxDepth; ++depth) {
        if (expectedMoveCounts.find(depth) == expectedMoveCounts.end()) {
            ASSERT_FALSE(false);
        }

        uint64_t expectedCount = expectedMoveCounts[depth];
        uint64_t moveCount = performPerftTest(FEN, depth);

        EXPECT_EQ(expectedCount, moveCount);
    }
}

INSTANTIATE_TEST_SUITE_P(PerftTests, PerftTest, ::testing::Values(
    "PerftPosition1.txt",
    "PerftPosition2.txt",
    "PerftPosition3.txt",
    "PerftPosition4.txt",
    "PerftPosition5.txt",
    "PerftPosition6.txt",
    "PerftPosition7.txt"
));