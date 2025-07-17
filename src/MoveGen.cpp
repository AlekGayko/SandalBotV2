#include "MoveGen.h"
#include "MoveOrderer.h"

#include <iostream>
#include <limits>
#include <stdexcept>
#include <thread>

using namespace std;

namespace SandalBot {

	// Generates all legal moves on the board. Returns the number of legalmoves
	// and populates the decayed moves array (Must be minimum length of 218 - maximum possible moves).
	// capturesOnly determines whether only capture moves are considered
	int MoveGen::generate(MovePoint moves[], bool capturesOnly) {
		board->sideToMove() == WHITE 
			? generateAllMoves<WHITE>(moves, capturesOnly) : generateAllMoves<BLACK>(moves, capturesOnly);

		return currentMoves;
	}

	template <Color Us>
	int MoveGen::generateAllMoves(MovePoint moves[], bool capturesOnly) {
		initVariables(); // Setup variables for current board
		generateCheckData<Us>(); // Determine pins and check

		generateKingMoves<Us>(moves, capturesOnly);

		// If king is checked twice, only legal moves is to move king
		if (doubleCheck)
			return currentMoves;

		generatePawnMoves<Us>(moves, capturesOnly);
		generateMoves<Us, QUEEN>(moves, capturesOnly);
		generateMoves<Us, KNIGHT>(moves, capturesOnly);
		generateMoves<Us, BISHOP>(moves, capturesOnly);
		generateMoves<Us, ROOK>(moves, capturesOnly);

		return currentMoves;
	}

	// Initialise variables for move generation
	void MoveGen::initVariables() {
		isCheck = false;
		doubleCheck = false;

		currentMoves = 0ULL;

		checkBB = 0ULL;
		checkRayBB = 0ULL;
		opponentAttacks = 0ULL;
	}

	template<Color Us, PieceType Type>
	void MoveGen::generateMoves(MovePoint moves[], bool capturesOnly) {
		Bitboard pieces = board->typesBB[Type] & board->colorsBB[Us];
	
		while (pieces != 0ULL) {
			Square from = popLSB(pieces);
			Bitboard movementBB = getMovementBoard<Type>(from, board->typesBB[ALL_PIECES]);
			movementBB &= ~board->colorsBB[Us];
			
			bool pinned = checkRayBB && (checkRayBB & (1ULL << from));

			if (pinned) {
				Bitboard pinLine = getLineBB(from, board->kingSquares[Us]);
				movementBB &= pinLine;
			}

			if (isCheck) {
				movementBB &= checkBB;
			}

			if (capturesOnly) {
				movementBB &= board->colorsBB[~Us];
			}

			while (movementBB != 0ULL) {
				Square to = popLSB(movementBB);
				addMove(moves, from, to);
			}
		}
	}

	// Generate all possible moves for pawns, including the many odd moves pawns can make.
	// Populates decayed moves array with new moves
	template <Color Us>
	void MoveGen::generatePawnMoves(MovePoint moves[], bool capturesOnly) {
		constexpr Direction pawnUp = pawnPush(Us);
		constexpr Row startRow = Us == WHITE ? ROW_2 : ROW_7;
		constexpr Row twoSquaresRow = Us == WHITE ? ROW_4 : ROW_5;
		constexpr Row promoteRow = Us == WHITE ? ROW_7 : ROW_2;

		Bitboard pawns = board->typesBB[PAWN] & board->colorsBB[Us];
		Bitboard allPieces = board->typesBB[ALL_PIECES];
		Bitboard emptySquares = ~allPieces;
		bool epAvailable = board->state->enPassantSquare != NONE_SQUARE;

		while (pawns != 0ULL) {
			Square from = popLSB(pawns);
			Bitboard pushBB = capturesOnly ? 0ULL : ((1ULL << (from + pawnUp)) & emptySquares);

			if (pushBB != 0ULL && toRow(from) == startRow && ((1ULL << (from + 2 * pawnUp)) & emptySquares) != 0ULL) {
				pushBB |= 1ULL << (from + 2 * pawnUp);
			}

			Bitboard attackBB = getPawnAttackMoves<Us>(from) & allPieces;

			Bitboard movementBB = pushBB | attackBB;

			movementBB &= ~board->colorsBB[Us];

			bool isPinned = checkRayBB && (checkRayBB & (1ULL << from));

			if (isPinned) {
				Bitboard pinLine = getLineBB(from, board->kingSquares[Us]);
				movementBB &= pinLine;
			}

			if (isCheck) {
				movementBB &= checkBB;
			}

			if (toRow(from) == promoteRow && movementBB != 0ULL) {
				promotionMoves<Us>(moves, from, movementBB);
			}
	
			while (movementBB != 0ULL) {
				Square to = popLSB(movementBB);

				if (toRow(from) == startRow && toRow(to) == twoSquaresRow) {
					addMove(moves, from, to, Move::Flag::PAWN_TWO_SQUARES);
				} else {
					addMove(moves, from, to);
				}
			}
			
			Bitboard epBB = getPawnAttackMoves<Us>(from) & (1ULL << board->state->enPassantSquare);

			if (epBB != 0ULL && epAvailable) {
				enPassantMoves<Us>(moves, from, board->state->enPassantSquare, isPinned);
			} 
		}
	}

	// Generate en passant move
	template <Color Us>
	void MoveGen::enPassantMoves(MovePoint moves[], Square from, Square to, bool isPinned) {
		Square enemyPawnSquare = to - pawnPush(Us);
		// If in check, and own pawn does not block check and enemy pawn being taken isnt the checking piece, 
		// cannot en passant
		if (isCheck && !(checkBB & (1ULL << to)) && !(checkBB & (1ULL << enemyPawnSquare)))
			return;

		if (isPinned) {
			Bitboard movementBB = 1ULL << to;
			Bitboard pinLine = getLineBB(from, board->kingSquares[Us]);
			movementBB &= pinLine;

			if (movementBB == 0ULL) {
				return;
			}
		}

		// If rare edge case occurs, cannot en passant
		if (enPassantPin<Us>(from, enemyPawnSquare))
			return;

		addMove(moves, from, to, Move::Flag::EN_PASSANT);
	}

	// Checks rare edge case where orthogonal piece can pin through two pawns
	// and cause check upon en passant. Example case: 8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1 
	template <Color Us>
	bool MoveGen::enPassantPin(Square friendlyPawnSquare, Square enemyPawnSquare) {
		Square kSq = board->kingSquares[Us];
		// If king and pawn on different rows, no pin possible
		if (toRow(kSq) != toRow(friendlyPawnSquare))
			return false;

		// Create blocker board
		Bitboard blockers = getBlockerOrthogonalMask(kSq) & board->typesBB[ALL_PIECES];
		// After en passant pawns will be gone
		blockers &= ~(1ULL << friendlyPawnSquare) & ~(1ULL << enemyPawnSquare);
		// Block column
		blockers |= getColMask(kSq) & ~(1ULL << kSq);
		blockers &= ~getRowMask(Square(ROW_8));
		blockers &= ~getRowMask(Square(ROW_1));
		// Get movement board
		Bitboard movementBitboard = getOrthMovementBoard(kSq, blockers);
		// Restrict movement to row and enemy orthogonal pieces
		movementBitboard &= (board->typesBB[ROOK] | board->typesBB[QUEEN]) & board->colorsBB[~Us];
		// If movement board != 0ULL, there is a pin
		return movementBitboard != 0ULL;
	}

	template <Color Us>
	void MoveGen::promotionMoves(MovePoint moves[], Square from, Bitboard movementBB) {
		while (movementBB != 0ULL) {
			Square to = popLSB(movementBB);

			addMove(moves, from, to, Move::Flag::QUEEN);
			addMove(moves, from, to, Move::Flag::ROOK);
			addMove(moves, from, to, Move::Flag::BISHOP);
			addMove(moves, from, to, Move::Flag::KNIGHT);
		}
	}

	// Generate all possible moves for king.
	// Populates decayed moves array with new moves
	template <Color Us>
	void MoveGen::generateKingMoves(MovePoint moves[], bool capturesOnly) {
		constexpr CastlingRights crMask = (Us == WHITE ? W_RIGHTS : B_RIGHTS);
		Square from = board->kingSquares[Us];

		// Get king movement board
		Bitboard moveBitboard = getMovementBoard<KING>(from, 0ULL);
		moveBitboard &= ~(opponentAttacks); // Disallow moving into opponent checks
		moveBitboard &= ~(board->colorsBB[Us]); // Avoid capturing own pieces

		// If captures only, only allow capturing enemy pieces
		if (capturesOnly)
			moveBitboard &= board->colorsBB[~Us];

		// Add all available moves
		while (moveBitboard != 0ULL) {
			Square to = popLSB(moveBitboard);

			addMove(moves, from, to);
		}

		// If king can castle, generate moves
		if (!capturesOnly && !isCheck && (crMask & board->state->cr) != NO_RIGHTS)
			castlingMoves<Us>(moves, from);
	}

	template <Color Us>
	// Generates castling moves for king
	void MoveGen::castlingMoves(MovePoint moves[], Square from) {
		if (canShortCastle(Us, board->state->cr)) {
			if (((shortCastleCheckSQ[Us] & opponentAttacks) == 0ULL) && ((emptyShortCastleSQ[Us] & board->typesBB[ALL_PIECES]) == 0ULL)) {
				addMove(moves, from, Square(from + 2 * EAST), Move::Flag::CASTLE);
			}
		}

		if (canLongCastle(Us, board->state->cr)) {
			if (((longCastleCheckSQ[Us] & opponentAttacks) == 0ULL) && ((emptyLongCastleSQ[Us] & board->typesBB[ALL_PIECES]) == 0ULL)) {
				addMove(moves, from, Square(from + 2 * WEST), Move::Flag::CASTLE);
			}
		}
	}

	template <Color Us>
	Bitboard MoveGen::generatePawnAttackData() {
		Bitboard attackBB = 0ULL;

		Bitboard pawns = board->typesBB[PAWN] & board->colorsBB[~Us];

		// Add only attack movement to overall attack board
		while (pawns != 0ULL) {
			Square from = popLSB(pawns);
			Bitboard movementBB = getPawnAttackMoves<~Us>(from);
			attackBB |= movementBB;
		}

		return attackBB;
	}

	template <Color Us, PieceType Type>
	Bitboard MoveGen::generateAttackData() {
		Bitboard attackBB = 0ULL;

		Bitboard pieces = board->typesBB[Type] & board->colorsBB[~Us];
		Bitboard blockers = board->typesBB[ALL_PIECES] & ~(1ULL << board->kingSquares[Us]);

		// Add movement to overall attack board
		while (pieces != 0ULL) {
			Square from = popLSB(pieces);
			
			Bitboard movementBB = getMovementBoard<Type>(from, blockers);

			attackBB |= movementBB;
		}

		return attackBB;
	}

	template <Color Us>
	// Generate attack movement from opponent for all pieces
	void MoveGen::generateAllAttackData() {
		opponentAttacks |= getMovementBoard<KING>(board->kingSquares[~Us], 0ULL);
		opponentAttacks |= generateAttackData<Us, QUEEN>();
		opponentAttacks |= generateAttackData<Us, ROOK>();
		opponentAttacks |= generateAttackData<Us, BISHOP>();
		opponentAttacks |= generateAttackData<Us, KNIGHT>();
		opponentAttacks |= generatePawnAttackData<Us>();
	}

	template <Color Us>
	// Calculate check and pin data to calculate where king can move
	void MoveGen::generateCheckData() {
		generateAllAttackData<Us>();

		Square kSq = board->kingSquares[Us];
		Bitboard friendlyBoard = board->colorsBB[Us];
		Bitboard enemyBoard = board->colorsBB[~Us];
		Bitboard temp;
		Bitboard enemyBlockers;
		// Get pieces 'blocking' kings orthogonal and diagonal view
		Bitboard orthogonalBlockers = board->typesBB[ALL_PIECES] & getBlockerOrthogonalMask(kSq);
		Bitboard diagonalBlockers = board->typesBB[ALL_PIECES] & getBlockerDiagonalMask(kSq);
		// Get kings orthogonal 'vision'
		temp = getOrthMovementBoard(kSq, orthogonalBlockers);
		Bitboard checkBoard = temp;
		enemyBlockers = temp & enemyBoard & (board->typesBB[ROOK] | board->typesBB[QUEEN]); // See if enemy orthogonal pieces attack the king
		// Get kings diagonal 'vision'
		temp = getDiagMovementBoard(kSq, diagonalBlockers);
		checkBoard |= temp;
		enemyBlockers |= temp & enemyBoard & (board->typesBB[BISHOP] | board->typesBB[QUEEN]); // See if enemy diagonal pieces attack king

		// For each diagonal/orthogonal piece that attacks the king
		while (enemyBlockers != 0ULL) {
			// Get mask with line between king and enemy piece
			Square to = popLSB(enemyBlockers);
			temp = getLineBetweenBB(kSq, to);
			checkBB |= temp & checkBoard; // Add line between king and enemy to check bitboard
			// Update check and double check
			doubleCheck = isCheck;
			isCheck = true;
			if (doubleCheck)
				break;
		}
		// Now that check lines have been done, now calculate 'rays' that pin friendly pieces

		// Take out first 'sight' of any friendly pieces, to only see second wave pieces which
		// may or may not pin those friendly pieces from behind
		orthogonalBlockers &= ~(checkBoard & friendlyBoard);
		diagonalBlockers &= ~(checkBoard & friendlyBoard);

		// Same logic as check but for pieces behind first wave friendly pieces
		temp = getOrthMovementBoard(kSq, orthogonalBlockers);
		checkBoard = temp;
		enemyBlockers = temp & enemyBoard & (board->typesBB[ROOK] | board->typesBB[QUEEN]);
		temp = getDiagMovementBoard(kSq, diagonalBlockers);
		checkBoard |= temp;
		enemyBlockers |= temp & enemyBoard & (board->typesBB[BISHOP] | board->typesBB[QUEEN]);
		// For every enemy piece pinning friendly pieces
		while (enemyBlockers != 0ULL) {
			Square to = popLSB(enemyBlockers);
			temp = getLineBetweenBB(kSq, to);
			checkRayBB |= temp & checkBoard; // Add 'ray' between king and enemy piece to rays
		}

		// See if any knights check the king
		Bitboard enemyKnights = board->typesBB[KNIGHT] & enemyBoard;
		Bitboard knightMoveBitboard = getMovementBoard<KNIGHT>(kSq, 0ULL);
		knightMoveBitboard &= enemyKnights;

		checkBB |= knightMoveBitboard;
		// Update check status if king can see any knights
		while (knightMoveBitboard != 0ULL) {
			popLSB(knightMoveBitboard);
			doubleCheck = isCheck;
			isCheck = true;
			if (doubleCheck)
				break;
		}
		
		// See if any pawns can check the king
		Bitboard enemyPawns = board->typesBB[PAWN] & enemyBoard;
		Bitboard pawnMoveBitboard = getPawnAttackMoves<Us>(kSq);
		pawnMoveBitboard &= enemyPawns;

		checkBB |= pawnMoveBitboard;
		// Update check status if king is attacked by any pawns
		while (pawnMoveBitboard != 0ULL) {
			popLSB(pawnMoveBitboard);
			doubleCheck = isCheck;
			isCheck = true;
			if (doubleCheck)
				break;
		}
	}

}