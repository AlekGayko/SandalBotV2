#pragma once

#ifndef COORDHELPER_H
#define COORDHELPER_H

#include "StringUtil.h"

#include <string>

struct Coord {
	int row;
	int col;
	Coord(int row, int col);
	Coord(int index);
};

class CoordHelper {
public:
	static const std::string charFiles;
	static const std::string charRanks;
	static constexpr std::string indexToString(int index);
	static constexpr int stringToIndex(std::string str);
	static constexpr int indexToRow(int index);
	static constexpr int indexToCol(int index);
	static constexpr bool validCoord(Coord coord);
	static constexpr bool validCoordAddition(Coord coord, Coord direction);
	static constexpr bool validCoordAddition(Coord coord, Coord direction, int scalar);
};

#endif