// ConstFold DDE

void empty_void() {}

void local_void() { int x = 0; }

int return_const() { return 42; }

int identity(int x) { return x; }

int add(int a, int b) { return a + b; }

int add3(int a, int b, int c) { return a + b + c; }

int call_simple() { return add(6, 7); }

int call_with_locals() {
  int x = 10;
  int y = 20;
  return add(x, y);
}

int nested_call() { return add(add(1, 2), 4); }

int call_void_then_return() {
  local_void();
  return 5;
}

int if_return(int x) {
  if (x)
    return 1;

  return 0;
}

int if_else_return(int x) {
  if (x)
    return 10;
  else
    return 20;
}

int multiple_returns(int x) {
  if (x < 0)
    return -1;

  if (x == 0)
    return 0;

  return 1;
}

int logical_return(int a, int b) { return a && b; }

int logical_return2(int a, int b) { return a || b; }

int unary_return(int x) { return !(-x); }

int loop_return() {
  int i = 0;
  int sum = 0;

  while (i < 5) {
    sum = sum + i;
    i = i + 1;
  }

  return sum;
}

int forward_target();

int forward_caller() { return forward_target(); }

int forward_target() { return 99; }

int factorial(int n) {
  if (n <= 1)
    return 1;

  return n * factorial(n - 1);
}

int fib(int n) {
  if (n <= 1)
    return n;

  return fib(n - 1) + fib(n - 2);
}

int main() {
  int x = call_simple();
  int y = nested_call();
  int z = factorial(5);

  return x + y + z;
}
