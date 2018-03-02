#include <iostream>

#include "scanner.h"
#include "absyn.h"
#include "parser.h"
#include "tiger_log.h"
#include "tiger_assert.h"
#include "types.h"
#include "semant.h"
#include "escape.h"
#include "tree.h"

void test_StringSourceCodeStream()
{
    char *string=(char*)"just a test";
    s32 len=strlen(string);
    tiger::scanner::StringSourceCodeStream stream((char*)"just a test");
    for(int i=0;i<len;i++)
        assert(*(string+i)==stream.Next());
    assert(tiger::scanner::kSourceCodeStream_EOS==stream.Next());
}
void test_FileSourceCodeStream()
{
    char *string=(char*)"just b test";
    s32 len=strlen(string);
    tiger::scanner::FileSourceCodeStream stream((char*)"a.txt");
    for(int i=0;i<len;i++)
        assert(*(string+i)==stream.Next());
    assert(tiger::scanner::kSourceCodeStream_EOS==stream.Next());
}
void test_Next_With_StringSourceCodeStream()
{
    s32 v,v1;
    tiger::Token t,t1;
    tiger::scanner::StringSourceCodeStream stream((char*)".=");
    tiger::scanner::Scanner scanner(&stream);
    
    v = scanner.Next(&t);
    assert(v==tiger::kToken_DOT);
    std::cout<<t.lineno<<","<<t.pos<<","<<t.abs_pos<<std::endl;
    
    v1 = scanner.Next(&t1);
    assert(v1==tiger::kToken_EQUAL);
    std::cout<<t1.lineno<<","<<t1.pos<<","<<t1.abs_pos<<std::endl;
    
    scanner.Back(&t1);
    scanner.Back(&t);
    
    v = scanner.Next(&t);
    assert(v==tiger::kToken_DOT);
    std::cout<<t.lineno<<","<<t.pos<<","<<t.abs_pos<<std::endl;


}
void test_Next_With_FileSourceCodeStream()
{
    s32 v;
    tiger::Token t;
    tiger::scanner::FileSourceCodeStream stream((char*)"a.txt");
    tiger::scanner::Scanner scanner(&stream);
    
    t.Clear();
    v = scanner.Next(&t);
    assert(v==tiger::kToken_ID);
    assert(strcmp(t.u.name,"just")==0);
    std::cout<<t.lineno<<","<<t.pos<<std::endl;

}
void test_sbsyn()
{
    tiger::Symbol *a,*b;
    a = new tiger::Symbol((char*)"a");
    b = new tiger::Symbol((char*)"b");
    tiger::SimpleVar* var = new tiger::SimpleVar(a);
    
}
void test_parser()
{
    tiger::scanner::FileSourceCodeStream stream((char*)"b.txt");
    //tiger::scanner::StringSourceCodeStream stream((char*)"`");
    tiger::Exp* exp;
    tiger::parser::Parser parser(&stream);
    parser.Parse(&exp);
    delete exp;
}
void test_Logger(){
    tiger::LoggerFile logger;
    logger.SetLevel(tiger::LoggerBase::kLogger_Level_Error);
    logger.D("debug info");
    logger.I("info info");
    logger.W("warn info");
    logger.E("error info");
}
void test_assert(){
    TIGER_ASSERT(1==0,"Expected %s %d",__FILE__,__LINE__);
}
void test_types(){
    tiger::TypeBase* a;
    a = new tiger::TypeNil;
    
    assert(a->Kind()==tiger::TypeBase::kType_Nil);
    tiger::LoggerStdio logger;
    logger.SetLevel(tiger::LoggerBase::kLogger_Level_Error);
    logger.D("%d",a->Kind());
}
void test_symtab(){
    tiger::SymTab symtab;
    symtab.Enter(symtab.MakeSymbol(new tiger::Symbol("a")),0);
    symtab.Enter(symtab.MakeSymbol(new tiger::Symbol("a")),0);
    symtab.BeginScope(tiger::ScopeMaker::kScope_Invalid);
    symtab.Enter(symtab.MakeSymbol(new tiger::Symbol("a")),0);
    symtab.Enter(symtab.MakeSymbol(new tiger::Symbol("b")),0);
    symtab.EndScope();
    std::cout<<"-----"<<std::endl;

}
void test_typecheck(){
    tiger::Exp* exp;
    tiger::ExpBaseTy* ty;
    
    tiger::LoggerStdio logger;
    logger.SetLevel(tiger::LoggerBase::kLogger_Level_Error);
    //logger.setModule("main");
    
    //tiger::scanner::FileSourceCodeStream stream((char*)"a.txt");
    //tiger::scanner::FileSourceCodeStream stream((char*)"b.txt");
    tiger::scanner::StringSourceCodeStream stream((char*)"let type X={a:int} var b:=X{a=10} in b.a:=11 end");
    
    /* generate sbstract syntax tree*/
    tiger::parser::Parser parser(&stream);
    parser.Parse(&exp);

    tiger::SymTab venv,tenv;
    /* init types */
    tenv.Enter(tenv.MakeSymbolFromString("nil"),new tiger::EnvEntryVar(new tiger::TypeNil(),tiger::EnvEntryVar::kEnvEntryVar_For_Type, 0));
    tenv.Enter(tenv.MakeSymbolFromString("void"),new tiger::EnvEntryVar(new tiger::TypeVoid(),tiger::EnvEntryVar::kEnvEntryVar_For_Type, 0));
    tenv.Enter(tenv.MakeSymbolFromString("int"),new tiger::EnvEntryVar(new tiger::TypeInt(),tiger::EnvEntryVar::kEnvEntryVar_For_Type, 0));
    tenv.Enter(tenv.MakeSymbolFromString("string"),new tiger::EnvEntryVar(new tiger::TypeString(),tiger::EnvEntryVar::kEnvEntryVar_For_Type, 0));
    
    /* for each external function, it's impossible to access outer level's variable 
    the outer most level do not need frame and actual list 
    */
    
    /* internal functions */
    /* print(x:string)*/
    tiger::TypeFieldNode* node;
    node = new tiger::TypeFieldNode;
    node->m_field = new tiger::TypeField(tenv.MakeSymbolFromString("x"),tenv.Type(tenv.MakeSymbolFromString("string")));
    venv.Enter(venv.MakeSymbolFromString("print"),new tiger::EnvEntryFun(new tiger::TypeFieldList(node),0,0,0));
    
    /* getchar()*/
    //tiger::TypeFieldNode* node;
    venv.Enter(venv.MakeSymbolFromString("getchar"),new tiger::EnvEntryFun(new tiger::TypeFieldList(0),tenv.Type(tenv.MakeSymbolFromString("string")),0,0));

    /* ord(s:string):int*/
    //tiger::TypeFieldNode* node;
    node = new tiger::TypeFieldNode;
    node->m_field = new tiger::TypeField(tenv.MakeSymbolFromString("s"),tenv.Type(tenv.MakeSymbolFromString("string")));
    venv.Enter(venv.MakeSymbolFromString("ord"),new tiger::EnvEntryFun(new tiger::TypeFieldList(node),tenv.Type(tenv.MakeSymbolFromString("int")),0,0));
    
    /* chr(i:int):string*/
    //tiger::TypeFieldNode* node;
    node = new tiger::TypeFieldNode;
    node->m_field = new tiger::TypeField(tenv.MakeSymbolFromString("i"),tenv.Type(tenv.MakeSymbolFromString("int")));
    venv.Enter(venv.MakeSymbolFromString("chr"),new tiger::EnvEntryFun(new tiger::TypeFieldList(node),tenv.Type(tenv.MakeSymbolFromString("string")),0,0));
    
    // find escape 
    tiger::EscapeHelper escaper;
    escaper.FindEscape(exp);
    
    tiger::Translator translator;
    ty=translator.TransExp(&venv,&tenv,translator.OuterMostLevel(),exp,0);
    
    translator.Traverse( ty->Tree() );
    
    // dump tree
    char s[1024]={0};
    
    if(ty->Tree()->Kind()==tiger::TreeBase::kTreeBase_Ex)
    {
        dynamic_cast<tiger::TreeBaseEx*>(ty->Tree())->GetExp()->Dump(s);
        delete dynamic_cast<tiger::TreeBaseEx*>(ty->Tree())->GetExp();
    }
    
    if(ty->Tree()->Kind()==tiger::TreeBase::kTreeBase_Nx)
    {
        dynamic_cast<tiger::TreeBaseNx*>(ty->Tree())->GetStatement()->Dump(s);
        delete dynamic_cast<tiger::TreeBaseNx*>(ty->Tree())->GetStatement();
    }
    if(ty->Tree()->Kind()==tiger::TreeBase::kTreeBase_Cx)
    {
        dynamic_cast<tiger::TreeBaseCx*>(ty->Tree())->GetStatement()->Dump(s);
        delete dynamic_cast<tiger::TreeBaseCx*>(ty->Tree())->GetStatement();
    }
    
    printf("\n%s\n",s);
    
    
    translator.TraverseFragList();
    
    delete ty;
    
    /* free */
    delete exp;
}
void test_escape()
{
    tiger::Exp* exp;
    
    //tiger::scanner::FileSourceCodeStream stream((char*)"a.txt");
    //tiger::scanner::FileSourceCodeStream stream((char*)"b.txt");
    tiger::scanner::StringSourceCodeStream stream((char*)"let type a1={a:int,b:string} var a_:=a1{a=0,b=\"\"} in let in a_.b:=\"\" end end");
    
    /* generate sbstract syntax tree*/
    tiger::parser::Parser parser(&stream);
    parser.Parse(&exp);

    
    tiger::EscapeHelper escaper;
    escaper.FindEscape(exp);
   
    /* free */
    delete exp;
}
void test_tree(){
    tiger::LoggerStdio logger;
    logger.SetModule("tree");
    logger.SetLevel(tiger::LoggerBase::kLogger_Level_Error);
    
    tiger::ExpBaseName* e;
    e = new tiger::ExpBaseName(tiger::TempLabel::NewNamedLabel("main"));
    
    logger.D(e->GetLabel()->Name());
    
    tiger::TempLabel::Exit();
    
    delete e;
    
}
void test_litstringlist(){
    tiger::LoggerStdio logger;
    logger.SetModule("tree");
    logger.SetLevel(tiger::LoggerBase::kLogger_Level_Error);
    
    tiger::LitStringList list;
    tiger::Label* l1,* l2,* l3;
    l1 = tiger::TempLabel::NewNamedLabel("a");
    l2 = tiger::TempLabel::NewNamedLabel("b");
    l3 = tiger::TempLabel::NewNamedLabel("c");
    list.Insert(l1,"a");
    list.Insert(l1,"b");
    logger.D(list.Find(l1));
    tiger::TempLabel::Exit();
}
int main()
{
    //test_Next_With_StringSourceCodeStream();
    //test_sbsyn();
    //test_parser();
    //test_Logger();
    //test_assert();
    //test_types();
    //test_symtab();
    //test_escape();
    //test_tree();
    //test_litstringlist();
    test_typecheck();
    return 0;
}
