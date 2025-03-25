#pragma once

#ifndef COORDHELPER_H
#define COORDHELPER_H

#include "StringUtil.h"

#include <string>

namespace SandalBot {

	// Coord encapsulates a coordinate
	struct Coord {
		int row{};
		int col{};
		Coord(int row, int col);
		Coord(int index);
		Coord operator+(const Coord& other) const;
		int operator+(const int& other) const;
		Coord operator*(const int other) const;
		friend std::ostream& operator<<(std::ostream& os, const Coord& coord);
	};

	// CoordHelper provides utility functions for dealing with coordinates in many forms
	namespace CoordHelper {
		// Human readable forms of rows and columns
		const std::string charFiles{ "abcdefgh" };
		const std::string charRanks{ "87654321" };

		std::string indexToString(int index);
		int stringToIndex(std::string str);
		constexpr int indexToRow(int index);
		constexpr int indexToCol(int index);
		int coordToIndex(Coord coord);
		int coordToIndex(int row, int col);
		bool validCoord(Coord coord);
		bool validCoordAddition(Coord coord, Coord direction);
		bool validCoordAddition(Coord coord, Coord direction, int scalar);
		bool validCoordAddition(int index, Coord direction);
	};

}

#endif