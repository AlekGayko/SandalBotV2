#ifndef MOVE_H
#define MOVE_H

#include <bitset>

#include "CoordHelper.h"
#include "Types.h"

namespace SandalBot {

	// Move encapsulates a chess move into a 16 bit value. Split into a 4-6-6 segmentation,
	// the 16 bit value holds the flag of the move (type), the starting square, and the 
	// target square.
	struct Move {
		enum class Flag {
			KNIGHT = KNIGHT,
			BISHOP = BISHOP,
			ROOK = ROOK,
			QUEEN = QUEEN,
			CASTLE,
			PAWN_TWO_SQUARES,
			EN_PASSANT,
			NO_FLAG
		};

		Move() {}
		Move(uint16_t move)
			: moveValue(move) {
		}
		Move(Square startingSquare, Square to)
			: moveValue((startingSquare << 6) | to) {
		}
		Move(Square startingSquare, Square to, Move::Flag flag)
			: moveValue((uint_fast16_t(flag) << 12) | (startingSquare << 6) | to) {
		}
		Move(const Move& other)
			: moveValue(other.moveValue) {
		}
		Move(Move&& move) noexcept
			: moveValue(move.moveValue) {
		}

		constexpr bool operator==(const Move& other) const { return moveValue == other.moveValue; }
		constexpr Move& operator=(const Move& other) { this->moveValue = other.moveValue;	return *this; }
		constexpr Move& operator=(Move&& other) noexcept { moveValue = other.moveValue; return *this; }
		constexpr bool operator!=(const Move& other) const { return moveValue != other.moveValue; }

		constexpr bool isPromotion() const { return flag() <= Flag::QUEEN; }
		constexpr PieceType promotionPieceType();

		constexpr Square from() const { return Square((moveValue & startingSquareMask) >> 6); }
		constexpr Square to() const { return Square(moveValue & toMask); }
		constexpr Flag flag() const { return Flag((moveValue & flagMask) >> 12); }

		std::string str() const;
		std::string binStr() const;
		friend std::ostream& operator<<(std::ostream& os, const Move& move);

		// Segment masks
		static constexpr uint16_t startingSquareMask{ 0b111111000000 };
		static constexpr uint16_t toMask{ 0b000000111111 };
		static constexpr uint16_t flagMask{ 0b1111 << 12 };

		uint16_t moveValue {}; // Single member
	};

}

#endif // !MOVE_H
