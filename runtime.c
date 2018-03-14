void stub()
{
    asm(".globl _tiger_resume");
    asm("jmp _tiger_entry");
    asm("_tiger_resume:");
}
int main()
{
    stub();
    return 0;
}