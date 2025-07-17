#ifndef INIT_H
#define INIT_H

#include "Bitboards.h"
#include "ZobristHash.h"

namespace SandalBot {

    inline void initGlobals() {
        ZobristHash::initHashes();
        initBitboards();
    }

}

#endif