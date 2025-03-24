#ifndef MOVELINE_H
#define MOVELINE_H

#include "Move.h"

#include <string>

namespace SandalBot {

	class MoveLine {
	private:
		Move* line{ nullptr };
		size_t capacity{ 0 };
		size_t size{ 0 };
	public:
		MoveLine() : size(0) {}
		MoveLine(int size);
		~MoveLine() { delete[] line; }

		void add(Move move);
		void reset();
		std::string str();
	};

}

#endif