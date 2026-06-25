// ConstantFolding DDE

int basic_fold() {
  int x = 6 + 7;
  return x;
}

int nested_fold() {
  int x = (2 + 3) * (4 + 1);
  return x;
}

int cmp_fold() {
  int x = 3 < 5;
  return x;
}

int eq_fold() {
  int x = 42 == 42;
  return x;
}

int no_fold(int a) {
  int x = a + 5;
  return x;
}

int div_zero() {
  int z = 0;
  int x = 10 / z;
  return x;
}

int more_fold() {
  int a = 1 + 2;
  int b = 3 + 4;
  int c = a * b;
  int d = c - 7;
  return d;
}
