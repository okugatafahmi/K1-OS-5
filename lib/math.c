#include "math.h"

int div(int a, int b)
{
  int count = 0;
  while (b <= a)
  {
    a -= b;
    count++;
  }
  return count;
}

int mod(int a, int b)
{
  while (b <= a)
  {
    a -= b;
  }
  return a;
}
