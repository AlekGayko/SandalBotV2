#include <iostream>
#include <bitset>
#include "Piece.h"
using namespace std;

int main() {
	unsigned long long int a = 0b1111 << 12;
	std::cout << a << std::endl;
	return 0;
}