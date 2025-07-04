#include "IUCI.h"

#include <cstring>
#include <cctype>
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;
using namespace chrono;

namespace SandalBot {
	// Opening message presented to user
	void IUCI::beginningMessage() {
		cout << name << " by " << author << "." << endl;
	}
	// Clears log file
	void IUCI::emptyLogs() {
		ofstream logs(logPath.data(), ios::trunc);

		if (!logs.is_open()) {
			cout << "hi" << endl;
			cerr << "Couldn't empty logs" << endl;
		}
		logs.close();
	}
	
	// Init members
	IUCI::IUCI() {
		beginningMessage();
		emptyLogs();
		bot = new Bot();
		optionHandler = new OptionHandler(bot);
	}

	IUCI::~IUCI() {
		delete bot;
		delete optionHandler;
	}
	// Processes command string and parses it.
	void IUCI::processCommand(string command) {
		try {
			logInfo("Command received: " + command);
			command = StringUtil::trim(command); // Strip leading and end spaces
			// Acquire first word
			const string commandType = StringUtil::toLower(StringUtil::splitString(command)[0]);
			// Parse different commands
			if (commandType == "uci") {
				UCIok();
			} else if (commandType == "isready") {
				respond("readyok");
			} else if (commandType == "ucinewgame") {
				newGame();
			} else if (commandType == "position") {
				processPositionCommand(command);
			} else if (commandType == "go") {
				processGoCommand(command);
			} else if (commandType == "eval") {
				eval();
			} else if (commandType == "stop") {
				stop();
			} else if (commandType == "quit") {
				quit();
			} else if (commandType == "setoption") {
				processSetOption(command);
			} else if (commandType == "d") {
				bot->printBoard();
			} else {
				logInfo("Unknown Command: " + commandType);
			}
		} catch (exception& e) {
			logInfo(e.what());
		}
	}
	// 'ucinewgame' command, deletes bot and instantiates new one
	void IUCI::newGame() {
		stop();
		delete bot;
		bot = new Bot();
	}
	// Stops any current searching of the bot
	void IUCI::stop() {
		// If thread is running
		if (goThread.joinable()) {
			bot->stopSearching();
			this_thread::sleep_for(chrono::milliseconds(100));
			goThread.join();
			this_thread::sleep_for(chrono::milliseconds(100));
		}
	}
	// Quit command stops go thread and exits program
	void IUCI::quit() {
		// Stop go thread
		if (goThread.joinable()) {
			bot->stopSearching();
			goThread.join();
		}
		// Exit
		exit(0);
	}
	// Processes 'uci' command. Responds to user with available options
	void IUCI::UCIok() {
		respond(std::string("id name ") + name);
		respond(std::string("id author ") + author);
		cout << endl;
		respond(optionHandler->getOptionsString());
		cout << endl;
		respond("uciok");
	}
	// Provides user with static evaluation of position
	void IUCI::eval() {
		int evaluation = bot->eval();

		respond("evaluation " + to_string((float)evaluation / 100.f));
	}
	// Outputs best move
	void IUCI::OnMoveChosen(string move) {
		respond("bestmove " + move);
	}
	// Process go command into either, constant movetime search, perft test, infinite go search,
	// or real time clock time search
	void IUCI::processGoCommand(string command) {
		// If currently searching dont process command
		if (goThread.joinable()) {
			return;
		}
		// Search position indefinitely, until stopped
		if (command == "go" || StringUtil::contains(command, "infinite")) {
			goThread = thread(&Bot::go, bot);
		} 
		// Search position for movetime milliseconds
		else if (StringUtil::contains(command, "movetime")) {
			int moveTimeMs = getLabelledValueInt(command, "movetime", goLabels); // Extract movetime number
			bot->generateMove(moveTimeMs); // Search position
		} 
		// Perft test, accepts user specified depth
		else if (StringUtil::contains(command, "perft")) {
			int searchDepth = getLabelledValueInt(command, "perft", goLabels); // Extract depth
			auto start = high_resolution_clock::now(); // Time the search

			uint64_t nodesSearched = bot->perft(searchDepth); // Get number of nodes

			auto end = high_resolution_clock::now();
			duration<double> duration = end - start;

			respond("Time taken: " + to_string(duration.count()) + "s, nodes per second: " + to_string(nodesSearched / duration.count()));
			respond("Nodes searched: " + to_string(nodesSearched));
		} 
		// Use real time clocks and increments to generate a move
		else {
			// Extract values
			int timeRemainingWhiteMs = getLabelledValueInt(command, "wtime", goLabels);
			int timeRemainingBlackMs = getLabelledValueInt(command, "btime", goLabels);
			int incrementWhiteMs = getLabelledValueInt(command, "winc", goLabels);
			int incrementBlackMs = getLabelledValueInt(command, "binc", goLabels);
			// Calculate think time for move
			int thinkTime = bot->chooseMoveTime(timeRemainingWhiteMs, timeRemainingBlackMs, incrementWhiteMs, incrementBlackMs);
			string str = "Thinking for: " + to_string(thinkTime);
			respond(str + " ms.");
			bot->generateMove(thinkTime); // Get move
		}

	}
	// Process position command, sets up position of board via FEN, start position, 
	// and optionally moves on given position
	void IUCI::processPositionCommand(string command) {
		// Do not process if currently searching
		if (goThread.joinable()) {
			return;
		}
		// If startpos position, reset to starting position
		if (StringUtil::contains(StringUtil::toLower(command), "startpos")) {
			bot->setPosition(FEN::startpos);
		} 
		// Else, extract FEN string and load that position
		else if (StringUtil::contains(StringUtil::toLower(command), "fen")) {
			string customFEN = getLabelledValue(command, "fen", positionLabels);
			bot->setPosition(customFEN);
		} else {
			return;
		}
		// If user provides moves to play on given position, enact them on board
		string allMoves = getLabelledValue(command, "moves", positionLabels);
		if (allMoves.size() > 0) {
			vector<string> moveList = StringUtil::splitString(allMoves);
			for (string move : moveList) {
				bot->makeMove(move);
			}

			logInfo("Make moves after setting position: " + to_string(moveList.size()));
		}
	}
	// Set an option from user input
	void IUCI::processSetOption(std::string command) {
		string optionName = getLabelledValue(command, "name", optionLabels);
		string optionValue = getLabelledValue(command, "value", optionLabels);

		optionHandler->processOption(optionName, optionValue);
	}
	// Prints response parameter and logs it to file
	void IUCI::respond(string response) {
		cout << response << endl;
		logInfo("Response: " + response);
		cout.flush();
	}

	template <typename T, std::size_t N>
	// Extracts integer accompanying a given label from a command
	int IUCI::getLabelledValueInt(string text, string label, const array<T, N> allLabels) {
		string valueString = getLabelledValue(text, label, allLabels); // Extract string integer
		string resultString = StringUtil::splitString(valueString)[0]; // Get first part

		if (!StringUtil::isDigitString(resultString)) {
			throw runtime_error("'" + resultString + "' is not an integer.'");
		}
		// Convert string to integer
		int value = stoi(resultString);

		if (value < 0) {
			throw runtime_error("'" + to_string(value) + "' is not a positive integer");
		}

		return value;
	}

	template <typename T, std::size_t N>
	// Extracts string accompanying a given label from a command
	string IUCI::getLabelledValue(string text, string label, const array<T, N> allLabels) {
		text = StringUtil::trim(text); // Delete leading and ending whitespace
		// If command does not contain given label, throw error
		if (!StringUtil::contains(text, label)) {
			throw runtime_error("'" + label + "' not found within '" + text + "'");
		}
		// Find start and end of where value should be
		int valueStart = StringUtil::indexOf(text, label) + label.size();
		int valueEnd = text.size();

		// Iterate over all labels and narrow start and end of value
		for (T label : allLabels) {
			if (label != label && StringUtil::contains(text, label.data())) {
				// If start of label is after valueStart and before valueEnd, 
				// it can narrow window for value
				int otherIDStartIndex = StringUtil::indexOf(text, label.data());
				if (otherIDStartIndex > valueStart && otherIDStartIndex < valueEnd) {
					valueEnd = otherIDStartIndex;
				}
			}
		}
		// After narrowing, trim value
		return StringUtil::trim(text.substr(valueStart, valueEnd - valueStart));
	}

	// Append text parameter to log file
	void IUCI::logInfo(string text) {
		ofstream outFile(logPath.data(), ios::app);
		if (!outFile.is_open()) {
			cerr << "Could not write to " << logPath << endl;
			outFile.close();
			return;
		}
		outFile << text << endl;
		outFile.close();
	}

}