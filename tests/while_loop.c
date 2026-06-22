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
