#include "CoordHelper.h"

#include <iostream>
#include <stdexcept>

using namespace std;

namespace SandalBot {

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

    // Override print operator for Coord
    std::ostream& operator<<(std::ostream& os, const Coord& coord) {
        os << "{ " << coord.row << ", " << coord.col << " }" << endl;
        return os;
    }

    namespace CoordHelper {
        // Converts an index from a linear board, to human-readable coordinate
        std::string CoordHelper::indexToString(int index) {
            int row = index / 8;
            row = row < 0 || row > 7 ? 0 : row;
            int col = index % 8;
            std::string coord = std::string(1, charFiles[col]) + std::string(1, charRanks[row]);
            return coord;
        }
        // Converts human-readable coordinate to an index on a linear board
        int CoordHelper::stringToIndex(std::string str) {
            if (str.size() != 2) {
                throw std::length_error("Coordinate incorrect size: " + str.size());
            }

            int row = StringUtil::indexOf(charRanks, std::string(1, str[1]));
            int col = StringUtil::indexOf(charFiles, std::string(1, str[0]));
            return row * 8 + col;
        }
        // Returns row from index
        constexpr int CoordHelper::indexToRow(int index) {
            return index / 8;
        }
        // Returns column from index
        constexpr int CoordHelper::indexToCol(int index) {
            return index % 8;
        }
        // Returns index from coordinate
        int CoordHelper::coordToIndex(Coord coord) {
            return coord.row * 8 + coord.col;
        }
        // Returns index from row and column
        int CoordHelper::coordToIndex(int row, int col) {
            return row * 8 + col;
        }
        // Returns boolean, depending on whether vector addition from two Coord's
        // will be out of bounds of the board
        bool CoordHelper::validCoordAddition(Coord coord, Coord direction) {
            if (coord.row + direction.row >= 8 || coord.row + direction.row < 0) {
                return false;
            }
            if (coord.col + direction.col < 0 || coord.col + direction.col >= 8) {
                return false;
            }
            return true;
        }
        // Returns boolean if addition between index and Coord remains in bounds of board
        bool CoordHelper::validCoordAddition(int index, Coord direction) {
            int result = index + direction.row * 8 + direction.col;
            int resultCol = index % 8 + direction.col;
            if (result < 0 || result > 63) return false;
            if (resultCol < 0 || resultCol > 7) return false;
            return true;
        }
        // Returns true if Coord is within bounds of board, false otherwise
        bool CoordHelper::validCoord(Coord coord) {
            return coord.row < 8 && coord.row >= 0 && coord.col < 8 && coord.col >= 0;
        }
        // Returns boolean on whether a direction multiplied by a scalar can be added to a coord
        // while remaining in bounds. True if in bounds, false otherwise.
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

}