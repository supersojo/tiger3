#include <iostream>

#include "scanner.h"
#include "absyn.h"
#include "parser.h"

void test_StringSourceCodeStream()
{
    char *string="just a test";
    s32 len=strlen(string);
    tiger::scanner::StringSourceCodeStream stream("just a test");
    for(int i=0;i<len;i++)
        assert(*(string+i)==stream.Next());
    assert(tiger::scanner::kSourceCodeStream_EOS==stream.Next());
}
void test_FileSourceCodeStream()
{
    char *string="just b test";
    s32 len=strlen(string);
    tiger::scanner::FileSourceCodeStream stream("a.txt");
    for(int i=0;i<len;i++)
        assert(*(string+i)==stream.Next());
    assert(tiger::scanner::kSourceCodeStream_EOS==stream.Next());
}
void test_Next_With_StringSourceCodeStream()
{
    s32 v;
    tiger::Token t;
    tiger::scanner::StringSourceCodeStream stream("then b");
    tiger::scanner::Scanner scanner(&stream);
    
    v = scanner.Next(&t);
    assert(v==tiger::kToken_THEN);
    std::cout<<t.lineno<<","<<t.pos<<std::endl;
    
    v = scanner.Next(&t);
    assert(v==tiger::kToken_ID);
    std::cout<<t.lineno<<","<<t.pos<<std::endl;
    


}
void test_Next_With_FileSourceCodeStream()
{
    s32 v;
    tiger::Token t;
    tiger::scanner::FileSourceCodeStream stream("a.txt");
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
    a = new tiger::Symbol("a");
    b = new tiger::Symbol("b");
    tiger::SimpleVar* var = new tiger::SimpleVar(a);
    
}
void test_parser()
{
    tiger::scanner::StringSourceCodeStream stream("a<>3");
    tiger::parser::Parser parser(&stream);
    parser.Parse();
    
}
int main()
{
    //test_Next_With_StringSourceCodeStream();
    //test_sbsyn();
    test_parser();
    return 0;
}