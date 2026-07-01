// ConstFold DBrE DDE

int always_true() {
  if (1) {
    return 10;
  } else {
    return 20;
  }
}

int always_false() {
  if (0) {
    return 10;
  } else {
    return 20;
  }
}

int folded_true() {
  if (6 > 3) {
    return 1;
  } else {
    return 0;
  }
}

int folded_false() {
  if ((2 * 3) == 7) {
    return 1;
  } else {
    return 0;
  }
}

int nested_dead_branch() {
  if (1) {
    if (0) {
      return 1;
    } else {
      return 2;
    }
  }

  return 3;
}

int logical_and_dead() {
  if (1 && 0) {
    return 1;
  }

  return 0;
}

int logical_or_dead() {
  if (0 || 42) {
    return 1;
  }

  return 0;
}

int chained() {
  int x = 6 / 3;
  if (x == 2) {
    return 1;
  } else {
    return 0;
  }
}
