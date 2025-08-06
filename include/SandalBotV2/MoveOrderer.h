#ifndef MOVEORDERER_H
#define MOVEORDERER_H

namespace SandalBot {

	class MoveGen;

	namespace MoveOrder {
		// Heuristically orders an array of moves from best to worst.
		void order(MoveGen* generator);

	};

}

#endif