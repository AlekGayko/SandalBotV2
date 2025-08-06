#include "MoveGen.h"
#include "MoveOrderer.h"
#include "PieceEvaluations.h"

#include <iostream>
#include <limits>
#include <stdexcept>
#include <thread>

using namespace std;

namespace SandalBot {

	constexpr PointValue checkBonus{ 300 };
	constexpr PointValue undefendedtoBonus{ -200 };
	constexpr PointValue enPassantBonus{ 300 };
	constexpr PointValue castleBonus{ 300 };
	constexpr PointValue pawnTwoSquareBonus{ 100 };
	constexpr PointValue queenPromotionBonus{ 600 };
	constexpr PointValue rookPromotionBonus{ 400 };
	constexpr PointValue bishopPromotionBonus{ 300 };
	constexpr PointValue knightPromotionBonus{ 300 };

	constexpr PointValue queenSafeBonus{ 500 };
	constexpr PointValue rookSafeBonus{ 250 };
	constexpr PointValue safeBonus{ 175 };

	template <MoveGen::GenType Type>
	void MoveGen::evaluateMoves() {
		auto start{ curr };
		auto end{ endMoves };

		Bitboard attackedAllies, attackedByPawn, attackedByMinor, attackedByRook;
		if constexpr (Type == QUIETS) {
			Color them = board->sideToMove();

			attackedByPawn = board->attacksBB(PAWN, them);
			attackedByMinor = board->attacksBB(KNIGHT, them) | board->attacksBB(BISHOP, them) | attackedByPawn;
			attackedByRook = board->attacksBB(ROOK, them) | attackedByMinor;

			// Own pieces attacked by pieces with lesser value
			attackedAllies = (board->pieces(~them, QUEEN) & attackedByRook)
				| (board->pieces(~them, ROOK) & attackedByMinor)
				| (board->pieces(KNIGHT, BISHOP) & board->sidePieces(~them) & attackedByPawn);
		}

		// For each move
		for (auto it{ start }; it != end; ++it) {
			const Square from = it->move.from();
			const Square to = it->move.to();
			const Move::Flag flag = it->move.flag();
			PieceType ownPiece = typeOf(board->squares[from]);
			PieceType enemyPiece = typeOf(board->squares[to]);

			assert(ownPiece != NO_PIECE_TYPE);

			// If piece make direct check, prioritise it
			if (board->checkSquares(ownPiece) & (1ULL << to)) {
				it->value += checkBonus;
			}

			// Add difference in piece positioning
			if (board->sideToMove() == BLACK) {
				it->value += PieceEvaluations::sqEvals[ownPiece][flipRow(to)];
				it->value -= PieceEvaluations::sqEvals[ownPiece][flipRow(from)];
			} else {
				it->value += PieceEvaluations::sqEvals[ownPiece][to];
				it->value -= PieceEvaluations::sqEvals[ownPiece][from];
			}

			if constexpr (Type == CAPTURES) {
				it->value += PieceEvaluations::pieceVals[enemyPiece];
			} else if constexpr (Type == QUIETS) {
				it->value += (attackedAllies & (1ULL << from) ?
					  (ownPiece == QUEEN && ((1ULL << to) & attackedByRook) == 0ULL ? queenSafeBonus : 
					  (ownPiece == ROOK && ((1ULL << to) & attackedByMinor) == 0ULL) ? rookSafeBonus :
					  ((1ULL << to) & attackedByMinor) == 0ULL ? safeBonus :	
					  0) : 0);
			} else if constexpr (Type == EVASIONS) {
				if (enemyPiece != NO_PIECE) {
					it->value += PieceEvaluations::pieceVals[enemyPiece] - PieceEvaluations::pieceVals[ownPiece];
				}
			}			

			// Moves with flags are most likely special (good)
			switch (flag) {
			case Move::Flag::EN_PASSANT:
				it->value += enPassantBonus;
				break;
			case Move::Flag::CASTLE:
				it->value += castleBonus;
				break;
			case Move::Flag::QUEEN:
				it->value += queenPromotionBonus;
				break;
			case Move::Flag::ROOK:
				it->value += rookPromotionBonus;
				break;
			case Move::Flag::BISHOP:
				it->value += bishopPromotionBonus;
				break;
			case Move::Flag::KNIGHT:
				it->value += knightPromotionBonus;
				break;
			}
		}
	}

	template<MoveGen::GenType MoveType>
	void MoveGen::generate() {
		board->sideToMove() == WHITE 
			? generateAllMoves<MoveType, WHITE>() : generateAllMoves<MoveType, BLACK>();
	}

	template <MoveGen::GenType MoveType, Color Us>
	void MoveGen::generateAllMoves() {
		constexpr bool checking = MoveType == QUIET_CHECKS;
		const Square kSq = board->kingSquares[Us];
		Bitboard target = MoveType == CAPTURES ? board->sidePieces(~Us)
									: MoveType == ALL ? ~board->sidePieces(Us)
									: MoveType == EVASIONS ? (board->checkBB() | getLineBetweenBB(kSq, LSB(board->state->checkBB))) & ~board->sidePieces(Us)
									: ~board->pieces(); // Quiets and quiet checks	

		// If king is checked twice, only legal moves are king moves
		if (MoveType != EVASIONS || !moreThanOne(board->checkBB())) {
			generatePawnMoves<MoveType, Us>(target);
			generateMoves<Us, KNIGHT, checking>(target);
			generateMoves<Us, BISHOP, checking>(target);
			generateMoves<Us, ROOK, checking>(target);
			generateMoves<Us, QUEEN, checking>(target);
		}

		if (!checking || (board->kingBlockersBB(~Us) & kSq)) { // If not quiet checks, unless the king is blocking a discovered attack
			target = MoveType == EVASIONS ? ~board->sidePieces(Us) : target;
			generateKingMoves<MoveType, Us, checking>(target);
		}
	}

	Move MoveGen::selectMove() {
		if (curr >= endMoves) {
			return Move();
		} else {
			return (*(curr++)).move;
		}
	}

	Move MoveGen::getMove() {
		Move move;
	start:
		switch (stage) {
		case MAIN_TT:
		case Q_TT:
		case EVASIONS_TT:
			stage++;
			if (!ttMove.isNull()) {
				return ttMove;
			} else {
				goto start;
			}
		case Q_CAPTURES_INIT:
		case MAIN_CAPTURE_INIT:
			generate<CAPTURES>();
			evaluateMoves<CAPTURES>();
			MoveOrder::order(this);
			stage++;
			goto start;
		case MAIN_CAPTURES:
		case Q_CAPTURES:
			if (!(move = selectMove()).isNull()) {
				return move;
			} else {
				stage++;
				goto start;
			}
		case MAIN_QUIET_INIT:
			generate<QUIETS>();
			evaluateMoves<QUIETS>();
			MoveOrder::order(this);
			stage++;
			goto start;
		case MAIN_QUIETS:
			if (!(move = selectMove()).isNull())
				return move;
			else
				stage++;
			goto start;
		case Q_CHECKS_INIT:
			generate<QUIET_CHECKS>();
			stage++;
			goto start;
		case Q_CHECKS:
			return selectMove();
		case EVASIONS_INIT:
			generate<EVASIONS>();
			evaluateMoves<EVASIONS>();
			MoveOrder::order(this);
			stage++;
			goto start;
		case EVASION_MOVES:
			return selectMove();
		case VANILLA_INIT:
			generate<ALL>();
			stage++;
			goto start;
		case VANILLA:
			return selectMove();
		}
		
		return move;
	}

	template<Color Us, PieceType Type, bool QuietChecking>
	void MoveGen::generateMoves(Bitboard target) {
		Bitboard pieces = board->typesBB[Type] & board->colorsBB[Us];

		while (pieces != 0ULL) {
			Square from = popLSB(pieces);
			Bitboard movementBB = getMovementBoard<Type>(from, board->typesBB[ALL_PIECES]) & target;

			// If quiet checking, restrict movement to check squares for piece. If piece is a blocker, 
			// moving anywhere will cause a discover check so it is not restricted.
			if (QuietChecking && (Type == QUEEN || !(board->kingBlockersBB(~Us) & (1ULL << from)))) {
				movementBB &= board->checkSquares(Type);
			}

			while (movementBB != 0ULL) {
				Square to = popLSB(movementBB);
				addMove(from, to);
			}
		}
	}

	// Generate all possible moves for pawns, including the many odd moves pawns can make.
	// Populates decayed moves array with new moves
	template <MoveGen::GenType Type, Color Us>
	void MoveGen::generatePawnMoves(Bitboard target) {
		constexpr Direction up = pawnPush(Us);
		constexpr Direction upLeft = Us == WHITE ? NORTH_EAST : SOUTH_WEST;
		constexpr Direction upRight = Us == WHITE ? NORTH_WEST : SOUTH_EAST;

		constexpr Row startRow = Us == WHITE ? ROW_2 : ROW_7;
		constexpr Row twoSquaresRow = Us == WHITE ? ROW_4 : ROW_5;
		constexpr Bitboard rank3 = Us == WHITE ? getRowMask(ROW_3) : getRowMask(ROW_6);
		constexpr Bitboard promoteRow = Us == WHITE ? getRowMask(ROW_7) : getRowMask(ROW_2);

		Bitboard promotingPawns = board->pieces(Us, PAWN) & promoteRow;
		Bitboard normalPawns = board->pieces(Us, PAWN) & ~promoteRow;

		Bitboard emptySquares = ~board->pieces();
		Bitboard them = Type == EVASIONS ? board->checkBB() : board->sidePieces(~Us);
		bool epAvailable = board->state->enPassantSquare != NONE_SQUARE;

		if constexpr (Type != CAPTURES) {
			Bitboard singlePushes = shift<up>(normalPawns) & emptySquares;
			Bitboard doublePushes = shift<up>(singlePushes & rank3) & emptySquares;
			
			if constexpr (Type == EVASIONS) {
				singlePushes &= target;
				doublePushes &= target;
			} else if constexpr (Type == QUIET_CHECKS) {
				Square kSq = board->kingSquares[~Us];
				Bitboard discoverPawnsMask = board->kingBlockersBB(~Us) & getColMask(kSq);
				singlePushes &= shift<up>(discoverPawnsMask) | getPawnAttackMoves(kSq, ~Us);
				doublePushes &= shift<up + up>(discoverPawnsMask) | getPawnAttackMoves(kSq, ~Us);
			}

			while (singlePushes != 0ULL) {
				Square to = popLSB(singlePushes);
				addMove(to - up, to);
			}

			while (doublePushes != 0ULL) {
				Square to = popLSB(doublePushes);
				addMove(to - up - up, to, Move::Flag::PAWN_TWO_SQUARES);
			}
		}

		if (promotingPawns) {
			Bitboard attackLeft = shift<upLeft>(promotingPawns) & them;
			Bitboard attackRight = shift<upRight>(promotingPawns) & them;
			Bitboard pushBB = shift<up>(promotingPawns) & emptySquares;

			if constexpr (Type == EVASIONS) {
				pushBB &= target;
			}

			while (attackLeft != 0ULL) {
				Square to = popLSB(attackLeft);
				promotionMoves(to - upLeft, to);
			}

			while (attackRight != 0ULL) {
				Square to = popLSB(attackRight);
				promotionMoves(to - upRight, to);
			}

			while (pushBB != 0ULL) {
				Square to = popLSB(pushBB);
				promotionMoves(to - up, to);
			}
		}

		if constexpr (Type == CAPTURES || Type == EVASIONS || Type == ALL) {
			Bitboard attackLeft = shift<upLeft>(normalPawns) & them;
			Bitboard attackRight = shift<upRight>(normalPawns) & them;

			while (attackLeft != 0ULL) {
				Square to = popLSB(attackLeft);
				addMove(to - upLeft, to);
			}

			while (attackRight != 0ULL) {
				Square to = popLSB(attackRight);
				addMove(to - upRight, to);
			}

			if (epAvailable) {
				Bitboard epPawns = normalPawns & getPawnAttackMoves(board->state->enPassantSquare, ~Us);

				// If pawn push caused a discovered check, en passant is futile
				if (Type == EVASIONS && (target & (1ULL << (board->state->enPassantSquare + up)))) {
					return;
				}

				while (epPawns != 0ULL) {
					Square from = popLSB(epPawns);
					addMove(from, board->state->enPassantSquare, Move::Flag::EN_PASSANT);
				}
			}
		}
	}

	void MoveGen::promotionMoves(Square from, Square to) {
		addMove(from, to, Move::Flag::QUEEN);
		addMove(from, to, Move::Flag::ROOK);
		addMove(from, to, Move::Flag::BISHOP);
		addMove(from, to, Move::Flag::KNIGHT);
	}

	// Generate all possible moves for king.
	// Populates decayed moves array with new moves
	template <MoveGen::GenType Type, Color Us, bool QuietChecking>
	void MoveGen::generateKingMoves(Bitboard target) {
		constexpr CastlingRights crMask = (Us == WHITE ? W_RIGHTS : B_RIGHTS);
		Square from = board->kingSquares[Us];

		// Get king movement board
		Bitboard moveBitboard = getMovementBoard<KING>(from, 0ULL) & target;

		// If quiet checking, limit movement to anywhere but line of sight to other king.
		// If condition outside function demands kin must be blocking attack if checking.
		if constexpr (QuietChecking) { 
			moveBitboard &= getMovementBoard<QUEEN>(board->kingSquares[~Us], 0ULL);
		}

		// Add all available moves
		while (moveBitboard != 0ULL) {
			Square to = popLSB(moveBitboard);
			addMove(from, to);
		}

		// If king can castle, generate moves
		if ((Type == QUIETS || Type == ALL) && board->checkBB() == 0ULL && (crMask & board->state->cr) != NO_RIGHTS) {
			if (canShortCastle(Us, board->state->cr) && (emptyShortCastleSQ[Us] & board->typesBB[ALL_PIECES]) == 0ULL) {
				addMove(from, from + EAST + EAST, Move::Flag::CASTLE);
			}

			if (canLongCastle(Us, board->state->cr) && (emptyLongCastleSQ[Us] & board->typesBB[ALL_PIECES]) == 0ULL) {
				addMove(from, from + WEST + WEST, Move::Flag::CASTLE);
			}
		}
	}

}