#include "tiger_assert.h"
#include "assem.h"


namespace tiger{

InstrList* CodeGenerator::CodeGen(FrameBase* f,StatementBaseList* l){
    InstrList* il = new InstrList;
    Munch(il,f,l);
    return il;
}

}//namespace tiger