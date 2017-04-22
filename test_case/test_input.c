int if_else(int n) {
    int cost;
    if (n > 500) {
        return 1;
    } else if (n > 300) {
        return 2;
    } else if (n > 100) {
        return 3;
    } else if (n > 50) {
        return 4;
    } else {
        return 5;
    }
}


int main() {
    int NR_DATA;
    int i;
    int data;
    i = 0;
    NR_DATA = read();

    while (i < NR_DATA) {
        data = read();
        write(if_else(data));
        i = i + 1;
    }

    return 0;
}
