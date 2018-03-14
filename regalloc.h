#ifndef REGALLOC_H
#define REGALLOC_H

#include "tiger_assert.h"
#include "assem.h"
#include "frame.h"
#include "graph.h"

namespace tiger{

TempMapList* RegAlloc(LivenessResult* lr,FrameBase* f,InstrList* il);
}//namespace tiger
#endif
