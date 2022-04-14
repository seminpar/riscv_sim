
/*
*		RISC-V Simulator main
*		(need description)
*
*
*		Authors: Derrick Genther, Angeline Alfred, Jehnoy Welsh, Se Min Park
*		
*		Optional Arguments:
*		-f <fileName>		Input memory image file (default = program.mem)
*		-s <startAddr>		Starting address (default = 0)
*		-t <stackAddr>		Stack Address (default = 65535)
*		-v					Enable Verbose mode (default = Silent Mode)
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "decoder.h"
#include "MMU.h"


int main(int argc, char const *argv[])
{

	bool debug = false;								//debug used for verbose mode
	int c, i = 0, startAddr = 0, stackAddr = 65536;
	char fileName[] = "branch.mem", numStr[5];

	while (--argc > 0 && (*++argv)[i] == '-') {
		while (c = *++argv[i]) {
			switch (c) {
			case 'f':						// get file name from command line arguments
				strcpy(fileName, argv[i+1]);// if present
				i += 2;
				break;
			case 's':						// get start address from command line arguments
				strcpy(numStr, argv[i+1]);	// if present
				startAddr = atoi(numStr);
				i += 2;
				break;
			case 't':						// get stack address from command line arguments
				strcpy(numStr, argv[i+1]);	// if present
				stackAddr = atoi(numStr);
				i += 2;
				break;
			case 'v':						// enable verbose mode if tag present
				debug = true;
				i += 1;
				break;
			default:
				printf("RISCV: invalid command line argument");
				argc = 0;
				break;
			}
			if (i >= argc) {
				break;
			}
		}
	}

	// Print command line argument details
	if (debug == true){
		printf("fileName = %s", fileName);
		if(strcmp(fileName, "program.mem")==0)
			printf(" (default)");
		printf("\n");

		printf("startAddr = %d", startAddr);
		if(startAddr == 0)
			printf(" (default)");
		printf("\n");

		printf("stackAddr = %d", stackAddr);
		if(stackAddr == 65535)
			printf(" (default)");
		printf("\n");
	}


	memory_init(stackAddr, startAddr, fileName);
	run_program(true);


	return 0;
}