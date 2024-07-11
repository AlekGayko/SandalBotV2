#include <iostream>
#include <bitset>
#include <vector>
#include "Piece.h"
#include "Move.h"
#include "PieceList.h"

using namespace std;

int main() {
	PieceList list = PieceList();
	for (int i = 0; i < 16; i++) {
		list.addPiece(i);
	}

	for (int i = 0; i < list.numPieces; i++) {
		cout << list[i] << ", ";
	}
	cout << endl;
	for (int i = 0; i < 64; i++) {
		cout << list.map[i] << ", ";
	}
	cout << endl;
	list.movePiece(3, 16);
	for (int i = 0; i < list.numPieces; i++) {
		cout << list[i] << ", ";
	}
	cout << endl;
	for (int i = 0; i < 64; i++) {
		cout << list.map[i] << ", ";
	}
	cout << endl;
	return 0;
}