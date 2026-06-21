int choose(int cond) {
  int x = 10;

  if (cond)
    x = 20;
  else
    x = 30;

  return x;
}

int flip(int cond) {
  int y = 1;

  if (cond)
    y = y + 5;
  else
    y = y + 9;

  return y;
}

int main() {
  int z = 0;

  if (z)
    z = 1;
  else
    z = 2;

  return z;
}
