#include "Move.h"

#include <bitset>
#include <iostream>

namespace SandalBot {

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

	std::string Move::binStr() const {
		std::bitset<4> binFlag { getFlag() };
		std::bitset<6> binStart { getStartSquare() };
		std::bitset<6> binTarget{ getTargetSquare() };

		return binFlag.to_string() + " " + 
			binStart.to_string() + " " + binTarget.to_string();
	}

	std::ostream& operator<<(std::ostream& os, const Move& move) {
		os << move.str();
		return os;
	}

}