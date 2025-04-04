#include "Move.h"

#include <bitset>
#include <iostream>

namespace SandalBot {
	// Returns the piece the move promotes to
	int Move::promotionPieceType() {
		switch (getFlag()) {
		case promoteToQueenFlag:
			return Piece::queen;
		case promoteToRookFlag:
			return Piece::rook;
		case promoteToBishopFlag:
			return Piece::bishop;
		case promoteToKnightFlag:
			return Piece::knight;
		default:
			return 0;
		}
	}
	// Returns a string representing the move in UCI notation
	std::string Move::str() const {
		std::string str = "";
		str += CoordHelper::indexToString(getStartSquare());
		str += CoordHelper::indexToString(getTargetSquare());

		switch (getFlag()) {
		case castleFlag:
			if (getTargetSquare() > getStartSquare()) {
				str = "O-O";
			} else {
				str = "O-O-O";
			}
			break;
		case promoteToQueenFlag:
			str += "q";
			break;
		case promoteToRookFlag:
			str += "r";
			break;
		case promoteToBishopFlag:
			str += "b";
			break;
		case promoteToKnightFlag:
			str += "n";
			break;
		}

		return str;
	}
	// Returns string representing the binary format of the 16 bit moveValue
	std::string Move::binStr() const {
		std::bitset<4> binFlag { getFlag() };
		std::bitset<6> binStart { getStartSquare() };
		std::bitset<6> binTarget{ getTargetSquare() };

		return binFlag.to_string() + " " + 
			binStart.to_string() + " " + binTarget.to_string();
	}
	// Override operator to print move
	std::ostream& operator<<(std::ostream& os, const Move& move) {
		os << move.str();
		return os;
	}

}