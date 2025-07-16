#ifndef TYPES_H
#define TYPES_H

#include <cstdint>

namespace SandalBot {

	using Bitboard = uint64_t;
	using HashKey = uint64_t;

	enum Color: int {
		BLACK, // 0
		WHITE, // 1
		COLOR_NB
	};

	enum Square : int {
		A8, B8, C8, D8, E8, F8, G8, H8,
		A7, B7, C7, D7, E7, F7, G7, H7,
		A6, B6, C6, D6, E6, F6, G6, H6,
		A5, B5, C5, D5, E5, F5, G5, H5,
		A4, B4, C4, D4, E4, F4, G4, H4,
		A3, B3, C3, D3, E3, F3, G3, H3,
		A2, B2, C2, D2, E2, F2, G2, H2,
		A1, B1, C1, D1, E1, F1, G1, H1,
		NONE_SQUARE,

		START_SQUARE = A8,
		SQUARES_NB = 64
	};

	enum Column : int {
		COL_A, COL_B, COL_C, COL_D, COL_E, COL_F, COL_G, COL_H, COL_NB, COL_START = COL_A
	};

	enum Row : int {
		ROW_8, ROW_7, ROW_6, ROW_5, ROW_4, ROW_3, ROW_2, ROW_1, ROW_NB, ROW_START = ROW_8
	};

	enum Direction : int {
		NORTH = -8,
		EAST = 1,
		SOUTH = -NORTH,
		WEST = -EAST,

		NORTH_EAST = NORTH + EAST,
		NORTH_WEST = NORTH + WEST,
		SOUTH_EAST = SOUTH + EAST,
		SOUTH_WEST = SOUTH + WEST
	};

	enum CastlingRights {
		NO_RIGHTS,
		B_OO = 0b0001,
		B_OOO = 0b0010,
		W_OO = 0b0100,
		W_OOO = 0b1000,

		W_RIGHTS = W_OO | W_OOO,
		B_RIGHTS = B_OO | B_OOO,
		SHORT_RIGHTS = W_OO | B_OO,
		LONG_RIGHTS = W_OOO | B_OOO,
		ALL_RIGHTS = W_RIGHTS | B_RIGHTS,

		RIGHTS_NB = 0b10000,

		B_KING_SQUARE = E8,
		W_KING_SQUARE = E1,

		B_ROOK_LONG_SQ = A8,
		B_ROOK_SHORT_SQ = H8,
		W_ROOK_LONG_SQ = A1,
		W_ROOK_SHORT_SQ = H1
	};

	constexpr Bitboard shortCastleCheckSQ[COLOR_NB] = { 1ULL << F1 | 1ULL << G1, 1ULL << F8 | 1ULL << G8 };
	constexpr Bitboard longCastleCheckSQ[COLOR_NB] = { 1ULL << D1 | 1ULL << C1, 1ULL << D8 | 1ULL << C8 };

	constexpr Bitboard emptyShortCastleSQ[COLOR_NB] = { shortCastleCheckSQ[0], shortCastleCheckSQ[1] };
	constexpr Bitboard emptyLongCastleSQ[COLOR_NB] = { shortCastleCheckSQ[0] | 1ULL << B1, shortCastleCheckSQ[1] | 1ULL << B8 };

	enum PieceType {
		NO_PIECE_TYPE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, // 0 - 6
		PIECE_TYPE_NB,
		ALL_PIECES = 0,
	};

	enum Piece {
		NO_PIECE, // 0
		B_PAWN = PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING, // 1 - 6
		W_PAWN = PAWN + 8, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING, // 9 - 14
		PIECE_NB = 15
	};

	constexpr int pieceMask{ 0b0111 };
	constexpr int colorMask{ 0b1000 };

	// Use operator overload macros from StockFish
	#define ENABLE_BASE_OPERATORS_ON(T)                                \
	constexpr T operator+(T d1, int d2) { return T(int(d1) + d2); }    \
	constexpr T operator-(T d1, int d2) { return T(int(d1) - d2); }    \
	constexpr T operator-(T d) { return T(-int(d)); }                  \
	inline T& operator+=(T& d1, int d2) { return d1 = d1 + d2; }       \
	inline T& operator-=(T& d1, int d2) { return d1 = d1 - d2; }

	#define ENABLE_INCR_OPERATORS_ON(T)                                \
	inline T& operator++(T& d) { return d = T(int(d) + 1); }           \
	inline T& operator--(T& d) { return d = T(int(d) - 1); }

	#define ENABLE_FULL_OPERATORS_ON(T)                                \
	ENABLE_BASE_OPERATORS_ON(T)                                        \
	constexpr T operator*(int i, T d) { return T(i * int(d)); }        \
	constexpr T operator*(T d, int i) { return T(int(d) * i); }        \
	constexpr T operator/(T d, int i) { return T(int(d) / i); }        \
	constexpr int operator/(T d1, T d2) { return int(d1) / int(d2); }  \
	inline T& operator*=(T& d, int i) { return d = T(int(d) * i); }    \
	inline T& operator/=(T& d, int i) { return d = T(int(d) / i); }

	ENABLE_INCR_OPERATORS_ON(PieceType)
	ENABLE_INCR_OPERATORS_ON(Piece)
	ENABLE_INCR_OPERATORS_ON(Square)
	ENABLE_INCR_OPERATORS_ON(Row)
	ENABLE_INCR_OPERATORS_ON(Column)
	ENABLE_INCR_OPERATORS_ON(CastlingRights)

	#undef ENABLE_FULL_OPERATORS_ON
	#undef ENABLE_INCR_OPERATORS_ON
	#undef ENABLE_BASE_OPERATORS_ON

	constexpr Square operator+(Square sq, Direction dir) { return Square(int(sq) + int(dir)); }
	constexpr Square operator-(Square s, Direction d) { return Square(int(s) - int(d)); }
	inline Square& operator+=(Square& s, Direction d) { return s = s + d; }
	inline Square& operator-=(Square& s, Direction d) { return s = s - d; }

	constexpr bool operator==(Square sq, CastlingRights cr) { return int(sq) == int(cr); }

	constexpr Piece makePiece(PieceType pieceType, Color color) { return color == WHITE? Piece(pieceType | colorMask) : Piece(pieceType); }

	constexpr Color colorOf(Piece piece) { return Color((piece & colorMask) >> 3); }

	constexpr PieceType typeOf(Piece piece) { return PieceType(piece & pieceMask); }

	constexpr bool isColor(Piece piece, Color color) { return colorOf(piece) == color && piece != NO_PIECE; }

	constexpr bool isType(Piece piece, PieceType pieceType) { return (piece & pieceMask) == pieceType; }

	constexpr bool isPiece(Piece piece1, Piece piece2) { return piece1 == piece2; }

	constexpr bool isSlidingPiece(Piece piece) {
		PieceType pieceType = PieceType(piece & pieceMask);
		return pieceType == QUEEN || pieceType == ROOK || pieceType == BISHOP;
	}

	constexpr bool isOrthogonal(Piece piece) {
		PieceType pieceType = PieceType(piece & pieceMask);
		return pieceType == QUEEN || pieceType == ROOK;
	}

	constexpr bool isDiagonal(Piece piece) {
		PieceType pieceType = PieceType(piece & pieceMask);
		return pieceType == QUEEN || pieceType == BISHOP;
	}

	Piece symbolToPiece(char symbol);
	char pieceToSymbol(Piece piece);

	constexpr Row toRow(Square sq) {
		return Row(sq >> 3);
	}

	constexpr Column toCol(Square sq) {
		return Column(sq & 7);
	}

	constexpr Color operator~(Color color) {
		return Color(color ^ WHITE);
	}

	constexpr Square flipRow(Square sq) { // Swap A1 to A8
		return Square(sq ^ A1);
	}

	constexpr Square flipCol(Square sq) { // Swap A1 to H8
		return Square(sq ^ H8);
	}

	constexpr Piece operator~(Piece pc) {
		return Piece(pc ^ colorMask);
	}

	constexpr Direction pawnPush(Color c) {
		return c == WHITE ? NORTH : SOUTH;
	}

	constexpr CastlingRights operator&(CastlingRights cr1, CastlingRights cr2) {
		return CastlingRights(uint_fast8_t(cr1) & uint_fast8_t(cr2));
	}

	constexpr CastlingRights operator|(CastlingRights cr1, CastlingRights cr2) {
		return CastlingRights(uint_fast8_t(cr1) | uint_fast8_t(cr2));
	}

	constexpr CastlingRights& operator|=(CastlingRights& cr1, CastlingRights cr2) {
		return cr1 = (cr1 | cr2);
	}

	constexpr CastlingRights operator&(Color c, CastlingRights cr) {
		return CastlingRights((c == WHITE ? W_RIGHTS : B_RIGHTS) & cr);
	}

	constexpr CastlingRights cancelCastlingRights(CastlingRights cr, Color c) {
		return ~c & cr;
	}

	constexpr CastlingRights cancelRookCastlingRights(CastlingRights cr, Color c, Square from) {
		if (c == WHITE) {
			if (from == W_ROOK_SHORT_SQ) {
				return CastlingRights(cr & W_OO);
			} else if (from == W_ROOK_LONG_SQ) {
				return CastlingRights(cr & W_OOO);
			}
		} else {
			if (from == B_ROOK_SHORT_SQ) {
				return CastlingRights(cr & B_OO);
			} else if (from == B_ROOK_LONG_SQ) {
				return CastlingRights(cr & B_OOO);
			}
		}

		return cr;
	}

	constexpr Square rCastleFrom(Square kingFrom, Square kingTo) {
		return kingFrom + Direction(kingTo > kingFrom ? 3 * EAST : 4 * WEST);
	}

	constexpr Square rCastleTo(Square kingFrom, Square kingTo) {
		return kingFrom + (kingTo > kingFrom ? EAST : WEST);
	}

	// Determine whether side can short castle
	constexpr bool canShortCastle(Color c, CastlingRights cr) {
		return bool(c == WHITE ? cr & W_OO : cr & B_OO);
	}
	// Determine whether side can long castle
	constexpr bool canLongCastle(Color c, CastlingRights cr) {
		return bool(c == WHITE ? cr & W_OOO : cr & B_OOO);
	}
}

#endif