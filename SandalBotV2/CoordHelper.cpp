#include "CoordHelper.h"

#include <stdexcept>

const std::string CoordHelper::charFiles = "abcdefgh";
const std::string CoordHelper::charRanks = "87654321";

std::string CoordHelper::indexToString(int index) {
    int row = index / 8;
    int col = index % 8;
    std::string coord = std::string(1, charFiles[col]) + std::string(1, charRanks[row]);
    return coord;
}

int CoordHelper::stringToIndex(std::string str) {
    if (str.size() != 2) {
        throw std::length_error("Coordinate incorrect size: " + str.size());
    }

    int row = StringUtil::indexOf(charRanks, std::string(1, str[0]));
    int col = StringUtil::indexOf(charFiles, std::string(1, str[1]));

    return row * 8 + col;
}

constexpr int CoordHelper::indexToRow(int index) {
    return index / 8;
}

constexpr int CoordHelper::indexToCol(int index) {
    return index % 8;
}

int CoordHelper::coordToIndex(Coord coord) {
    return coord.row * 8 + coord.col;
}

bool CoordHelper::validCoordAddition(Coord coord, Coord direction) {
    if (coord.row + direction.row >= 8 || coord.row + direction.row < 0) {
        return false;
    }
    if (coord.col + direction.col < 0 || coord.col + direction.col >= 8) {
        return false;
    }
    return true;
}

bool CoordHelper::validCoord(Coord coord) {
    return coord.row < 8 && coord.row >= 0 && coord.col < 8 && coord.col >= 0;
}

bool CoordHelper::validCoordAddition(Coord coord, Coord direction, int scalar) {
    if (coord.row + direction.row * scalar >= 8 || coord.row + direction.row * scalar < 0) {
        return false;
    }
    if (coord.col + direction.col * scalar < 0 || coord.col + direction.col * scalar >= 8) {
        return false;
    }
    return true;
}

Coord::Coord(int row, int col) {
    this->row = row;
    this->col = col;
}

Coord::Coord(int index) {
    this->row = CoordHelper::indexToRow(index);
    this->col = CoordHelper::indexToCol(index);
}

Coord Coord::operator+(const Coord& other) const {
    return { row + other.row, col + other.col };
}

Coord Coord::operator*(const int other) const {
    return { row * other, col * other };
}
