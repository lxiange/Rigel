int main()
{
	int a = 15, b = 28 , c;
	int s, t;
	c = (a - b) / 2 + a;
	a = c * 3 + a;
	s = a - c;
	write(s);
	t = (a / b) * c + s / c * 4 - b;
	write(t);
	return 0;
}
