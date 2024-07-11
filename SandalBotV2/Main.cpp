#include <iostream>
#include <bitset>
#include <vector>
#include "Piece.h"
#include "Move.h"
using namespace std;

int main() {
	vector<char> pieceSymbols;
	for (int piece : Piece::pieces) {
		//cout << Piece::pieceToSymbol(piece);
		pieceSymbols.push_back(Piece::pieceToSymbol(piece));
	}
	for (char symbol : pieceSymbols) {
		cout << Piece::symbolToPiece(symbol) << ", ";
	}
	return 0;
}