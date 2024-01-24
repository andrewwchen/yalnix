#include <yuser.h>

int main(void )
{
  char big_buffer[10*1024];
  int foo;
  int i;

  foo = 42;
  for (i = 0; i < 10 * 1024; i++) 
    big_buffer[i] = 'a';

  for (i = 0; i < 10*1024; i+=512) 
    TracePrintf(0,"&big_buffer[%d] = %x; big_buffer[%d] = %c\n",
		i, &big_buffer[i], i, big_buffer[i]);

  Exit(0);
}
