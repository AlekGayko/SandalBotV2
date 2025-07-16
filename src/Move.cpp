#include "Move.h"

#include <bitset>
#include <iostream>

namespace SandalBot {
	// Returns the piece the move promotes to
	constexpr PieceType Move::promotionPieceType() {
		return PieceType(flag());
	}
	// Returns a string representing the move in UCI notation
	std::string Move::str() const {
		std::string str = "";
		str += CoordHelper::indexToString(from());
		str += CoordHelper::indexToString(to());

		switch (flag()) {
		case Flag::CASTLE:
			if (to() > from()) {
				str = "O-O";
			} else {
				str = "O-O-O";
			}
			break;
		case Flag::QUEEN:
			str += "q";
			break;
		case Flag::ROOK:
			str += "r";
			break;
		case Flag::BISHOP:
			str += "b";
			break;
		case Flag::KNIGHT:
			str += "n";
			break;
		}

		return str;
	}
	// Returns string representing the binary format of the 16 bit moveValue
	std::string Move::binStr() const {
		std::bitset<4> binFlag { static_cast<uint64_t>(flag()) };
		std::bitset<6> binStart { static_cast<uint64_t>(from()) };
		std::bitset<6> binTarget{ static_cast<uint64_t>(to()) };

		return binFlag.to_string() + " " + 
			binStart.to_string() + " " + binTarget.to_string();
	}
	// Override operator to print move
	std::ostream& operator<<(std::ostream& os, const Move& move) {
		os << move.str();
		return os;
	}

}