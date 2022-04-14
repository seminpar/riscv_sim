
/*  MMU.H - memory management header file  */

#ifndef MMU_H
#define MMU_H


	#include <stdio.h>
	#include <stdlib.h>
	#include <stdlib.h>
	#include <string.h>

	#define MEM_SIZE  0x100000000
	#define MAX		  16384  

	int8_t *MEMORY, *STACK_TOP, *PROGRAM;


	struct IR_field{
		uint8_t opcode;
		uint8_t funct3;
		int funct7;
		uint8_t shamt;
		int rd;
		int rs1;
		int rs2;
		int32_t immediate;
	};

	struct IR_field IR;

	int32_t reg_file[32];

	int32_t PC;

	void System_reset()
	{
		PC = 0;
		for (int i = 0; i < 32; i++)
		{
			reg_file[i] = 0;
		}
	}


	void load_program(char* fileName)
	{
		FILE *fp = fopen(fileName, "r");

	    if (!fp){                                   //Check for file in immediate folder
	        char fileName1[] = "../memfiles/";      //if not found in immediate folder,
	        strcat(fileName1, fileName);            //check ../memfiles
	        fp = fopen(fileName1, "r");             //if not in ../memfiles
	        if (!fp){  
	            printf("Error opening file..\n");  
	            exit(0); 
	        }
	    }
		char buffer[32];
		int i = 0;

		while(fgets(buffer,32, fp) != NULL) 
		{
			char * token = strtok(buffer, ":");
			token = strtok(NULL, " ");
		    *((int32_t *)&PROGRAM [i])= (int32_t) strtol(token, NULL, 16);
		    i += 4;
		}
	}


	void printMemory(){

		printf("STACK VIEWER:\n");
		printf(":  Address  :      Value :\n");
		//printf("%ld", MEM_SIZE);
		
		int32_t *data = (int32_t*)STACK_TOP;
		for (int i = 32; i > 0; i--)
		{
			printf(": %p  :   %08x :\n", data, *data--);
			
		}

		for(int i = 0; i < 32; i++)
		{
			printf("R%d = %d\n", i, reg_file[i]);
		}
	}



	void memory_init(int32_t stackAddr, int32_t startAddr, char* fileName)
	{
		MEMORY = (int8_t *) malloc(sizeof(int8_t)*MEM_SIZE);
		PROGRAM = &MEMORY[startAddr];
		STACK_TOP = &MEMORY[stackAddr-4];
		System_reset();
		reg_file[0] = 0;
		reg_file[2] = stackAddr-4; 
		load_program(fileName);
		PC = 0;	
	}

	void exit_program()
	{
		printMemory();
		exit(0);
	}


#endif

