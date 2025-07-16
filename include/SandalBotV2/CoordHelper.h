#ifndef COORDHELPER_H
#define COORDHELPER_H

#include "StringUtil.h"

#include <string>
#include <string_view>

namespace SandalBot {

	using namespace std::literals::string_view_literals;

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
		constexpr std::string_view charFiles{ "abcdefgh"sv };
		constexpr std::string_view charRanks{ "87654321"sv };

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