struct tiny {
    char c;
    char d;
    char e;
};
void foo(struct tiny x)
{
    if (x.c != 1)
        abort();
    if (x.e != 1)
        abort();
}
int main()
{
    struct tiny s;
    s.c = 1;
    s.d = 1;
    s.e = 1;
    foo(s);
    return 0;
}
