#include "Bitboards.h"
#include "Board.h"
#include "BoardState.h"
#include "Evaluator.h"
#include "FEN.h"
#include "PieceEvaluations.h"
#include "Types.h"
#include "ZobristHash.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;

namespace SandalBot {

	Board::Board() {
		loadPosition(FEN::startpos);
	}

	// Parses a given FEN string and initialises position accordingly
	void Board::loadPosition(std::string_view fen) {
		PositionInfo newPos { FEN::fenToPosition(fen) }; // Extract info from FEN string
		std::copy(std::begin(newPos.squares), std::end(newPos.squares), squares); // Overwrite board squares

		stateHistory.clear();
		history.clear();

		Square enPassantSquare = newPos.enPassantSquare;
		int fiftyMoveCounter = newPos.fiftyMoveCount;
		CastlingRights cr = newPos.cr;

		std::fill(pieceCount, pieceCount + int(PIECE_NB), 0);
		std::fill(sideValues, sideValues + int(COLOR_NB), 0);
		std::fill(pieceSquareValues, pieceSquareValues + int(COLOR_NB), 0);
		std::fill(MMPieces, MMPieces + int(COLOR_NB), 0);

		for (Square sq = START_SQUARE; sq < SQUARES_NB; ++sq) {
			Piece piece = squares[sq];

			if (piece == NO_PIECE) {
				continue;
			}

			PieceType type = typeOf(piece);
			Color color = colorOf(piece);

			pieceCount[piece] += 1;

			sideValues[color] += PieceEvaluations::pieceVals[type];
			pieceSquareValues[color] += PieceEvaluations::pieceEvals[type][color == WHITE ? sq : flipRow(sq)];

			if (type != PAWN && type != KING) {
				MMPieces[color] += 1;
			} else if (type == KING) {
				kingSquares[color] = sq;
			}
		}

		// If number of kings of either side isn't one, illegal board position
		if (pieceCount[W_KING] != 1 || pieceCount[B_KING] != 1) {
			loadPosition(FEN::startpos);
			return;
		}

		// Init the histories and state of the board
		mMoveCounter = newPos.moveCount;
		mSideToMove = newPos.sideToMove;

		BoardState newState = BoardState(NO_PIECE, enPassantSquare, cr, fiftyMoveCounter, 0ULL, Move{});
		stateHistory.push(newState);
		state = &stateHistory.back();
		state->zobristHash = ZobristHash::hashBoard(this);
		history.push(state->zobristHash, false);

		initBitboards(); // Init bitboards
	}

	// Synchronise the board position with the bitboards
	void Board::initBitboards() {
		std::fill(typesBB, typesBB + int(PIECE_TYPE_NB), 0ULL);
		std::fill(colorsBB, colorsBB + int(COLOR_NB), 0ULL);

		for (Square sq = START_SQUARE; sq < SQUARES_NB; ++sq) {
			Piece piece = squares[sq];
			if (piece == NO_PIECE) {
				continue;
			}

			PieceType type = typeOf(piece);
			Color color = colorOf(piece);

			typesBB[type] ^= 1ULL << sq;
			typesBB[ALL_PIECES] ^= 1ULL << sq;
			colorsBB[color] ^= 1ULL << sq;
		}
	}

	// Enact a move on the board
	void Board::makeMove(Move move) {
		Square from = move.from();
		Square to = move.to();
		Move::Flag flag = move.flag();
		Piece piece = squares[from];

		Square capturedSquare = to;
		Piece capturedPiece = squares[capturedSquare];
		
		Square enPassantSquare = NONE_SQUARE;
		CastlingRights cr = state->cr;
		Bitboard newZobristHash = state->zobristHash;
		int fiftyMoveCounter = (piece == makePiece(PAWN, mSideToMove) || capturedPiece != NO_PIECE) ? 0 : state->fiftyMoveCounter + 1;

		assert(typeOf(capturedPiece) != KING && piece != NO_PIECE);

		newZobristHash ^= ZobristHash::whiteMoveHash; // Change hash move side
		
		if (state->enPassantSquare != NONE_SQUARE)
			newZobristHash ^= ZobristHash::enPassantHash[state->enPassantSquare];

		if (flag == Move::Flag::EN_PASSANT) {
			capturedSquare -= pawnPush(mSideToMove);
			capturedPiece = squares[capturedSquare];
		}

		if (capturedPiece != NO_PIECE) {
			deletePiece(capturedSquare);

			PieceType type = typeOf(capturedPiece);

			if (type == ROOK) {
				cr = cancelRookCastlingRights(cr, ~mSideToMove, capturedSquare);
			}

			newZobristHash ^= ZobristHash::pieceHashes[~mSideToMove][type][capturedSquare];
		}

		movePiece(from, to);
		newZobristHash ^= ZobristHash::pieceHashes[mSideToMove][typeOf(piece)][from] ^ ZobristHash::pieceHashes[mSideToMove][typeOf(piece)][to];

		if (move.isPromotion()) {
			Piece promotionPiece = makePiece(PieceType(flag), mSideToMove); // Move flag promotions inherit piecetype values
			deletePiece(to);
			placePiece(promotionPiece, to);
			newZobristHash ^= ZobristHash::pieceHashes[mSideToMove][typeOf(piece)][to] ^ ZobristHash::pieceHashes[mSideToMove][typeOf(promotionPiece)][to];
		} else if (flag == Move::Flag::CASTLE) {
			Square rookFrom = rCastleFrom(from, to);
			Square rookTo = rCastleTo(from, to);

			movePiece(rookFrom, rookTo);
			newZobristHash ^= ZobristHash::pieceHashes[mSideToMove][ROOK][rookFrom] ^ ZobristHash::pieceHashes[mSideToMove][ROOK][rookTo];
		} else if (flag == Move::Flag::PAWN_TWO_SQUARES) {
			enPassantSquare = to - pawnPush(mSideToMove);
			newZobristHash ^= ZobristHash::enPassantHash[enPassantSquare];
		}

		switch (typeOf(piece)) {
		case KING:
			cr = cancelCastlingRights(cr, mSideToMove);
			newZobristHash ^= ZobristHash::castlingRightsHash[int(state->cr)] ^ ZobristHash::castlingRightsHash[int(cr)];
			break;
		case ROOK:
			cr = cancelRookCastlingRights(cr, mSideToMove, from);
			newZobristHash ^= ZobristHash::castlingRightsHash[int(state->cr)] ^ ZobristHash::castlingRightsHash[int(cr)];
			break;
		}				

		stateHistory.push(BoardState(capturedPiece, enPassantSquare, cr, fiftyMoveCounter, newZobristHash, move));
		state = &stateHistory.back();

		bool reset = (capturedPiece != NO_PIECE) || (piece == makePiece(PAWN, mSideToMove));

		history.push(state->zobristHash, reset);

		mMoveCounter++;
		mSideToMove = ~mSideToMove;
	}

	// Roll back state of board from move
	void Board::unMakeMove() {
		mSideToMove = ~mSideToMove;
		mMoveCounter--;

		Move move = state->prevMove;
		Square from = move.from();
		Square to = move.to();
		Piece piece = squares[to];
		Piece capturedPiece = state->capturedPiece;
		Move::Flag flag = move.flag();

		if (move.isPromotion()) {
			deletePiece(to);
			placePiece(makePiece(PAWN, mSideToMove), to);
		} else if (flag == Move::Flag::CASTLE) {
			Square rookFrom = rCastleFrom(from, to);
			Square rookTo = rCastleTo(from, to);

			movePiece(rookTo, rookFrom);
		}

		movePiece(to, from);

		if (capturedPiece != NO_PIECE) {
			Square capturedSquare = to;

			if (flag == Move::Flag::EN_PASSANT) {
				capturedSquare -= pawnPush(mSideToMove);
			}

			placePiece(capturedPiece, capturedSquare);
		}

		// Rollback states
		history.pop();
		stateHistory.pop();
		state = &stateHistory.back();
	}

	// Prints the formatted board position, FEN representation, and the zobrist hash
	void Board::printBoard() const {
		string result = "";

		for (int y = 0; y < 8; y++) {
			result += "+---+---+---+---+---+---+---+---+\n";

			for (int x = 0; x < 8; x++) {
				int squareIndex = CoordHelper::coordToIndex(y, x);

				Piece piece = squares[squareIndex];

				result += "| ";
				result += pieceToSymbol(piece);
				result += " ";

				if (x == 7) {
					// Show rank number
					result += "| " + to_string(8 - y);
					result += '\n';
				}
			}

			if (y == 7) {
				// Show file names
				result += "+---+---+---+---+---+---+---+---+\n";
				const string fileNames = "  a   b   c   d   e   f   g   h  \n";
				const string fileNamesRev = "  h   g   f   e   d   c   b   a  \n";
				result += fileNames;
				result += '\n';
			}
		}

		result += "Fen: ";
		result += FEN::generateFEN(this);
		result += '\n';

		result += "Key: ";
		result += to_string(state->zobristHash);
		result += '\n';

		cout << result << endl;
	}

	void Board::printBitboards() const {
		std::cout << "Piece Type Bitboards:\n";

		for (PieceType type = PAWN; type < PIECE_TYPE_NB; ++type) {
			std::cout << pieceToSymbol(makePiece(type, WHITE)) << " Bitboard:\n";
			printBB(typesBB[type]);
		}

		std::cout << "\nColor Bitboards:\n";

		printBB(colorsBB[BLACK]);
		printBB(colorsBB[WHITE]);

		std::cout << "\nAll Pieces Bitboards:\n";

		printBB(typesBB[ALL_PIECES]);
	}

	void Board::movePiece(Square from, Square to) {
		Piece piece = squares[from];
		
		Bitboard fromTo = (1ULL << from) | (1ULL << to);

		assert(piece != NO_PIECE);
		assert(squares[to] == NO_PIECE);

		typesBB[typeOf(piece)] ^= fromTo;
		colorsBB[colorOf(piece)] ^= fromTo;
		typesBB[ALL_PIECES] ^= fromTo;

		squares[to] = squares[from];
		squares[from] = NO_PIECE;

		Square evalFrom = colorOf(piece) == WHITE ? from : flipRow(from);
		Square evalTo = colorOf(piece) == WHITE ? to : flipRow(to);

		pieceSquareValues[colorOf(piece)] += PieceEvaluations::pieceEvals[typeOf(piece)][evalTo] 
			- PieceEvaluations::pieceEvals[typeOf(piece)][evalFrom];

		if (typeOf(piece) == KING) {
			kingSquares[colorOf(piece)] = to;
		}
	}

	void Board::placePiece(Piece piece, Square sq) {
		assert(piece != NO_PIECE);
		assert(typeOf(piece) != KING);

		Bitboard sqBB = 1ULL << sq;

		typesBB[typeOf(piece)] |= sqBB;
		colorsBB[colorOf(piece)] |= sqBB;
		typesBB[ALL_PIECES] |= sqBB;

		pieceCount[piece] += 1;

		squares[sq] = piece;

		Square evalSq = colorOf(piece) == WHITE ? sq : flipRow(sq);

		sideValues[colorOf(piece)] += PieceEvaluations::pieceVals[typeOf(piece)];
		pieceSquareValues[colorOf(piece)] += PieceEvaluations::pieceEvals[typeOf(piece)][evalSq];

		if (typeOf(piece) != PAWN) {
			MMPieces[colorOf(piece)] += 1;
		}
	}

	void Board::deletePiece(Square sq) {
		Piece piece = squares[sq];

		assert(piece != NO_PIECE);
		assert(typeOf(piece) != KING);

		Bitboard captureBB = 1ULL << sq;

		typesBB[typeOf(piece)] &= ~captureBB;
		colorsBB[colorOf(piece)] &= ~captureBB;
		typesBB[ALL_PIECES] &= ~captureBB;

		pieceCount[piece] -= 1;

		squares[sq] = NO_PIECE;

		Square evalSq = colorOf(piece) == WHITE ? sq : flipRow(sq);
		sideValues[colorOf(piece)] -= PieceEvaluations::pieceVals[typeOf(piece)];
		pieceSquareValues[colorOf(piece)] -= PieceEvaluations::pieceEvals[typeOf(piece)][evalSq];

		if (typeOf(piece) != PAWN) {
			MMPieces[colorOf(piece)] -= 1;
		}
	}

}