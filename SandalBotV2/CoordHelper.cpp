#include "CoordHelper.h"

#include <iostream>
#include <stdexcept>

using namespace std;
using namespace StringUtil;

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

int Coord::operator+(const int& other) const {
    return other + row * 8 + col;
}

Coord Coord::operator*(const int other) const {
    return { row * other, col * other };
}

std::ostream& operator<<(std::ostream& os, const Coord& coord) {
    os << "{ " << coord.row << ", " << coord.col << " }" << endl;
    return os;
}

namespace CoordHelper {
    std::string CoordHelper::indexToString(int index) {
        int row = index / 8;
        row = row < 0 || row > 7 ? 0 : row;
        int col = index % 8;
        std::string coord = std::string(1, charFiles[col]) + std::string(1, charRanks[row]);
        return coord;
    }

    int CoordHelper::stringToIndex(std::string str) {
        if (str.size() != 2) {
            throw std::length_error("Coordinate incorrect size: " + str.size());
        }

        int row = indexOf(charRanks, std::string(1, str[1]));
        int col = indexOf(charFiles, std::string(1, str[0]));
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

    int CoordHelper::coordToIndex(int row, int col) {
        return row * 8 + col;
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

    bool CoordHelper::validCoordAddition(int index, int move) {
        if (index + move < 0 || index + move > 63) return false;
        int indexCol = index % 8;
        int moveCol = move < 0 ? (move % 8) : move % 8;

        int resultCol = indexCol + moveCol;
        if (resultCol < 0 || resultCol >= 8) return false;
        return true;
    }

    bool CoordHelper::validCoordAddition(int index, Coord direction) {
        int result = index + direction.row * 8 + direction.col;
        int resultCol = index % 8 + direction.col;
        if (result < 0 || result > 63) return false;
        if (resultCol < 0 || resultCol > 7) return false;
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
}