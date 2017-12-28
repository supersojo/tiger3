#include <iostream>

#include "scanner.h"


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
    tiger::scanner::StringSourceCodeStream stream("in !");
    tiger::scanner::Scanner scanner(&stream);
    
    v = scanner.Next(&t);
    assert(v==tiger::kToken_IN);
    std::cout<<t.lineno<<","<<t.pos<<std::endl;
    
    v = scanner.Next(&t);
    assert(v==tiger::kToken_LE);
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

int main()
{
    test_Next_With_StringSourceCodeStream();
    return 0;
}