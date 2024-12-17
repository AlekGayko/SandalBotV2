#pragma once
#ifndef MOVELINE_H
#define MOVELINE_H

#include "Move.h"

#include <string>

class MoveLine {
private:
	Move* line = nullptr;
	size_t capacity = 0;
	size_t size = 0;
public:
	MoveLine();
	MoveLine(int size);
	~MoveLine();
	void add(Move& move);
	void reset();
	std::string str();
};

#endif