

int  func_1()
{
    int l_2[3];
    int l_3 = 0;
    int i0;
    i0 = 0;
    while (i0 < 3)
    {
        l_2[i0] = 0;
        i0 = i0 + 1;
    }
    l_3 = 0;
    while ((l_3 <= 2))
    {
        int l_4 = 73524122;
        int i1;
        l_2[l_3] = l_4;
        l_3 = l_3+1;
    }
    return l_3;
}


int main ()
{
    write(func_1());

    return 0;
}