#ifndef REGALLOC_H
#define REGALLOC_H

#include "tiger_assert.h"
#include "assem.h"
#include "frame.h"
#include "graph.h"
#include "regalloc.h"

namespace tiger{

void RegAlloc(TempMapList* map,LivenessResult* lr,FrameBase* f,InstrList* il);

}//namespace tiger
#endif
