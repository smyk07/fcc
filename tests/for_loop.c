int simple_for() {
  int sum = 0;

  for (int i = 0; i < 5; i = i + 1) {
    sum = sum + i;
  }

  return sum;
}

int multi_var() {
  int sum = 0;
  int prod = 1;

  for (int i = 1; i <= 4; i = i + 1) {
    sum = sum + i;
    prod = prod * 2;
  }

  return sum + prod;
}

int for_continue() {
  int sum = 0;

  for (int i = 0; i < 6; i = i + 1) {
    if (i == 3)
      continue;

    sum = sum + i;
  }

  return sum;
}

int for_break() {
  int sum = 0;

  for (int i = 0; i < 10; i = i + 1) {
    if (i == 7)
      break;

    sum = sum + i;
  }

  return sum;
}

int break_continue_for() {
  int sum = 0;

  for (int i = 0; i < 10; i = i + 1) {
    if (i == 2)
      continue;

    if (i == 8)
      break;

    sum = sum + i;
  }

  return sum;
}
