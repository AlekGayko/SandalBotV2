#ifndef MOVELINE_H
#define MOVELINE_H

#include "Move.h"

#include <string>

namespace SandalBot {
	
	// MoveLine stores an array of moves in a dynamic array,
	// used for storing a principal variation from a search
	class MoveLine {
	public:
		MoveLine() : size(0) {}
		MoveLine(int size);
		~MoveLine() { delete[] line; }

		void add(Move move);
		void reset();
		std::string str();
	private:
		Move* line{ nullptr }; // Array of moves
		size_t capacity{ 0 };
		size_t size{ 0 };
	};

}

#endif