#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

#include <gtest/gtest.h>

#include "TestFileReader.h"
#include "Bot.h"
#include "InitGlobals.h"

using namespace SandalBot;

TEST_F(GlobalInit, InitOnce) {
    SUCCEED();
}

uint64_t performPerftTest(std::string FEN, int depth) {
	Bot bot = Bot();

	bot.setPosition(FEN);

	uint64_t moves = bot.perft(depth);
	return moves;
}

class PerftTest : public ::testing::TestWithParam<std::string> {};

TEST_P(PerftTest, CorrectMoveGeneration) {
    GlobalInit::SetUpTestSuite();
    
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
    
    std::vector<uint64_t> expectedMoveCounts(maxDepth + 1, 0ULL);
    // Populate hashmap
    for (std::string& line : expectedLines) {
        int exDepth;
        uint64_t moveCount;
        std::stringstream ss(line);
        ss >> exDepth;
        ss >> moveCount;
        if (exDepth > maxDepth) {
            FAIL();
        }
        expectedMoveCounts[exDepth] = moveCount;
    }

    // Test for movecount until maxdepth is reached
    for (int depth = 1; depth <= maxDepth; ++depth) {
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