#include "IUCI.h"

#include <cctype>
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;
using namespace chrono;
using namespace StringUtil;

const vector<string> IUCI::positionLabels = { "position", "fen", "moves" };
const vector<string> IUCI::goLabels = { "go", "movetime", "wtime", "btime", "winc", "binc", "movestogo", "perft" };
const string IUCI::logPath = "logs.txt";

void IUCI::beginningMessage() {
	cout << startingMessage << endl;
}

void IUCI::emptyLogs() {
	ofstream logs(logPath, ios::trunc);

	if (!logs.is_open()) {
		cerr << "Couldn't empty logs" << endl;
	}
	logs.close();

	return;
}

IUCI::IUCI() {
	beginningMessage();
	emptyLogs();
	bot = new Bot();
}

IUCI::~IUCI() {
	delete bot;
}

void IUCI::processCommand(string command) {
	try {
		//logInfo("Command received: " + command);
		command = trim(command);
		const string commandType = toLower(splitString(command)[0]);

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
		} else if (commandType == "d") {
			bot->printBoard();
		} else {
			logInfo("Unknown Command: " + commandType);
		}
	} catch (exception& e) {
		logInfo(e.what());
	}
}

void IUCI::newGame() {
	stop();
	delete bot;
	bot = new Bot();
}

void IUCI::stop() {
	
	if (goThread.joinable()) {
		bot->stopSearching();
		this_thread::sleep_for(chrono::milliseconds(100));
		goThread.join();
		this_thread::sleep_for(chrono::milliseconds(100));
	}
}

void IUCI::quit() {
	if (goThread.joinable()) {
		bot->stopSearching();
		goThread.join();
	}

	exit(0);
}

void IUCI::UCIok() {
	respond("id name " + name);
	respond("id author " + author);
	cout << endl;
	respond("uciok");
}

void IUCI::eval() {
	int evaluation = bot->eval();
	string sign = evaluation >= 0 ? "+" : "-";
	respond("evaluation " + sign + to_string((float) evaluation / 100.f));
}

void IUCI::OnMoveChosen(string move) {
	respond("bestmove " + move);
}

void IUCI::processGoCommand(string command) {
	if (goThread.joinable()) {
		return;
	}
	if (command == "go" || contains(command, "infinite")) {
		goThread = thread(&Bot::go, bot);
	} else if (contains(command, "movetime")) {
		int moveTimeMs = getLabelledValueInt(command, "movetime", goLabels);
		bot->generateMove(moveTimeMs);
	} else if (contains(command, "perft")) {
		int searchDepth = getLabelledValueInt(command, "perft", goLabels);
		auto start = high_resolution_clock::now();

		uint64_t nodesSearched = bot->perft(searchDepth);

		auto end = high_resolution_clock::now();
		duration<double> duration = end - start;

		respond("Time taken: " + to_string(duration.count()) + "s, nodes per second: " + to_string(nodesSearched / duration.count()));
		respond("Nodes searched: " + to_string(nodesSearched));
	} else {
		int timeRemainingWhiteMs = getLabelledValueInt(command, "wtime", goLabels);
		int timeRemainingBlackMs = getLabelledValueInt(command, "btime", goLabels);
		int incrementWhiteMs = getLabelledValueInt(command, "winc", goLabels);
		int incrementBlackMs = getLabelledValueInt(command, "binc", goLabels);

		int thinkTime = bot->chooseMoveTime(timeRemainingWhiteMs, timeRemainingBlackMs, incrementWhiteMs, incrementBlackMs);
		string str = "Thinking for: " + to_string(thinkTime);
		respond(str + " ms.");
		bot->generateMove(thinkTime);
	}
	
}

void IUCI::processPositionCommand(string command) {
	if (goThread.joinable()) {
		return;
	}
	if (contains(toLower(command), "startpos")) {
		bot->setPosition(FEN::startpos);
	} else if (contains(toLower(command), "fen")) {
		string customFEN = getLabelledValue(command, "fen", positionLabels);
		bot->setPosition(customFEN);
	} else {
		return;
	}

	string allMoves = getLabelledValue(command, "moves", positionLabels);
	if (allMoves.size() > 0) {
		vector<string> moveList = splitString(allMoves);
		for (string move : moveList) {
			bot->makeMove(move);
		}

		logInfo("Make moves after setting position: " + to_string(moveList.size()));
	}
}

void IUCI::respond(string response) {
	cout << response << endl;
	logInfo("Response: " + response);
	cout.flush();
}

int IUCI::getLabelledValueInt(string text, string label, const vector<string> allLabels) {
	string valueString = getLabelledValue(text, label, allLabels);
	string resultString = splitString(valueString)[0];

	if (!isDigitString(resultString)) {
		throw runtime_error("'" + resultString + "' is not an integer.'");
	}

	int value = stoi(resultString);

	if (value < 0) {
		throw runtime_error("'" + to_string(value) + "' is not a positive integer");
	}

	return value;
}

string IUCI::getLabelledValue(string text, string label, const vector<string> allLabels) {
	text = trim(text);

	if (!contains(text, label)) {
		throw runtime_error("'" + label + "' not found within '" + text + "'");
	}

	int valueStart = indexOf(text, label) + label.size();
	int valueEnd = text.size();

	for (int i = 0; i < allLabels.size(); i++) {
		if (allLabels[i] != label && contains(text, allLabels[i])) {
			int otherIDStartIndex = indexOf(text, allLabels[i]);
			if (otherIDStartIndex > valueStart && otherIDStartIndex < valueEnd) {
				valueEnd = otherIDStartIndex;
			}
		}
	}

	return trim(text.substr(valueStart, valueEnd - valueStart));
}



void IUCI::logInfo(string text) {
	ofstream outFile(logPath, ios::app);
	if (!outFile.is_open()) {
		cerr << "Could not write to " << logPath << endl;
		outFile.close();
		return;
	}
	outFile << text << endl;
	outFile.close();
}

string IUCI::engineDataPath() {
	return logPath;
}