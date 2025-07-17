#ifndef MOVELINE_H
#define MOVELINE_H

#include "Move.h"

#include <string>

namespace SandalBot {
	
	// MoveLine stores an array of moves in a dynamic array,
	// used for storing a principal variation from a search
	class MoveLine {
	public:
		MoveLine() {}
		MoveLine(int size);
		~MoveLine() { delete[] line; }
		
		MoveLine& operator=(const MoveLine& other);

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