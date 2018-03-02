#ifndef REGALLOC_H
#define REGALLOC_H

#include "tiger_assert.h"
#include "assem.h"
#include "frame.h"

namespace tiger{
s32 RegAlloc(FrameBase* f,InstrList* il);
}//namespace tiger
#endif
