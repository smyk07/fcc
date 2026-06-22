int wheelie() {
  int i = 0;

  while (i < 10) {
    i = i + 1;
  }

  return i;
}

int sum_while() {
  int i = 0;
  int sum = 0;

  while (i < 4) {
    sum = sum + i;
    i = i + 1;
  }

  return sum;
}

int branch_in_loop() {
  int i = 0;
  int x = 0;

  while (i < 6) {
    if (i < 3)
      x = x + 1;
    else
      x = x + 2;

    i = i + 1;
  }

  return x;
}

int loop_loop_loop() {
  int n = 0;

  int i = 0;
  int j = 0;
  int k = 0;

  while (i < 10) {
    while (j < 10) {
      while (k < 10) {
        n = n + 1;
        k = k + 1;
      }
      j = j + 1;
    }
    i = i + 1;
  }

  return n;
}
