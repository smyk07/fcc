int break_continue() {
  int i = 0;
  int sum = 0;

  while (i < 10) {
    i = i + 1;

    if (i == 3) {
      continue;
    }

    if (i == 7) {
      break;
    }

    sum = sum + i;
  }

  return sum;
}

int nested_break() {
  int i = 0;
  int total = 0;

  while (i < 3) {
    int j = 0;

    while (j < 10) {
      if (j == 2)
        break;

      total = total + 1;
      j = j + 1;
    }

    i = i + 1;
  }

  return total;
}

int nested_continue() {
  int i = 0;
  int total = 0;

  while (i < 2) {
    int j = 0;

    while (j < 4) {
      j = j + 1;

      if (j == 2)
        continue;

      total = total + 1;
    }

    i = i + 1;
  }

  return total;
}

int mix_n_match() {
  int i = 0;
  int sum = 0;

  while (i < 10) {
    i = i + 1;

    if (i == 2)
      continue;

    if (i == 8)
      break;

    if (i < 5)
      sum = sum + 1;
    else
      sum = sum + 2;
  }

  return sum;
}
