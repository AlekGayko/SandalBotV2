#include "IUCI.h"

#include <fstream>
#include <iostream>
#include <string>

using namespace std;

const std::string IUCI::positionLabels[3] = { "position", "fen", "moves" };
const std::string IUCI::goLabels[7] = { "go", "movetime", "wtime", "btime", "winc", "binc", "movestogo" };
const std::string IUCI::logPath = "logs.txt";

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
}

void IUCI::processGoCommand(string command) {
}

void IUCI::processPositionCommand(string command) {
}

void IUCI::respond(string response) {
	cout << response << endl;
	logInfo("Response: " + response);
}

int IUCI::getLabelledValueInt(string text, string label, string allLabels[], int defaultValue) {
	return 0;
}

string IUCI::getLabelledValue(string text, string label, string allLabels[], string defaultValue) {
	return string();
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