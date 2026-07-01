int add_assign(int x) {
  x += 1;
  return x;
}

int sub_assign(int x) {
  x -= 1;
  return x;
}

int mul_assign(int x) {
  x *= 2;
  return x;
}

int div_assign(int x) {
  x /= 2;
  return x;
}

int mod_assign(int x) {
  x %= 3;
  return x;
}

int expr_value(int x) {
  int y = (x += (x *= 5));
  return y;
}

int nested_expr(int x) { return (x += 2) * 3; }

int branch_assign(int x, int y) {
  if (x)
    y += 1;
  else
    y -= 1;

  return y;
}

int rhs_expr(int x, int y) {
  x += y * 2;
  return x;
}

int fold_me(int x) {
  x *= 1;
  x += 0;
  x -= 0;
  x /= 1;
  return x;
}

int multi_var(int a, int b) {
  a += b;
  b *= a;
  return a + b;
}

int loop_assign(int n) {
  int sum = 0;

  while (n > 0) {
    sum += n;
    n -= 1;
  }

  return sum;
}
