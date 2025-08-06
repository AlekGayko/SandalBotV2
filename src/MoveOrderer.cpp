#include "MoveOrderer.h"

#include "Board.h"
#include "Move.h"
#include "MoveGen.h"
#include "PieceEvaluations.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>

using namespace std;

namespace SandalBot::MoveOrder {

	// Quicksort implementation which sorts moves array based on moveVals point system
	static void quickSort(MovePoint* start, MovePoint* end) {
		if (start >= end)
			return;

		MovePoint* pivotSpot{ end - 1 };
		MovePoint* pivot{ start };

		for (auto it{ start }; it != end; ++it) {
			if (it == pivotSpot)
				continue;

			if (it->value > pivotSpot->value) {
				std::swap(*it, *pivot);

				++pivot;
			}
		}

		std::swap(*pivotSpot, *pivot);

		quickSort(start, pivot);
		quickSort(pivot + 1, end);
	}

	static void bubbleSort(MovePoint* start, MovePoint* end) {
		bool swapped{};
		while (swapped) {
			swapped = false;
			for (auto it{ start }; it + 1 < end; ++it) {
				if (it->value < (it + 1)->value) {
					std::swap(*it, *(it + 1));
					swapped = true;
				}
			}
		}
	}

	static void insertionSort(MovePoint* start, MovePoint* end, PointValue limit) {
		for (auto sortedEnd{ start }, it{ start + 1 }; it != end; ++it) {
			if (it->value >= limit) {
				MovePoint tmp = *it, *it2;
				*it = *++sortedEnd;
				for (it2 = sortedEnd; it2 != start && (it2 - 1)->value < tmp.value; --it2) {
					*it2 = *(it2 - 1);
				}
				*it2 = tmp;
			}
		}
	}

	static void selectionSort(MovePoint* start, MovePoint* end) {
		auto maxElement{ start };

		for (auto it{ start }; it != end - 1; ++it) {
			maxElement = it;
			for (auto it2{ it + 1 }; it2 != end; ++it2) {
				if (it2->value > maxElement->value) {
					maxElement = it2;
				}
			}
			if (it != maxElement)
				std::swap(*it, *maxElement);
		}
	}

	// Assigns each move in decayed c array a heuristic value depending on its effectiveness.
	// Sorts array based on list point values
	void order(MoveGen* generator) {
		// No need to sort one move
		if ((generator->end() - generator->begin()) <= 1)
			return;

		insertionSort(generator->begin(), generator->end(), 200);
		//std::sort(start, end, [](MovePoint& mp1, MovePoint& mp2) { return mp1.value > mp2.value; });
	}

}