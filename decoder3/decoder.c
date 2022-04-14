/*
*               RISC-V Simulator main
*               (need description)
*
*
*               Authors: Derrick Genther, Angeline Alfred, Jehnoy Welsh, Se Min Park
*
*               Optional Arguments:
*               -f <fileName>           Input memory image file (default = program.mem)
*               -s <startAddr>          Starting address (default = 0)
*               -t <stackAddr>          Stack Address (default = 65536)
*               -v                      Enable Verbose mode (default = Silent Mode)
*               -b <breakpoint>         Sets a breakpoint for memory and register inspection (default = 0)
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "decoder.h"

int32_t *program_location;
int program_size;
FILE *fp;

int main(int argc, char *argv[])
{
    bool debug = false;                                                             //debug used for verbose mode
    int c, i = 0, startAddr = 0;
    char fileName[50], numStr[32];
    strcpy(fileName, "program.mem");

    while (--argc > 0 && (*++argv)[i] == '-') {
        while (c = *++argv[i]) {
            switch (c) {
            case 'f':                                               // get file name from command line arguments
                strcpy(fileName, argv[i+1]);// if present
                i += 2;
                break;
            case 's':                                               // get start address from command line arguments
                strcpy(numStr, argv[i+1]);      // if present
                startAddr = atoi(numStr);
                if(startAddr < 0){
                    printf("ERROR Starting PC cannot be negative.\n");
                    return(1);
                }
                if(startAddr%4 != 0){
                    printf("ERROR: Starting PC must be 4-byte aligned.\n");
                    return(1);
                }
                i += 2;
                break;
            case 't':                                               // get stack address from command line arguments
                strcpy(numStr, argv[i+1]);      // if present
                stackAddr = atoi(numStr);
                if(startAddr < 0){
                    printf("ERROR Starting Stack Pointer cannot be negative.\n");
                    return(1);
                }
                if(startAddr%4 != 0){
                    printf("ERROR: Starting Stack Pointer address must be 4-byte aligned.\n      (decimal integer multiple of 4)\n");
                    return(1);
                }
                i += 2;
                break;
            case 'v':                                               // enable verbose mode if tag present
                debug = true;
                i += 1;
                break;
            case 'b':
                strcpy(numStr, argv[i+1]);
                breakpoint = atoi(numStr);
                if(breakpoint%4 != 0){
                    printf("ERROR: Breakpoints must be 4-byte aligned (decimal integer multiple of 4)\n");
                    return(1);
                }
                i += 2;
            default:
                printf("RISCV: invalid command line argument\n");
                argc = 0;
                break;
            }
            if (i >= argc) {
                break;
            }
        }
    }

    PC = startAddr;

    //int **memory = (int **)malloc(2 * sizeof(int *));
    //for (int i = 0; i < 2; i++) memory[i] = (int *)malloc(stackAddr * sizeof(int));

    int memSize = (stackAddr / 4);

    //printf("memSize = %d\n", memSize);

    memory = calloc(memSize, sizeof(int *));
    for(int i = 0; i < memSize; i++){
        memory[i] = calloc(2, sizeof(int));
    }

    reg_file[2] = stackAddr;        // Register 2 is the stack pointer

    char target[MAX];
    char delim[] = " :";
    char *ptr;
    int instructionCount = 0;

    fp = fopen(fileName, "r");

    if (!fp){                                   //Check for file in immediate folder
        char fileName1[] = "memfile_tests/";      //if not found in immediate folder,
        strcat(fileName1, fileName);            //check /memfile_tests/
        fp = fopen(fileName1, "r");     
        //printf("fileName1 = %s\n", fileName1);        
        if (!fp){                              
            printf("Error opening file..\n");   
            return(1);
        }
    }
                                        
    int ii = 0;                         
    int jj = 0;

    while (fgets(target, MAX, fp) != NULL) // Until EOF
    {

        char *ptr = strtok(target, delim);

        while (ptr != NULL) // Until end of line
        {
            memory[jj][ii] = (int)strtol(ptr, NULL, 16);
            ii++;
            
            ptr = strtok(NULL, delim);
        }

        jj++;
        ii = 0;
        instructionCount++;
    }
    fclose(fp);

    //printf("instructionCount = %d\n", instructionCount);

    if(debug) printf("PROGRAM INSTRUCTIONS:\n");
    IR_decoder(debug);
    if(debug) printf("\n\n"); 

    //printMemory();
    if (!debug){
	    printPC();
	    printRegFile();
    }

    free(memory);

	return 0;
}
