#ifndef OPTIONHANDLER_H
#define OPTIONHANDLER_H

#include <iostream>
#include <functional>
#include <unordered_map>
#include <variant>

namespace SandalBot {

	class Bot;

	using ConfigValue = std::variant<int, float, std::string, bool>;

	// OptionHandler handles the setting of options from the users
	// to change the current configuration of the bot
	class OptionHandler {
	private:
		// Option represents option with name, description, and action function
		struct Option {
			std::string name{};
			std::string description{};
			std::function<void(std::string&)> changeSettings{};
		};

		Bot* bot{ nullptr };
		std::unordered_map<std::string, Option> options{};
	public:
		OptionHandler(Bot* bot);
		void initOptions();
		void processOption(std::string optionName, std::string value);
		std::string getOptionsString();
	};

}

#endif // !OPTIONHANDLER_H
