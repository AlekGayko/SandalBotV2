#include "IUCI.h"

#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

const vector<string> IUCI::positionLabels = { "position", "fen", "moves" };
const vector<string> IUCI::goLabels = { "go", "movetime", "wtime", "btime", "winc", "binc", "movestogo" };
const string IUCI::logPath = "logs.txt";

IUCI::IUCI() {
	bot = new Bot();
}

IUCI::~IUCI() {
	delete bot;
}

void IUCI::processCommand(string command) {
	logInfo("Command received: " + command);
	command = StringUtil::trim(command);
	const string commandType = StringUtil::toLower(StringUtil::splitString(command)[0]);

	if (commandType == "uci") {
		respond("uciok");
	} else if (commandType == "isready") {
		respond("readyok");
	} else if (commandType == "ucinewgame") {
		//player.NotifyNewGame();
	} else if (commandType == "position") {
		processPositionCommand(command);
	} else if (commandType == "go") {
		processGoCommand(command);
	} else if (commandType == "stop") {
		/*if (player.IsThinking)
				{
					player.StopThinking();
				}*/
	} else if (commandType == "quit") {
		//player.Quit();
	} else if (commandType == "d") {
		//Console.WriteLine(player.GetBoardDiagram());
	} else {
		logInfo("Unknown Command: " + commandType);
	}
}

void IUCI::OnMoveChosen(string move) {
	logInfo("OnMoveChosen: book mov = " /*+ player.LatestMoveIsBookMove*/);
	respond("bestmove " + move);
}

void IUCI::processGoCommand(string command) {
	if (StringUtil::contains(command, "movetime")) {
		int moveTimeMs = getLabelledValueInt(command, "movetime", goLabels, 0);
		// player.ThinkTimed(moveTimeMs);
	} else {
		int timeRemainingWhiteMs = getLabelledValueInt(command, "wtime", goLabels, 0);
		int timeRemainingBlackMs = getLabelledValueInt(command, "btime", goLabels, 0);
		int incrementWhiteMs = getLabelledValueInt(command, "winc", goLabels, 0);
		int incrementBlackMs = getLabelledValueInt(command, "binc", goLabels, 0);
		int thinkTime = 0;// = player.ChooseThinkTime(timeRemainingWhiteMs, timeRemainingBlackMs, incrementWhiteMs, incrementBlackMs);
		string str = "Thinking for: " + to_string(thinkTime);
		logInfo(str + " ms.");
		//player.ThinkTimed(thinkTime);
	}
	
}

void IUCI::processPositionCommand(string command) {
	if (StringUtil::contains(StringUtil::toLower(command), "startpos")) {
		//player.SetPosition(FenUtility.StartPositionFEN);
	} else if (StringUtil::contains(StringUtil::toLower(command), "fen")) {
		string customFEN = getLabelledValue(command, "fen", positionLabels);
		cout << customFEN << endl;
		//player.SetPosition(customFen);
	} else {
		cout << "Invalid position command (expected 'startpos' or 'fen')" << endl;
		return;
	}

	string allMoves = getLabelledValue(command, "moves", positionLabels);
	cout << allMoves << endl;
	if (allMoves.size() > 0) {
		vector<string> moveList = StringUtil::splitString(allMoves);
		for (string move : moveList) {
			//player.MakeMove(move);
		}

		logInfo("Make moves after setting position: " + to_string(moveList.size()));
	}
}

void IUCI::respond(string response) {
	cout << response << endl;
	logInfo("Response: " + response);
}

int IUCI::getLabelledValueInt(string text, string label, const vector<string> allLabels, int defaultValue) {
	string valueString = getLabelledValue(text, label, allLabels, defaultValue + "");
	string resultString = StringUtil::splitString(valueString)[0];
	if (StringUtil::isDigitString(resultString)) {
		return stoi(resultString);
	}
	return defaultValue;
}

string IUCI::getLabelledValue(string text, string label, const vector<string> allLabels, string defaultValue) {
	text = StringUtil::trim(text);
	if (StringUtil::contains(text, label)) {
		int valueStart = StringUtil::indexOf(text, label) + label.size();
		int valueEnd = text.size();
		for (int i = 0; i < allLabels.size(); i++) {
			if (allLabels[i] != label && StringUtil::contains(text, allLabels[i])) {
				int otherIDStartIndex = StringUtil::indexOf(text, allLabels[i]);
				if (otherIDStartIndex > valueStart && otherIDStartIndex < valueEnd) {
					valueEnd = otherIDStartIndex;
				}
			}
		}

		return StringUtil::trim(text.substr(valueStart, valueEnd - valueStart));
	}
	return defaultValue;
}



void IUCI::logInfo(string text) {
	ofstream outFile(logPath, ios::app);
	if (!outFile.is_open()) {
		cerr << "Could not write to " << logPath << endl;
		return;
	}
	outFile << text << endl;
	outFile.close();
}

string IUCI::engineDataPath() {
	return logPath;
}