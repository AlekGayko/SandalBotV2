#ifndef MOVE_H
#define MOVE_H

#include <bitset>

#include "CoordHelper.h"
#include "Piece.h"

namespace SandalBot {

	// Move encapsulates a chess move into a 16 bit value. Split into a 4-6-6 segmentation,
	// the 16 bit value holds the flag of the move (type), the starting square, and the 
	// target square.
	struct Move {
		// Segment masks
		static constexpr uint16_t startingSquareMask{ 0b111111000000 };
		static constexpr uint16_t targetSquareMask{ 0b000000111111 };
		static constexpr uint16_t flagMask{ 0b1111 << 12 };

		// Masks for flag types
		static constexpr uint16_t noFlag{ 0b0000 };
		static constexpr uint16_t enPassantCaptureFlag{ 0b0001 };
		static constexpr uint16_t pawnTwoSquaresFlag{ 0b0010 };
		static constexpr uint16_t castleFlag{ 0b0011 };
	
		static constexpr uint16_t promoteToQueenFlag{ 0b0100 };
		static constexpr uint16_t promoteToRookFlag{ 0b0101 };
		static constexpr uint16_t promoteToBishopFlag{ 0b0110 };
		static constexpr uint16_t promoteToKnightFlag{ 0b0111 };

		uint16_t moveValue {}; // Single member

		Move() {}
		Move(uint16_t move) 
			: moveValue(move) {}
		Move(uint16_t startingSquare, uint16_t targetSquare) 
			: moveValue((startingSquare << 6) | targetSquare) {}
		Move(uint16_t startingSquare, uint16_t targetSquare, uint16_t flag) 
			: moveValue((flag << 12) | (startingSquare << 6) | targetSquare) {}
		Move(const Move& other)
			: moveValue(other.moveValue) {}
		Move(Move&& move) noexcept 
			: moveValue(move.moveValue) {}

		bool operator==(const Move& other) const { return moveValue == other.moveValue; }
		Move& operator=(const Move& other) { this->moveValue = other.moveValue;	return *this; }
		Move& operator=(Move&& other) noexcept { moveValue = other.moveValue; return *this; }
		bool operator!=(const Move& other) const { return moveValue != other.moveValue; }

		bool isPromotion() const { return getFlag() >= promoteToQueenFlag; }
		bool isEnPassant() const { return getFlag() == enPassantCaptureFlag; }
		bool isCastle() const { return getFlag() == castleFlag; }
		int promotionPieceType();

		uint16_t getStartSquare() const { return (moveValue & startingSquareMask) >> 6; }
		uint16_t getTargetSquare() const { return moveValue & targetSquareMask; }
		uint16_t getFlag() const { return (moveValue & flagMask) >> 12; }

		std::string str() const;
		std::string binStr() const;
		friend std::ostream& operator<<(std::ostream& os, const Move& move);
	};

}

#endif // !MOVE_H
