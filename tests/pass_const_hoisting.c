// ConstFold DBrE DBE ConstHoisting DDE

int basic(int x) {
  int a = x + 5;
  int b = a - 5;
  int c = b * 5;
  return c / 5;
}

int distinct(int x) {
  int a = x + 1;
  int b = a + 2;
  int c = b + 3;
  int d = c + 2;
  return d + 1;
}

int branches(int x) {
  if (x)
    return 42;
  else
    return 42;
}

int loop(int x) {
  int sum = 0;

  while (x > 0) {
    sum += 7;
    x -= 1;
  }

  return sum + 7;
}

int at_entry(int x) {
  int y = 4;
  if (x)
    return y + 4;
  return 4;
}

int nested(int x, int y) {
  if (x) {
    if (y)
      return 9;
    return 9 + 2;
  }

  return 9 + 2;
}

int logical(int a, int b) { return a && b; }

int logical2(int a, int b) { return a || b; }

int fact(int n) {
  if (n <= 1)
    return 1;

  return n * fact(n - 1);
}

int nodup(int x) { return x + 123; }

int evil_type_shit(int a, int b) {
  int x = 1;
  int y = 1;

  if (a && b)
    x += 1;
  else
    y += 1;

  return x + y + 1;
}
