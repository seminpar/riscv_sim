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

struct node {
        int address;
        int content;
        struct node* next;
        struct node* previous;
};

struct node* head = NULL;
struct node* tail = NULL;

bool isEmpty()
{
        return head == NULL;
}

struct node* getNode(int x, int y) {
        struct node* nNode = (struct node*)malloc(sizeof(struct node));
        nNode->address = x;
        nNode->content = y;
        nNode->previous = NULL;
        nNode->next = NULL;
        return nNode;
}

// inserts at head
void insert(int address, int content) {

        struct node* newNode = (struct node*) malloc(sizeof(struct node));

        newNode->address = address;

        newNode->content = content;

        if(isEmpty())
        {
                tail = newNode;
        }
        else
        {
                head->previous = newNode;
        }

        newNode->next = head; // Redirect to old head node

        head = newNode; // point to new head node

}

void Print() {
        struct node* temp = head;
        printf("Linked List: \n");
        while(temp != NULL) {
                printf("%d", temp->address);
                printf(": %d\n", temp->content);
                temp = temp->next;
        }
        printf("\n");
}

int main(int argc, char *argv[]) {

	bool debug = false;								//debug used for verbose mode
	int c, i = 0, startAddr = 0, stackAddr = 65535;
	char fileName[] = "program.mem", numStr[5];

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


        FILE *fptr;
        char *trace = NULL;
        size_t length = 0;
        ssize_t read;

        char grabbedString[50];

        if ((fptr = fopen(fileName, "r")) == NULL)
        {
                printf("Error reading in file\n");
                exit(1);
        }

        while ((read = getline(&trace, &length, fptr)) != -1)
        {

                char delim[] = " :";

                char *ptr = strtok(trace, delim);

                char destination[] = "";

                int target[2];

                int i = 0;

                while (ptr != NULL)
                {
                  strcat(destination, ptr);

                  target[i] = (int)strtol(ptr, NULL, 16);
                  i++;

                  ptr = strtok (NULL, delim);
                }
                
                insert(target[0], target[1]);
        }

        Print();

        return 0;
}
