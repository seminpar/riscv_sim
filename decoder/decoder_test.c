#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "decoder.h"

int32_t *program_location;
int program_size;
FILE *fp;

int main()
{
    program_location = (uint32_t *) malloc(sizeof(uint32_t) * 100);
    fp = fopen("branch.txt", "r");

    uint32_t *p = program_location;
    uint32_t x;
    int program_size = 0;
    do{
        fscanf(fp, "%x", &x);
        program_location[program_size++] = x;
    }while(x != 0);


   program_location = realloc(program_location, sizeof(uint32_t) *program_size);
   printf("This is the program:\n");
   int i = 0;
   while(*p)
   {
       int32_t IR = *p;
       IR_decoder(IR, true);
       p++;

   }

    free(program_location);

	return 0;

}
