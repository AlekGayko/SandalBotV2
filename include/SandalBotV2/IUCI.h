#ifndef IUCI_H
#define IUCI_H

#include <array>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "Bot.h"
#include "FEN.h"
#include "OptionHandler.h"
#include "StringUtil.h"

namespace SandalBot {

	using namespace std::literals::string_view_literals;

	// IUCI is an interface class which handles user input and relays information
	// to the SandalBot or Bot class. IUCI implements UCI (Universal Chess Interface) protocol
	// so it can be integrated wherever other bots can.
	class IUCI {
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
		template <typename T, std::size_t N>
		int getLabelledValueInt(std::string text, std::string label, const std::array<T, N> allLabels);
		template <typename T, std::size_t N>
		std::string getLabelledValue(std::string text, std::string label, const std::array<T, N> allLabels);
		void logInfo(std::string text);
	private:
		Bot* bot{ nullptr };
		OptionHandler* optionHandler{ nullptr };
		std::thread goThread{}; // Thread for asynchronous searching
		// Label vectors contain key words for specific commands to aid parsing commands
		const std::array<std::string_view, 3> positionLabels { "position"sv, "fen"sv, "moves"sv };
		const std::array<std::string_view, 8> goLabels { "go"sv, "movetime"sv, "wtime"sv, "btime"sv, "winc"sv, "binc"sv, "movestogo"sv, "perft"sv };
		const std::array<std::string_view, 2> optionLabels { "name"sv, "value"sv };

		const std::string_view logPath { "logs.txt"sv }; // filePath for log file
		// Data for starting message
		const char* name { "SandalBotV2" };
		const char* author { "DirtySandals" };

		void beginningMessage();
		void emptyLogs();
	};

}

#endif

