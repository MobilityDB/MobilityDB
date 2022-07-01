#include <stdio.h>

#define BITARRAY_SIZE(x) (x/8+(!!(x%8)))

void toggle_bit(char *array, int index)
{
  array[index / 8] ^= 1 << (index % 8);
}

char
get_bit(char *array, int index)
{
  return 1 & (array[index / 8] >> (index % 8));
}

int main(void)
{
  /* initialize empty array with the right size */
  char x[BITARRAY_SIZE(100)] = { 0 };
  int i;

  for (i = 0; i < 100; i += 2)
    toggle_bit(x, i);
  toggle_bit(x, 56);
  for (i = 0; i < SIZE; i++)
    printf("%d: %d\n", i, get_bit(x, i));

  return 0;
}
