#ifndef ASSEM_H
#define ASSEM_H

#include "assem.h"
#include "frame.h"
#include "tree.h"

namespace tiger{

class InstrList{
};


class CodeGenerator{
public:
    InstrList* CodeGen(FrameBase* f,StatementBaseList* l);
private:
    void Munch(InstrList* il,FrameBase* f,StatementBaseList* l){
    }
};



}// namespace tiger

#endif
