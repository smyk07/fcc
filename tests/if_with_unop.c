int if_or(int a, int b) {
  if (a || b) {
    return 1;
  }

  return 0;
}

int if_not_and(int a, int b) {
  if (!(a && b)) {
    return 1;
  }

  return 0;
}

int nested_logic(int a, int b, int c) {
  if ((a && b) || c) {
    return 1;
  }

  return 0;
}

int nested_logic2(int a, int b, int c) {
  if (a && (b || c)) {
    return 1;
  }

  return 0;
}

int chain_and(int a, int b, int c) {
  if (a && b && c) {
    return 1;
  }

  return 0;
}

int chain_or(int a, int b, int c) {
  if (a || b || c) {
    return 1;
  }

  return 0;
}

int compare_logic(int x, int y) {
  if ((x < 5) && (y > 2)) {
    return 1;
  }

  return 0;
}
