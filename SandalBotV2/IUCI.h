#pragma once

#ifndef IUCI_H
#define IUCI_H

#include "Bot.h"
#include "StringUtil.h"

class IUCI {
private:
	Bot bot;
	static const std::string positionLabels[3];
	static const std::string goLabels[7];
	static const std::string logPath;
public:
	IUCI();
	void processCommand(std::string command);
	void OnMoveChosen(std::string move);
	void processGoCommand(std::string command);
	void processPositionCommand(std::string command);
	void respond(std::string response);
	int getLabelledValueInt(std::string text, std::string label, std::string allLabels[], int defaultValue = 0);
	std::string getLabelledValue(std::string text, std::string label, std::string allLabels[], std::string defaultValue = "");
	void logInfo(std::string text);
	std::string engineDataPath();
};

#endif // !IUCI_H
