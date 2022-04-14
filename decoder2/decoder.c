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
*               -t <stackAddr>          Stack Address (default = 65535)
*               -v                                      Enable Verbose mode (default = Silent Mode)
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "decoder.h"

#define MAX 16384        // Maximum number of instructions to take from .mem file

int32_t *program_location;
int program_size;
FILE *fp;

int main(int argc, char *argv[])
{
    bool debug = false;                                                             //debug used for verbose mode
    int c, i = 0, startAddr = 0, stackAddr = 65536;
    char fileName[] = "program.mem", numStr[5];

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
                if(startAddr%4 != 0){
                    printf("ERROR: Starting PC must be 4-byte aligned\n");
                    return(1);
                }
                i += 2;
                break;
            case 't':                                               // get stack address from command line arguments
                strcpy(numStr, argv[i+1]);      // if present
                stackAddr = atoi(numStr);
                i += 2;
                break;
            case 'v':                                               // enable verbose mode if tag present
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

    PC = startAddr;

    reg_file[2] = stackAddr;        // Register 2 is the stack pointer

    char target[MAX];
    char delim[] = " :";
    char *ptr;
    int instructionCount = 0;

    fp = fopen(fileName, "r");

    if (!fp){                                   //Check for file in immediate folder
        char fileName1[] = "../memfiles/";      //if not found in immediate folder,
        strcat(fileName1, fileName);            //check ../memfiles
        fp = fopen(fileName1, "r");             //if not in ../memfiles
        if (!fp){                               //check ../
            char fileName2[] = "../";           
            strcat(fileName2, fileName);
            
            printf("Error opening file..\n");   
        }
        return(1);
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

    if(debug) printf("PROGRAM INSTRUCTIONS:\n");
    IR_decoder(memory, debug);
    if(debug) printf("\n\n");
    printMemory(memory);
    
	return 0;
}
