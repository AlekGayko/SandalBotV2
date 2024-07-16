#pragma once

#ifndef IUCI_H
#define IUCI_H

#include <string>
#include <vector>

#include "Bot.h"
#include "FEN.h"
#include "StringUtil.h"

class IUCI {
private:
	Bot* bot = nullptr;
	static const std::vector<std::string> positionLabels;
	static const std::vector<std::string> goLabels;
	static const std::string logPath;
	const std::string startingMessage = "SandalBotV2 by DirtySandals.";
	void beginningMessage();
	void emptyLogs();
public:
	IUCI();
	~IUCI();
	void processCommand(std::string command);
	void newGame();
	void OnMoveChosen(std::string move);
	void processGoCommand(std::string command);
	void processPositionCommand(std::string command);
	void respond(std::string response);
	int getLabelledValueInt(std::string text, std::string label, const std::vector<std::string> allLabels, int defaultValue = 0);
	std::string getLabelledValue(std::string text, std::string label, const std::vector<std::string> allLabels, std::string defaultValue = "");
	void logInfo(std::string text);
	std::string engineDataPath();
};

#endif

