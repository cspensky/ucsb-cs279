#include <stdio.h>

#include <stdlib.h>



int main(int argc, char *argv[])

{

   if(!argv[1])

      exit(1);

   printf("%#x\n", getenv(argv[1]));

   return 0;

}
