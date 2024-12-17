#pragma once

#ifndef PIECEEVALUATIONS_H
#define PIECEEVALUATIONS_H

class PieceEvaluations {
public:
    static short int pawnEval[64];

    static short int pawnEndgameEval[64];

    static short int knightEval[64];

    static short int bishopEval[64];

    static short int rookEval[64];

    static short int queenEval[64];

    static short int kingEval[64];

    static short int kingEndgameEval[64];
    static short int* pieceEvals[7];

    static constexpr int emptyVal = 0;
    static constexpr int pawnVal = 100;
    static constexpr int knightVal = 320;
    static constexpr int bishopVal = 330;
    static constexpr int rookVal = 500;
    static constexpr int queenVal = 900;
    static constexpr int kingVal = 0;
    static constexpr int pieceVals[7] = { emptyVal, pawnVal, knightVal, bishopVal, rookVal, queenVal, kingVal };

};

#endif // !PIECEEVALUATIONS_H
