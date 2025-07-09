#include "MoveGen.h"
#include "MoveOrderer.h"

#include <iostream>
#include <limits>
#include <stdexcept>
#include <thread>

using namespace std;

namespace SandalBot {

	MoveGen::~MoveGen() {
		delete preComp;
	}

	MoveGen::MoveGen() {
		preComp = new MovePrecomputation();
	}

	MoveGen::MoveGen(Board* board) {
		if (board == nullptr) throw std::invalid_argument("board cannot be nullptr");
		this->board = board;
		preComp = new MovePrecomputation();
	}

	// Generates all legal moves on the board. Returns the number of legalmoves
	// and populates the decayed moves array (Must be minimum length of 218 - maximum possible moves).
	// capturesOnly determines whether only capture moves are considered
	int MoveGen::generateMoves(MovePoint moves[], bool capturesOnly) {
		initVariables(capturesOnly); // Setup variables for current board
		generateCheckData(); // Determine pins and check
		generateKingMoves(moves);

		// If king is checked twice, only legal moves is to move king
		if (doubleCheck)
			return currentMoves;

		generatePawnMoves(moves);
		generateKnightMoves(moves);
		generateOrthogonalMoves(moves);
		generateDiagonalMoves(moves);

		return currentMoves;
	}

	// Initialise variables for move generation
	void MoveGen::initVariables(bool capturesOnly) {
		squares = board->squares;
		isCheck = false;
		doubleCheck = false;
		whiteTurn = board->state->whiteTurn;
		enPassantSquare = board->state->enPassantSquare;
		castlingRights = board->state->castlingRights;
		fiftyMoveCounter = board->state->fiftyMoveCounter;
		moveCounter = board->state->moveCounter;

		colorIndex = whiteTurn ? Board::whiteIndex : Board::blackIndex;
		enemyColorIndex = !whiteTurn ? Board::whiteIndex : Board::blackIndex;
		currentColor = whiteTurn ? Piece::white : Piece::black;
		opposingColor = !whiteTurn ? Piece::white : Piece::black;
		currentMoves = 0ULL;

		friendlyKingSquare = board->pieceLists[colorIndex][Piece::king][0];
		enemyKingSquare = board->pieceLists[enemyColorIndex][Piece::king][0];

		checkBB = 0ULL;
		checkRayBB = 0ULL;
		opponentAttacks = 0ULL;

		friendlyBoard = whiteTurn ? board->whitePieces : board->blackPieces;
		enemyBoard = !whiteTurn ? board->whitePieces : board->blackPieces;

		generateCaptures = capturesOnly;
	}

	// Generate all possible moves for orthogonal moving pieces (rooks and queens).
	// Populates decayed moves array with new moves
	void MoveGen::generateOrthogonalMoves(MovePoint moves[]) {
		int startSquare;
		int targetSquare;
		bool isPinned;
		uint64_t orthSliders = board->orthogonalPieces & friendlyBoard; // All friendly orthogonal pieces

		// Iterate over friendly orthogonal pieces
		while (orthSliders != 0ULL) {
			// Get square of piece and remove it from bitboard
			startSquare = BitBoardUtility::popLSB(orthSliders);
			isPinned = checkRayBB & (1ULL << startSquare);

			// If pinned diagonally, cannot move orthogonally
			if (isPinned && abs(friendlyKingSquare - startSquare) % 7 == 0) {
				continue;
			} else if (isPinned && abs(friendlyKingSquare - startSquare) % 9 == 0) {
				continue;
			}
			// Generate movement board from magic bitboard hashtable
			uint64_t blockers = board->allPieces & preComp->getBlockerOrthogonalMask(startSquare);
			uint64_t moveBitboard = preComp->getOrthMovementBoard(startSquare, blockers);

			moveBitboard &= ~(friendlyBoard); // Disallow taking friendly pieces

			// If in check, only allow taking checking piece or blocking check
			if (isCheck) {
				moveBitboard &= checkBB;
			}

			// If pinned, only allow movement along pinned direction
			if (isPinned) {
				moveBitboard &= checkRayBB;
			}

			// If only captures, only consider moves that take enemy pieces
			if (generateCaptures) {
				moveBitboard &= enemyBoard;
			}

			// If pinned orthogonally, only allow movement in pinned direction
			if (isPinned && friendlyKingSquare / 8 == startSquare / 8) {
				moveBitboard &= preComp->getRowMask(startSquare);
			}
			if (isPinned && abs(friendlyKingSquare - startSquare) % 8 == 0) {
				moveBitboard &= preComp->getColMask(startSquare);
			}

			// For every available move, add to moves array
			while (moveBitboard != 0ULL) {
				targetSquare = BitBoardUtility::popLSB(moveBitboard);

				moves[currentMoves++].move = Move(startSquare, targetSquare);
			}

		}
	}
	// Generate all possible moves for diagonal moving pieces (bishops and queens).
	// Populates decayed moves array with new moves
	void MoveGen::generateDiagonalMoves(MovePoint moves[]) {
		int startSquare;
		int targetSquare;
		bool isPinned;
		uint64_t diagSliders = board->diagonalPieces & friendlyBoard; // All friendly diagonal pieces

		// Iterate over friendly diagonal pieces
		while (diagSliders != 0ULL) {
			// Get square of piece and remove it from bitboard
			startSquare = BitBoardUtility::popLSB(diagSliders);

			isPinned = checkRayBB & (1ULL << startSquare);

			// If pinned orthogonal, cannot move diagonally
			if (isPinned && friendlyKingSquare / 8 == startSquare / 8) {
				continue;
			} else if (isPinned && abs(friendlyKingSquare - startSquare) % 8 == 0) {
				continue;
			}
			// Generate movement board from magic bitboard hashtable
			uint64_t blockers = board->allPieces & preComp->getBlockerDiagonalMask(startSquare);
			uint64_t moveBitboard = preComp->getDiagMovementBoard(startSquare, blockers);

			moveBitboard &= ~(friendlyBoard); // Disallow taking friendly pieces
			// If in check, only allow taking checking piece or blocking check
			if (isCheck) {
				moveBitboard &= checkBB;
			}
			// If pinned, only allow movement along pinned direction
			if (isPinned) {
				moveBitboard &= checkRayBB;
			}
			// If only captures, only consider moves that take enemy pieces
			if (generateCaptures) {
				moveBitboard &= enemyBoard;
			}
			// If pinned diagonally, only allow movement in pinned direction
			if (isPinned && abs(friendlyKingSquare - startSquare) % 7 == 0) {
				moveBitboard &= preComp->getForwardMask(startSquare);
			} else if (isPinned && abs(friendlyKingSquare - startSquare) % 9 == 0) {
				moveBitboard &= preComp->getBackwardMask(startSquare);
			}
			// For every available move, add to moves array
			while (moveBitboard != 0ULL) {
				targetSquare = BitBoardUtility::popLSB(moveBitboard);

				moves[currentMoves++].move = Move(startSquare, targetSquare);
			}

		}
	}
	// Generate all possible moves for knights.
	// Populates decayed moves array with new moves
	void MoveGen::generateKnightMoves(MovePoint moves[]) {
		int startSquare;
		int targetSquare;
		bool isPinned;
		uint64_t knights = board->knights & friendlyBoard; // Get bitboard of all friendly knights
		// Iterate over all friendly knights
		while (knights != 0ULL) {
			// Get knight square
			startSquare = BitBoardUtility::popLSB(knights);
			isPinned = checkRayBB & (1ULL << startSquare);
			// If knight is pinned, impossible to move
			if (isPinned) continue;
			// Get movement board
			uint64_t moveBitboard = preComp->getKnightBoard(startSquare);
			moveBitboard &= ~(friendlyBoard); // Avoid capturing friendly pieces
			// If in check, attempt to capture checking piece or block check
			if (isCheck)
				moveBitboard &= checkBB;
			// If captures only, only consider capturing enemy pieces
			if (generateCaptures)
				moveBitboard &= enemyBoard;
			// Add all possible moves left to moves array
			while (moveBitboard != 0) {
				targetSquare = BitBoardUtility::popLSB(moveBitboard);

				moves[currentMoves++].move = Move(startSquare, targetSquare);
			}
		}
	}
	// Generate all possible moves for king.
	// Populates decayed moves array with new moves
	void MoveGen::generateKingMoves(MovePoint moves[]) {
		int targetSquare;
		const int castleMask = whiteTurn ? BoardState::whiteCastleMask : BoardState::blackCastleMask;
		const int colorIndex = whiteTurn ? board->whiteIndex : board->blackIndex;
		// If king can castle, generate moves
		if (!generateCaptures && friendlyKingSquare == startingKingSquares[colorIndex] && castlingRights & castleMask && !isCheck)
			castlingMoves(moves, friendlyKingSquare);
		// Get king movement board
		uint64_t moveBitboard = preComp->getKingMoves(friendlyKingSquare);
		moveBitboard &= ~(opponentAttacks); // Disallow moving into opponent checks
		moveBitboard &= ~(friendlyBoard); // Avoid capturing own pieces
		// If captures only, only allow capturing enemy pieces
		if (generateCaptures)
			moveBitboard &= enemyBoard;

		// Add all available moves
		while (moveBitboard != 0ULL) {
			targetSquare = BitBoardUtility::popLSB(moveBitboard);

			moves[currentMoves++].move = Move(friendlyKingSquare, targetSquare);
		}
	}
	// Generate all possible moves for pawns, including the many odd moves pawns can make.
	// Populates decayed moves array with new moves
	void MoveGen::generatePawnMoves(MovePoint moves[]) {
		int direction = whiteTurn ? whitePawnDirection : blackPawnDirection;
		const int startRow = whiteTurn ? 6 : 1;
		const int promotionRow = whiteTurn ? 1 : 6;
		bool isPinned;
		int startSquare;
		int targetSquare;
		int kingDir;

		uint64_t pawns = board->pawns & friendlyBoard;

		// Iterate over all friendly pawns
		while (pawns != 0ULL) {
			startSquare = BitBoardUtility::popLSB(pawns);
			kingDir = abs(friendlyKingSquare - startSquare); // Direction to king
			isPinned = checkRayBB & (1ULL << startSquare);

			// Cannot move if pinned horiztonally
			if (isPinned && friendlyKingSquare / 8 == startSquare / 8) continue;

			uint64_t moveBitboard = (1ULL << (startSquare + direction)) & ~board->allPieces; // Move forward one square
			// If nothing is blocking movement forward and on starting row, allow movement forward two squares 
			if (moveBitboard != 0ULL && startSquare / 8 == startRow) {
				moveBitboard |= (1ULL << (startSquare + 2 * direction)) & ~board->allPieces;
			}
			// If captures only, cannot move forward
			if (generateCaptures)
				moveBitboard = 0ULL;
			// If pinned, and not on same column (diagonally pinned)
			if (isPinned && friendlyKingSquare % 8 != startSquare % 8)
				moveBitboard = 0ULL;
			// Generate movement for diagonal attacks
			uint64_t attackBitboard = preComp->getPawnAttackMoves(startSquare, currentColor);
			// If pinned on file, cannot attack diagonally
			if (isPinned && friendlyKingSquare % 8 == startSquare % 8) {
				attackBitboard = 0ULL;
			} 
			// If pinned at a -45 degree angle, cannot move forward and only attack on that diagonal
			else if (isPinned && kingDir % 9 == 0) {
				attackBitboard &= preComp->getBackwardMask(startSquare);
				moveBitboard = 0ULL;
			}
			// "", but with 45 degree diagonal
			else if (isPinned && kingDir % 7 == 0) {
				attackBitboard &= preComp->getForwardMask(startSquare);
				moveBitboard = 0ULL;
			}
			// En Passant
			if (enPassantSquare != -1 && (attackBitboard & 1ULL << enPassantSquare)) {
				enPassantMoves(moves, enPassantSquare, startSquare);
			}
			// Combine attack moves and forward moves
			attackBitboard &= enemyBoard;
			moveBitboard |= attackBitboard;
			// If check, allow movement only to block or capture checking piece
			if (isCheck)
				moveBitboard &= checkBB;
			// Add moves to array
			while (moveBitboard != 0ULL) {
				targetSquare = BitBoardUtility::popLSB(moveBitboard);
				// Promotion moves
				if (startSquare / 8 == promotionRow) {
					for (int i = 0; i < 4; i++) {
						moves[currentMoves++].move = Move(startSquare, targetSquare, promotionFlags[i]);
					}
				} 
				// Regular pawn moves
				else if (abs(targetSquare - startSquare) < 10) {
					moves[currentMoves++].move = Move(startSquare, targetSquare);
				} 
				// Two squares forward
				else {
					moves[currentMoves++].move = Move(startSquare, targetSquare, Move::pawnTwoSquaresFlag);
				}
			}
		}
	}
	// Generate en passant move
	void MoveGen::enPassantMoves(MovePoint moves[], int targetSquare, int startSquare) {
		const int enemyPawnOffset = whiteTurn ? 8 : -8;
		const int enemyPawnSquare = targetSquare + enemyPawnOffset;
		// If in check, and own pawn does not block check and enemy pawn being taken isnt the checking piece, 
		// cannot en passant
		if (isCheck && !(checkBB & 1ULL << targetSquare) && !(checkBB & 1ULL << enemyPawnSquare)) return;
		// If rare edge case occurs, cannot en passant
		if (enPassantPin(startSquare, enemyPawnSquare)) return;

		moves[currentMoves++].move = Move(startSquare, targetSquare, Move::enPassantCaptureFlag);
	}
	// Checks rare edge case where orthogonal piece can pin through two pawns
	// and cause check upon en passant. Example case: 8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1 
	bool MoveGen::enPassantPin(int friendlyPawnSquare, int enemyPawnSquare) {
		// If king and pawn on different rows, no pin possible
		if (friendlyKingSquare / 8 != friendlyPawnSquare / 8) return false;

		// Create blocker board
		uint64_t blockers = preComp->getBlockerOrthogonalMask(friendlyKingSquare) & board->allPieces;
		// After en passant pawns will be gone
		blockers &= ~(1ULL << friendlyPawnSquare) & ~(1ULL << enemyPawnSquare);
		// Block column
		blockers |= preComp->getColMask(friendlyKingSquare) & ~(1ULL << friendlyKingSquare);
		blockers &= ~preComp->getRowMask(0);
		blockers &= ~preComp->getRowMask(63);
		// Get movement board
		uint64_t movementBitboard = preComp->getOrthMovementBoard(friendlyKingSquare, blockers);
		// Restrict movement to row and enemy orthogonal pieces
		movementBitboard &= board->orthogonalPieces & enemyBoard & preComp->getRowMask(friendlyKingSquare);
		// If movement board != 0ULL, there is a pin
		return movementBitboard != 0ULL;
	}
	// Generates castling moves for king
	void MoveGen::castlingMoves(MovePoint moves[], int startSquare) {
		// Castling rights
		const int shortMask = whiteTurn ? BoardState::whiteShortCastleMask : BoardState::blackShortCastleMask;
		const int longMask = whiteTurn ? BoardState::whiteLongCastleMask : BoardState::blackLongCastleMask;
		const int friendlyRook = whiteTurn ? Piece::whiteRook : Piece::blackRook;
		// Squares king 'travels' over
		const int travellingShortSquare = startSquare + 1;
		const int travellingLongSquare = startSquare - 1;
		// Checks if king 'travels' over square that is in check
		const bool shortAttacked = opponentAttacks & (1ULL << travellingShortSquare);
		const bool longAttacked = opponentAttacks & (1ULL << travellingLongSquare);
		// Short castling
		if (!(opponentAttacks & (1ULL << shortCastleKingSquares[colorIndex])) 
			&& !shortAttacked && castlingRights & shortMask && squares[shortCastleRookSquares[colorIndex]] == friendlyRook 
			&& squares[startSquare + 1] == Piece::empty && squares[startSquare + 2] == Piece::empty) {
			moves[currentMoves++].move = std::move(Move(startingKingSquares[colorIndex], shortCastleKingSquares[colorIndex], Move::castleFlag));
		}
		// Long castling
		if (!(opponentAttacks & (1ULL << longCastleKingSquares[colorIndex])) 
			&& !longAttacked && castlingRights & longMask && squares[longCastleRookSquares[colorIndex]] == friendlyRook 
			&& squares[startSquare - 1] == Piece::empty && squares[startSquare - 2] == Piece::empty 
			&& squares[startSquare - 3] == Piece::empty) {
			moves[currentMoves++].move = std::move(Move(startingKingSquares[colorIndex], longCastleKingSquares[colorIndex], Move::castleFlag));
		}
	}
	// Generate all squares 'attacked' by orthogonal pieces, return them in a bitboard
	uint64_t MoveGen::generateOrthogonalAttackData(const uint64_t orthogonalPieces, const uint64_t enemyBoard, int friendlyKingSquare) {
		uint64_t opponentAttacks = 0ULL;
		int startSquare;
		// Enemy orthogonal pieces
		uint64_t orthSliders = orthogonalPieces & enemyBoard;
		// Calculate movement for each piece and add to overall attack board
		while (orthSliders != 0ULL) {
			startSquare = BitBoardUtility::popLSB(orthSliders);

			uint64_t blockers = board->allPieces & preComp->getBlockerOrthogonalMask(startSquare);
			BitBoardUtility::deleteBit(blockers, friendlyKingSquare);
			uint64_t moveBitboard = preComp->getOrthMovementBoard(startSquare, blockers);

			opponentAttacks |= moveBitboard;
		}

		return opponentAttacks;
	}
	// Generate all squares 'attacked' by diagonal pieces, return them in a bitboard
	uint64_t MoveGen::generateDiagonalAttackData(const uint64_t diagonalPieces, const uint64_t enemyBoard, int friendlyKingSquare) {
		uint64_t opponentAttacks = 0ULL;
		int startSquare;
		// Enemy diagonal pieces
		uint64_t diagSliders = diagonalPieces & enemyBoard;
		// Calculate movement for each piece and add to overall attack board
		while (diagSliders != 0ULL) {
			startSquare = BitBoardUtility::popLSB(diagSliders);

			uint64_t blockers = board->allPieces & preComp->getBlockerDiagonalMask(startSquare);
			BitBoardUtility::deleteBit(blockers, friendlyKingSquare);
			uint64_t moveBitboard = preComp->getDiagMovementBoard(startSquare, blockers);

			opponentAttacks |= moveBitboard;
		}

		return opponentAttacks;
	}
	// Generate all squares 'attacked' by knights, return them in a bitboard
	uint64_t MoveGen::generateKnightAttackData(const uint64_t enemyBoard) {
		uint64_t opponentAttacks = 0ULL;
		int startSquare;
		// Enemy knights
		uint64_t knights = board->knights & enemyBoard;
		// Add movement to overall attack board
		while (knights != 0ULL) {
			startSquare = BitBoardUtility::popLSB(knights);
			opponentAttacks |= preComp->getKnightBoard(startSquare);
		}

		return opponentAttacks;
	}
	// Generate all squares 'attacked' by pawns, return them in a bitboard
	uint64_t MoveGen::generatePawnAttackData(const uint64_t enemyBoard, const int opposingColor) {
		uint64_t opponentAttacks = 0ULL;
		int startSquare;
		// Enemy pawns
		uint64_t pawns = board->pawns & enemyBoard;
		// Add only attack movement to overall attack board
		while (pawns != 0ULL) {
			startSquare = BitBoardUtility::popLSB(pawns);
			uint64_t moveBitboard = preComp->getPawnAttackMoves(startSquare, opposingColor);
			opponentAttacks |= moveBitboard;
		}

		return opponentAttacks;
	}
	// Generate all squares 'attacked' by king, return them in a bitboard
	uint64_t MoveGen::generateKingAttackData(const int enemyKingSquare) {
		return preComp->getKingMoves(enemyKingSquare);
	}
	// Generate attack movement from opponent for all pieces
	void MoveGen::generateAttackData() {
		opponentAttacks |= generateOrthogonalAttackData(board->orthogonalPieces, enemyBoard, friendlyKingSquare);
		opponentAttacks |= generateDiagonalAttackData(board->diagonalPieces, enemyBoard, friendlyKingSquare);
		opponentAttacks |= generateKingAttackData(enemyKingSquare);
		opponentAttacks |= generateKnightAttackData(enemyBoard);
		opponentAttacks |= generatePawnAttackData(enemyBoard, opposingColor);
	}
	// Calculate check and pin data to calculate where king can move
	void MoveGen::generateCheckData() {
		generateAttackData();

		int targetSquare;
		uint64_t temp;
		uint64_t enemyBlockers;
		// Get pieces 'blocking' kings orthogonal and diagonal view
		uint64_t orthogonalBlockers = board->allPieces & preComp->getBlockerOrthogonalMask(friendlyKingSquare);
		uint64_t diagonalBlockers = board->allPieces & preComp->getBlockerDiagonalMask(friendlyKingSquare);
		// Get kings orthogonal 'vision'
		temp = preComp->getOrthMovementBoard(friendlyKingSquare, orthogonalBlockers);
		uint64_t checkBoard = temp;
		enemyBlockers = temp & enemyBoard & board->orthogonalPieces; // See if enemy orthogonal pieces attack the king
		// Get kings diagonal 'vision'
		temp = preComp->getDiagMovementBoard(friendlyKingSquare, diagonalBlockers);
		checkBoard |= temp;
		enemyBlockers |= temp & enemyBoard & board->diagonalPieces; // See if enemy diagonal pieces attack king

		// For each diagonal/orthogonal piece that attacks the king
		while (enemyBlockers != 0ULL) {
			// Get mask with line between king and enemy piece
			targetSquare = BitBoardUtility::popLSB(enemyBlockers);
			temp = preComp->getDirectionMask(friendlyKingSquare, targetSquare);
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
		temp = preComp->getOrthMovementBoard(friendlyKingSquare, orthogonalBlockers);
		checkBoard = temp;
		enemyBlockers = temp & enemyBoard & board->orthogonalPieces;
		temp = preComp->getDiagMovementBoard(friendlyKingSquare, diagonalBlockers);
		checkBoard |= temp;
		enemyBlockers |= temp & enemyBoard & board->diagonalPieces;
		// For every enemy piece pinning friendly pieces
		while (enemyBlockers != 0ULL) {
			targetSquare = BitBoardUtility::popLSB(enemyBlockers);
			temp = preComp->getDirectionMask(friendlyKingSquare, targetSquare);
			checkRayBB |= temp & checkBoard; // Add 'ray' between king and enemy piece to rays
		}

		// See if any knights check the king
		uint64_t enemyKnights = board->knights & enemyBoard;
		uint64_t knightMoveBitboard = preComp->getKnightBoard(friendlyKingSquare);
		knightMoveBitboard &= enemyKnights;

		checkBB |= knightMoveBitboard;
		// Update check status if king can see any knights
		while (knightMoveBitboard != 0) {
			BitBoardUtility::popLSB(knightMoveBitboard);
			doubleCheck = isCheck;
			isCheck = true;
			if (doubleCheck)
				break;
		}
		
		// See if any pawns can check the king
		uint64_t enemyPawns = board->pawns & enemyBoard;
		uint64_t pawnMoveBitboard = preComp->getPawnAttackMoves(friendlyKingSquare, currentColor);
		pawnMoveBitboard &= enemyPawns;

		checkBB |= pawnMoveBitboard;
		// Update check status if king is attacked by any pawns
		while (pawnMoveBitboard != 0) {
			BitBoardUtility::popLSB(pawnMoveBitboard);
			doubleCheck = isCheck;
			isCheck = true;
			if (doubleCheck)
				break;
		}
	}

}