// ConstantFolding DBrE DBE DDE

int simple_true() {
  int x = 1;
  if (x == 1) {
    return 1;
  } else {
    return 0;
  }
}

int simple_false() {
  int x = 1;
  if (x == 0) {
    return 1;
  } else {
    return 0;
  }
}

int nested_dead() {
  int x = 1;
  if (x == 1) {
    if (x != 0) {
      return 10;
    } else {
      return 20;
    }
  } else {
    return 30;
  }
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

int while_never_runs() {
  int x = 0;
  while (x == 0) {
    return 1;
  }

  return 2;
}

int while_always_runs() {
  int x = 1;
  while (x == 1) {
    return 7;
  }

  return 0;
}

int chained_cfg() {
  int x = 6 / 3;

  if (x == 2) {
    return 1;
  } else {
    return 0;
  }
}

int multiple_unreachable() {
  int x = 1;
  if (x == 1) {
    return 1;
  }

  return 2;
}

int phi_test(int x) {
  int z = 1;
  int y = 0;

  if (z == 1) {
    y = 10;
  } else {
    y = 20;
  }

  return y;
}
