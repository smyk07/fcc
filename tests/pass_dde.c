// DDE

int dead() {
  int a = 2;
  int b = 3;

  int c = a + b;
  int d = c * b;
  int e = d - a;
  int f = e + 10;

  return c;
}

int dead2(int cond) {
  int x = 2;
  int y = 3;

  if (cond)
    x = x + y;
  else
    x = y;

  int z = x * 100;
  return x;
}
