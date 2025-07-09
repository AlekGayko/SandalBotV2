#include "OptionHandler.h"
#include "Bot.h"

namespace SandalBot {

	OptionHandler::OptionHandler(Bot* bot) {
		this->bot = bot;
		initOptions();
	}

	// Initialise options in map
	void OptionHandler::initOptions() {
		// Clears entire transposition table of values
		Option clearHash = {
			"Clear Hash",
			"type button",
			[this]([[maybe_unused]] std::string& value) {
				this->bot->clearHash();
			}
		};

		options[clearHash.name] = clearHash; // Add to map

		// Changes size of hash table (deletes contents as well)
		Option changeHashSize = {
			"Hash",
			"type spin default 128 min 1 max 2000",
			[this](std::string& value) {
				int valueInt = std::stoi(value);
				if (valueInt < 1 || valueInt > 2000) {
					return;
				}
				this->bot->changeHashSize(valueInt);
			}
		};

		options[changeHashSize.name] = changeHashSize; // Add to map of options
	}

	// Invoke option action function
	void OptionHandler::processOption(std::string optionName, std::string value) {
		if (options.find(optionName) == options.end()) { // Find option
			return;
		}

		options[optionName].changeSettings(value);
	}

	// Returns display string of options, for user interface
	std::string OptionHandler::getOptionsString() {
		std::string optionsString = "";
		for (auto& pair : options) {
			optionsString += "option name " + pair.first + " " + pair.second.description + "\n";
		}
		if (optionsString.size() > 0) {
			optionsString.pop_back();
		}
		return optionsString;
	}

}