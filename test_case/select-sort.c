

int main() {
	int i, j, k, t;
	int N = 100;
	int a[100];
	i = 0;
	while(i < N) {
		a[i] = N - i;
		i = i + 1;
	}
	i = 0;
	while(i < N - 1) {
		k = i;
		j = i + 1;
		while(j < N) {
			if(a[j] < a[k]) {
				k = j;
			}

			j = j + 1;
		}

		t = a[i];
		a[i] = a[k];
		a[k] = t;

		i = i + 1;
	}
	i = 0;
	while(i < N) {
		write(a[i]);
		i = i + 1;
	}

	return 0;
}
