#pragma once

#ifndef IUCI_H
#define IUCI_H

#include <string>
#include <thread>
#include <vector>

#include "Bot.h"
#include "FEN.h"
#include "StringUtil.h"

class IUCI {
private:
	Bot* bot = nullptr;
	std::thread goThread;
	static const std::vector<std::string> positionLabels;
	static const std::vector<std::string> goLabels;
	static const std::string logPath;
	const std::string name = "SandalBotV2";
	const std::string author = "DirtySandals";
	const std::string startingMessage = name + " by " + author + ".";
	void beginningMessage();
	void emptyLogs();
public:
	IUCI();
	~IUCI();
	void processCommand(std::string command);
	void newGame();
	void stop();
	void quit();
	void UCIok();
	void eval();
	void OnMoveChosen(std::string move);
	void processGoCommand(std::string command);
	void processPositionCommand(std::string command);
	void respond(std::string response);
	int getLabelledValueInt(std::string text, std::string label, const std::vector<std::string> allLabels);
	std::string getLabelledValue(std::string text, std::string label, const std::vector<std::string> allLabels);
	void logInfo(std::string text);
	std::string engineDataPath();
};

#endif

