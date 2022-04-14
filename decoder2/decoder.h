#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define opcode_bit_mask 0x0000007F
#define rd_bit_mask		0x00000F80
#define funct3_bit_mask 0x00007000
#define rs1_bit_mask	0x000F9000
#define rs2_bit_mask    0x01F00000
#define funct7_bit_mask 0xFE000000
#define shamt_bit_mask  0x01F00000

//from page 130
#define LUI		0x37
#define AUIPC	0x17
#define	JAL 	0x6F
#define JALR    0x67
#define BRANCH	0x63
#define LOAD	0x03
#define STORE	0x23
#define ALU_I 	0x13
#define ALU 	0x33
#define FENCE	0x0F
#define ECALL	0x73
#define EBREAK	0x73


//funct3: opcode use to specify ALU operation
#define ADD_or_SUB		0
#define SLL		1
#define SLT  	2
#define SLTU 	3
#define XOR 	4
#define SRL_OR_SRA 	5
#define OR 		6
#define AND     7

//funct3: opcode use to specify LOAD instruction
#define LB		0
#define LH		1
#define LW		2
#define LBU		4
#define LHU		5

//funct3: opcode use to specify STORE instruction
#define SB		0
#define SH		1
#define SW		2

//funct3: opcode use to specify BRANCH instruction
#define BEQ		0
#define BNE		1
#define BLT		4
#define BGE		5
#define BLTU	6
#define BGEU	7




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

int reg_file[32];
int PC;

int32_t *p32;
int16_t *p16;
int8_t *p8;

int byteOffset;
int dataWordPosition;
int temp;

int memory[16384][2];       // Memory


static void Reg_decoder(int32_t IR_data[][2], bool debug)
{
	int32_t data = IR_data[PC/4][1];
	
	IR.opcode = (data & opcode_bit_mask);
	IR.rd  = ((data & rd_bit_mask) >> 7);
	IR.rs1 = ((data & rs1_bit_mask) >> 15);
	IR.rs2 = ((data & rs2_bit_mask) >> 20);
	IR.funct3 = ((data & funct3_bit_mask) >> 12);
	IR.funct7 = ((unsigned)(data & funct7_bit_mask) >> 25);
	IR.shamt = ((data & shamt_bit_mask) >> 20);
	
	switch(IR.opcode)
	{
		case LUI:
			IR.immediate = data & 0xFFFFF000;
			break;

		case AUIPC:
			IR.immediate = data & 0xFFFFF000;
			break;

		case JAL:
			IR.immediate = ((data & 0x80000000) >> 11) | (data & 0x000FF000);
			IR.immediate|= ((data & 0x000FF000) >> 10) | ((data & 0x7FE00000) >> 21);
			break;

		case JALR:
			IR.immediate = ((data & 0xFFF00000) >> 20);
			break;

		case BRANCH:
			IR.immediate = ((data & 0x80000000) >> 20)| ((data & 0x7E000000) >> 24);
			IR.immediate|= ((data & 0x00000080) << 4) | ((data & 0x00000F00) >> 7);
			break;

		case LOAD:
			IR.immediate = (data  >> 20);
			break;

		case STORE:
			IR.immediate = ((data  >> 20) & 0xFFFFFFE0) | ((data & 0x00000F80) >> 7);
			break;

		case ALU_I:
			IR.immediate = (data  >> 20);
			break;

		default:
			IR.immediate = 0;
			break;
	}
}

void IR_decoder(int32_t data[][2], bool debug)
{
	int32_t IR_data = data[PC/4][1];
	int IR_PC = data[PC/4][0];
	
	Reg_decoder(data, debug);

	switch(IR.opcode)
	{
	    case LUI:
			PC += 4;
			reg_file[IR.rd] = IR.immediate;
			if(debug) printf("%4x  : %08x  : LUI R%d, %d\n", IR_PC, IR_data, IR.rd, IR.immediate);
			break;

		case AUIPC:
			reg_file[IR.rd] = IR.immediate + PC;
			PC += 4;
			if(debug) printf("%4x  : %08x  : AUIPC R%d, %d\n", IR_PC, IR_data, IR.rd, IR.immediate);
			break;

		case JAL:
			reg_file[1] = PC + 4;		//R1 = ra
			PC += IR.immediate;
			if(debug) printf("%4x  : %08x  : JAL R%d, %d\n", IR_PC, IR_data, IR.rd, IR.immediate);
			break;

		case JALR:
			reg_file[1] = PC + 4;		//R1 = ra
			PC = reg_file[IR.rs1] + IR.immediate;
			if(debug) printf("%4x  : %08x  : JALR R%d, %d(R%d)\n", IR_PC, IR_data, IR.rd, IR.immediate, IR.rs1);
			break;

        case BRANCH:
			switch(IR.funct3){
				case BEQ:
					if(reg_file[IR.rs1] == reg_file[IR.rs2])
						PC += IR.immediate;

                    if(debug) printf("%4x  : %08x  : BEQ R%d, R%d, %d\n", IR_PC, IR_data, IR.rs1, IR.rs2, IR.immediate);
					break;

				case BNE:
					if(reg_file[IR.rs1] != reg_file[IR.rs2])
						PC += IR.immediate;
                    if(debug) printf("%4x  : %08x  : BNE R%d, R%d, %d\n", IR_PC, IR_data, IR.rs1, IR.rs2, IR.immediate);
					break;

				case BLT:
					if(reg_file[IR.rs1] <= reg_file[IR.rs2])
						PC += IR.immediate;
                    if(debug) printf("%4x  : %08x  : BLT R%d, R%d, %d\n", IR_PC, IR_data, IR.rs1, IR.rs2, IR.immediate);
					break;

				case BGE:
					if(reg_file[IR.rs1] >= reg_file[IR.rs2])
						PC += IR.immediate;
                    if(debug) printf("%4x  : %08x  : BGE R%d, R%d, %d\n", IR_PC, IR_data, IR.rs1, IR.rs2, IR.immediate);
					break;

				case BLTU:
					if((unsigned)reg_file[IR.rs1] <= (unsigned)reg_file[IR.rs2])
						PC += IR.immediate;
                    if(debug) printf("%4x  : %08x  : BLTU R%d, R%d, %d\n", IR_PC, IR_data, IR.rs1, IR.rs2, IR.immediate);
					break;

				case BGEU:
					if((unsigned)reg_file[IR.rs1] >= (unsigned)reg_file[IR.rs2])
						PC += IR.immediate;
                    if(debug) printf("%4x  : %08x  : BGEU R%d, R%d, %d\n", IR_PC, IR_data, IR.rs1, IR.rs2, IR.immediate);
					break;
				default:
					break;
			}
			break;

        case LOAD:
			switch(IR.funct3)
			{
				case 3:
				case LW:
					PC += 4;
					reg_file[IR.rd] = data[(reg_file[IR.rs1] + IR.immediate) / 4][1];
					if(debug) printf("%4x  : %08x  : LW R%d, %d(R%d)\n", IR_PC, IR_data, IR.rd, IR.immediate, IR.rs1);
					break;

				case LH:		// load sign-extended half word
					PC += 4;
								// isolate and extend the sign bit
					temp = (data[(reg_file[IR.rs1] + IR.immediate) / 4][1] & 0x00008000) << 16;
								// OR the rest of the bits with the sign extended value
					reg_file[IR.rd] = temp | (data[(reg_file[IR.rs1] + IR.immediate) / 4][1] & 0x00007FFF);

					if(debug) printf("%4x  : %08x  : LH R%d, %d(R%d)\n", IR_PC, IR_data, IR.rd, IR.immediate, IR.rs1);
					break;

				case LHU:		// load unsigned half word
					PC += 4;
					reg_file[IR.rd] = (data[(reg_file[IR.rs1] + IR.immediate) / 4][1] & 0x0000FFFF);
					if(debug) printf("%4x  : %08x  : LHU R%d, %d(R%d)\n", IR_PC, IR_data, IR.rd, IR.immediate, IR.rs1);
					break;

				case LB:		// load sign-extended byte
					PC += 4;
								//isolate and extend sign bit
					temp = (data[(reg_file[IR.rs1] + IR.immediate) / 4][1] & 0x00000080) << 24;
								// OR the rest of the bits with the sign extended value
					reg_file[IR.rd] = temp | (data[(reg_file[IR.rs1] + IR.immediate) / 4][1] & 0x0000007F);
					if(debug) printf("%4x  : %08x  : LB R%d, %d(R%d)\n", IR_PC, IR_data, IR.rd, IR.immediate, IR.rs1);
					break;

				case LBU:
					PC += 4;
					reg_file[IR.rd] = (data[(reg_file[IR.rs1] + IR.immediate) / 4][1] & 0x000000FF);
					if(debug) printf("%4x  : %08x  : LBU R%d, %d(R%d)\n", IR_PC, IR_data, IR.rd, IR.immediate, IR.rs1);
					break;

				default:
					break;
			}
			break;

        case STORE:
			switch(IR.funct3)
			{
				case SW:
					
					PC += 4;
					// add immediate to the value in the register # indicted by rs1.
					// divide by 4 to align to the word

				    data[ ((reg_file[IR.rs1] + IR.immediate) / 4)][1] = IR.rs2;

					if(debug) printf("%4x  : %08x  : SW R%d, %d(R%d)\n", IR_PC, IR_data, IR.rs2, IR.immediate, IR.rs1);
					break;
				case SH:

					PC += 4;
					byteOffset = (reg_file[IR.rs1] + IR.immediate + 1) % 4;
					dataWordPosition = ((reg_file[IR.rs1] + IR.immediate) / 4);

					switch(byteOffset){
						case 0:
						case 1:
							//place in first half of the word
							data[dataWordPosition][1] &= 0xFFFF0000;
							temp = reg_file[IR.rs2] & 0x0000FFFF;
							data[dataWordPosition][1] |= temp;
							break;
						case 2:
						case 3:
							//place in second half of the word
							data[dataWordPosition][1] &= 0x0000FFFF;
							temp = reg_file[IR.rs2] & 0xFFFF0000;
							data[dataWordPosition][1] |= temp;
							break;
					}
					
					if(debug) printf("%4x  : %08x  : SH R%d, %d(R%d)\n", IR_PC, IR_data, IR.rs2, IR.immediate, IR.rs1);
					break;
				case SB:

					PC += 4;
					byteOffset = (reg_file[IR.rs1] + IR.immediate + 1) % 4;
					dataWordPosition = ((reg_file[IR.rs1] + IR.immediate) / 4);

					switch(byteOffset){
						case 0:
							//place in lowest order byte of the word
							data[dataWordPosition][1] &= 0xFFFFFF00;
							temp = reg_file[IR.rs2] & 0x000000FF;
							data[dataWordPosition][1] |= temp;
							break;
						case 1:
							//place in second lowest order byte of the word
							data[dataWordPosition][1] &= 0xFFFF00FF;
							temp = reg_file[IR.rs2] & 0x0000FF00;
							data[dataWordPosition][1] |= temp;
							break;
						case 2:
							//place in third lowest order byte of the word
							data[dataWordPosition][1] &= 0xFF00FFFF;
							temp = reg_file[IR.rs2] & 0x00FF0000;
							data[dataWordPosition][1] |= temp;
							break;
						case 3:
							//place in high order byte of the word
							data[dataWordPosition][1] &= 0x00FFFFFF;
							temp = reg_file[IR.rs2] & 0xFF000000;
							data[dataWordPosition][1] |= temp;
							break;
					}

					if(debug) printf("%4x  : %08x  : SB R%d, %d(R%d)\n", IR_PC, IR_data, IR.rs2, IR.immediate, IR.rs1);
					break;
				default:
					break;
			}
			break;

        case ALU:
			switch(IR.funct3){
				case ADD_or_SUB:
					if (IR.funct7 == 0x00)
					{
						PC += 4;
						reg_file[IR.rd] = reg_file[IR.rs1] + reg_file[IR.rs2];
						if(debug) printf("%4x  : %08x  : ADD R%d, R%d, %d\n", IR_PC, IR_data, IR.rd, IR.rs1, IR.rs2);
					}
		 			else if(IR.funct7 == 0x20)
					{
						PC += 4;
						reg_file[IR.rd] = reg_file[IR.rs1] - reg_file[IR.rs2];
						if(debug) printf("%4x  : %08x  : SUB R%d, R%d, %d\n", IR_PC, IR_data, IR.rd, IR.rs1, IR.rs2);
					}
		 			break;

				case SLL:
					PC += 4;
					reg_file[IR.rd] = reg_file[IR.rs1] << (reg_file[IR.rs2] & 0x0000001F);
					if(debug) printf("%4x  : %08x  : SLL R%d, R%d, %d\n", IR_PC, IR_data, IR.rd, IR.rs1, IR.rs2);
					break;

				case SRL_OR_SRA:
				    if(IR.funct7 == 0x00)
                    {
						PC += 4;
                       	reg_file[IR.rd] = reg_file[IR.rs1] >> (reg_file[IR.rs2] & 0x0000001F);
                       	if(debug) printf("%4x  : %08x  : SRL R%d, R%d, %d\n", IR_PC, IR_data, IR.rd, IR.rs1, IR.rs2);
                    }
					else if(IR.funct7 == 0x20)
                    {
						PC += 4;
                        reg_file[IR.rd] = reg_file[IR.rs1] >> (reg_file[IR.rs2] & 0x0000001F);
                        if(debug) printf("%4x  : %08x  : SRA R%d, R%d, %d\n", IR_PC, IR_data, IR.rd, IR.rs1, IR.rs2);
                    }
					break;

				case SLT:
					PC += 4;
					reg_file[IR.rd] = (reg_file[IR.rs1] < reg_file[IR.rs2]) ? 1 : 0;
					if(debug) printf("%4x  : %08x  : SLT R%d, R%d, %d\n", IR_PC, IR_data, IR.rd, IR.rs1, IR.rs2);
					break;

				case SLTU:
					PC += 4;
					reg_file[IR.rd] = ((unsigned)reg_file[IR.rs1] < (unsigned) reg_file[IR.rs2]) ? 1 : 0;
					if(debug) printf("%4x  : %08x  : SLTU R%d, R%d, %d\n", IR_PC, IR_data, IR.rd, IR.rs1, IR.rs2);
					break;

				case XOR:
					PC += 4;
					reg_file[IR.rd] = reg_file[IR.rs1] ^ reg_file[IR.rs2];
					if(debug) printf("%4x  : %08x  : XOR R%d, R%d, %d\n", IR_PC, IR_data, IR.rd, IR.rs1, IR.rs2);
					break;


				case OR:
					PC += 4;
					reg_file[IR.rd] = reg_file[IR.rs1] | reg_file[IR.rs2];
					if(debug) printf("%4x  : %08x  : OR R%d, R%d, %d\n", IR_PC, IR_data, IR.rd, IR.rs1, IR.rs2);
					break;

				case AND:
					PC += 4;
					reg_file[IR.rd] = reg_file[IR.rs1] & reg_file[IR.rs2];
					if(debug) printf("%4x  : %08x  : AND R%d, R%d, %d\n", IR_PC, IR_data, IR.rd, IR.rs1, IR.rs2);
					break;
				default:
					break;
			}
			break;

        case ALU_I:
            //if(debug) printf("%08x  : ALU funct3: %d funct7: %d\n", IR_data, IR.funct3, IR.funct7);
			switch(IR.funct3){
				case ADD_or_SUB:
					PC += 4;
                    reg_file[IR.rd] = reg_file[IR.rs1] + IR.immediate;
                    if(debug) printf("%4x  : %08x  : ADDI R%d, R%d, %d\n", IR_PC, IR_data, IR.rd, IR.rs1, IR.immediate);
		 			break;

				case SLL:
					PC += 4;
					reg_file[IR.rd] = reg_file[IR.rs1] << IR.shamt;
					if(debug) printf("%4x  : %08x  : SLLI R%d, R%d, %d\n", IR_PC, IR_data, IR.rd, IR.rs1, IR.immediate);
					break;

				case SRL_OR_SRA:
				    if (IR.funct7 == 0x00)
					{
						PC += 4;
                        reg_file[IR.rd] = reg_file[IR.rs1] >> IR.shamt;
                        if(debug) printf("%4x  : %08x  : SRLI R%d, R%d, %d\n", IR_PC, IR_data, IR.rd, IR.rs1, IR.immediate);
					}
                    else if (IR.funct7 == 0x20)
					{
						PC += 4;
                        reg_file[IR.rd] = reg_file[IR.rs1] >> IR.shamt;
                        if(debug) printf("%4x  : %08x  : SRAI R%d, R%d, %d\n", IR_PC, IR_data, IR.rd, IR.rs1, IR.immediate);
					}
					break;

				case SLT:
					PC += 4;
					reg_file[IR.rd] = (reg_file[IR.rs1] < IR.immediate) ? 1 : 0;
					if(debug) printf("%4x  : %08x  : SLTI R%d, R%d, %d\n", IR_PC, IR_data, IR.rd, IR.rs1, IR.immediate);
					break;

				case SLTU:
					PC += 4;
					reg_file[IR.rd] = ((unsigned)reg_file[IR.rs1] < (unsigned) IR.immediate) ? 1 : 0;
					if(debug) printf("%4x  : %08x  : SLTUI R%d, R%d, %d\n", IR_PC, IR_data, IR.rd, IR.rs1, IR.immediate);
					break;

				case XOR:
					PC += 4;
					reg_file[IR.rd] = reg_file[IR.rs1] ^ IR.immediate;
					if(debug) printf("%4x  : %08x  : XORI R%d, R%d, %d\n", IR_PC, IR_data, IR.rd, IR.rs1, IR.immediate);
					break;


				case OR:
					PC += 4;
					reg_file[IR.rd] = reg_file[IR.rs1] | IR.immediate;
					if(debug) printf("%4x  : %08x  : ORI R%d, R%d, %d\n", IR_PC, IR_data, IR.rd, IR.rs1, IR.immediate);
					break;

				case AND:
					PC += 4;
					reg_file[IR.rd] = reg_file[IR.rs1] & IR.immediate;
					if(debug) printf("%4x  : %08x  : ANDI R%d, R%d, %d\n", IR_PC, IR_data, IR.rd, IR.rs1, IR.immediate);
					break;

				default:
					break;
			}
			break;

        default:
            if(debug) printf("%4x  : %08x  : LUI R%d, %d\n", IR_PC, IR_data, IR.rd, IR.immediate);
	}

	// If we encounter JR RA (JALR x0, 0(ra)) 
	// return because we're done decoding.
	// This will stop the recursion.
	if(IR.opcode == JALR && IR.rd == 0 && IR.rs1 == 1 && IR.immediate == 0 && IR.funct3 == 0){// && reg_file[IR.rs1] == 0) {
		printf("		    END OF PROGRAM\n");
		return;
	}

	// RECURSION!
	IR_decoder(memory, debug);

}

void printMemory(int32_t data[][2]){

	printf("STACK VIEWER:\n");
	printf(":  Address  :      Value :\n");

	for (int i = 16383; i >= 0; i--){
		if(data[i][1] != 0){
			printf(": %8d  :   %08x :\n", (i*4), data[i][1]);
		}
		
	}
}
