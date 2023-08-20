#include <stdio.h>
#include <stdint.h>

int main()
{
 char a[10];

 a[0] = 1;
 printf("char a is: %d \n", a[0]);

 a[0] = 0;
 printf("char a is: %d \n", (int)a[0]);

 a[0] = 2;
 printf("char a is: %d \n", (int)a[0]);

 uint8_t b;
 b = (uint8_t)256;
 printf("uint8_t a is: %d \n", b);
}
