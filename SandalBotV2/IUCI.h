#ifndef IUCI_H
#define IUCI_H

#include <string>
#include <thread>
#include <vector>

#include "Bot.h"
#include "FEN.h"
#include "OptionHandler.h"
#include "StringUtil.h"

namespace SandalBot {

	// IUCI is an interface class which handles user input and relays information
	// to the SandalBot or Bot class. IUCI implements UCI (Universal Chess Interface) protocol
	// so it can be integrated wherever other bots can.
	class IUCI {
	private:
		Bot* bot{ nullptr };
		OptionHandler* optionHandler{ nullptr };
		std::thread goThread{}; // Thread for asynchronous searching
		// Label vectors contain key words for specific commands to aid parsing commands
		static const std::vector<std::string> positionLabels;
		static const std::vector<std::string> goLabels;
		static const std::vector<std::string> optionLabels;

		static const std::string logPath; // filePath for log file
		// Data for starting message
		const std::string name{ "SandalBotV2" };
		const std::string author{ "DirtySandals" };
		const std::string startingMessage{ name + " by " + author + "." };

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
		void processSetOption(std::string command);
		void respond(std::string response);
		int getLabelledValueInt(std::string text, std::string label, const std::vector<std::string> allLabels);
		std::string getLabelledValue(std::string text, std::string label, const std::vector<std::string> allLabels);
		void logInfo(std::string text);
	};

}

#endif

