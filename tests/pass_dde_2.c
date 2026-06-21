int dead(int cond) {
  int x = 2;
  int y = 3;

  if (cond)
    x = x + y;
  else
    x = y;

  int z = x * 100;
  return x;
}
