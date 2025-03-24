#ifndef PIECE_H
#define PIECE_H

#include <bitset>

namespace SandalBot {

	class Piece {
	public:
		static constexpr int king{ 0b0110 }; // 6
		static constexpr int queen{ 0b0101 }; // 5
		static constexpr int rook{ 0b0100 }; // 4
		static constexpr int bishop{ 0b0011 }; // 3
		static constexpr int knight{ 0b0010 }; // 2
		static constexpr int pawn{ 0b0001 }; // 1
		static constexpr int empty{ 0b0000 }; // 0

		static constexpr int black{ 0b0000 }; // 0
		static constexpr int white{ 0b1000 }; // 8

		static constexpr int blackKing{ king | black }; // 6
		static constexpr int blackQueen{ queen | black }; // 5
		static constexpr int blackRook{ rook | black }; // 4
		static constexpr int blackBishop{ bishop | black }; // 3
		static constexpr int blackKnight{ knight | black }; // 2
		static constexpr int blackPawn{ pawn | black }; // 1

		static constexpr int whiteKing{ king | white }; // 14
		static constexpr int whiteQueen{ queen | white }; // 13
		static constexpr int whiteRook{ rook | white }; // 12
		static constexpr int whiteBishop{ bishop | white }; // 11
		static constexpr int whiteKnight{ knight | white }; // 10
		static constexpr int whitePawn{ pawn | white }; // 9

		static constexpr int pieceMask{ 0b0111 };
		static constexpr int colorMask{ 0b1000 };

		static constexpr int pieces[12] {
			blackKing, blackQueen, blackRook, blackBishop, blackKnight, blackPawn,
			whiteKing, whiteQueen, whiteRook, whiteBishop, whiteKnight, whitePawn
		};

		static constexpr int makePiece(int pieceType, int color) { return pieceType | color; }
		static constexpr int makePiece(int pieceType, bool isWhite) { return makePiece(pieceType, isWhite ? white : black); }
		static constexpr int color(int piece) { return piece & colorMask; }
		static constexpr int type(int piece) { return piece & pieceMask; }
		static constexpr bool isColor(int piece, int color) { return (piece & colorMask) == color && piece != empty; }
		static constexpr bool isType(int piece, int pieceType) { return (piece & pieceMask) == pieceType; }
		static constexpr bool isPiece(int piece1, int piece2) { return piece1 == piece2; }
		static constexpr bool isSlidingPiece(int piece) {
			int pieceType = piece & pieceMask;
			return pieceType == queen || pieceType == rook || pieceType == bishop;
		}
		static constexpr bool isOrthogonal(int piece) {
			int pieceType = piece & pieceMask;
			return pieceType == queen || pieceType == rook;
		}
		static constexpr bool isDiagonal(int piece) {
			int pieceType = piece & pieceMask;
			return pieceType == queen || pieceType == bishop;
		}

		static int symbolToPiece(char symbol);
		static char pieceToSymbol(int piece);
	};

}

#endif