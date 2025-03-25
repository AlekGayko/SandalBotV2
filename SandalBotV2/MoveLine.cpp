#include "MoveLine.h"

namespace SandalBot {

	MoveLine::MoveLine(int size) : capacity(size) {
		this->line = new Move[capacity];
		this->size = 0;

		for (int i = 0; i < capacity; i++) {
			line[i] = Move();
		}
	}

	// Standard push function
	void MoveLine::add(Move move) {
		if (line == nullptr)
			return;

		if (size >= capacity)
			return;

		line[size] = std::move(move);
		size++;
	}

	// Reset array
	void MoveLine::reset() {
		size = 0;
	}

	// Return human-readable string for line of moves
	std::string MoveLine::str() {
		std::string lineStr = "";

		if (line == nullptr)
			return lineStr;

		if (size >= capacity)
			return lineStr;

		for (size_t i = 0; i < size; i++) {
			lineStr += line[i].str();
			if (i != size - 1) {
				lineStr += " ";
			}
		}

		return lineStr;
	}

}