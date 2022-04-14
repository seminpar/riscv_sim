
/*  Instruction decoder header file  */

#ifndef DECODER_H
#define DECODER_H


	#include "constant.h"
	#include "MMU.h"


	static void Reg_decoder(int32_t data, bool debug)
	{
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
				IR.immediate|= ((data & 0x000FF000) >> 10) | ((data & 0x7FE0000) >> 21);
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

	void IR_decoder(int32_t IR_data, bool debug)
	{
		Reg_decoder(IR_data, debug);
		switch(IR.opcode)
		{
		    case LUI:
				reg_file[IR.rd] = IR.immediate;
				if(debug) printf("%02x : %08x  : LUI R%d, %d\n", PC, IR_data, IR.rd, IR.immediate);
				break;

			case AUIPC:
				reg_file[IR.rd] = IR.immediate + PC;
				if(debug) printf("%02x : %08x  : AUIPC R%d, %d\n", PC, IR_data, IR.rd, IR.immediate);
				break;

			case JAL:
				reg_file[IR.rd] = PC + 4;
				PC += IR.immediate;
				if(debug) printf("%02x : %08x  : JAL R%d, %d\n", PC, IR_data, IR.rd, IR.immediate);
				break;

			case JALR:
				if(debug) printf("%02x : %08x  : JALR R%d, %d(R%d)\n", PC, IR_data, IR.rd, IR.immediate, IR.rs1);
				if(reg_file[IR.rs1] == 0) 
				{
					exit_program();
				}
				reg_file[IR.rd] = PC + 4;
				PC = reg_file[IR.rs1] + IR.immediate;
				break;

	        case BRANCH:
				switch(IR.funct3){
					case BEQ:
						if(reg_file[IR.rs1] == reg_file[IR.rs2])
							PC += IR.immediate;

	                    if(debug) printf("%02x : %08x  : BEQ R%d, R%d, %d\n", PC, IR_data, IR.rs1, IR.rs2, IR.immediate);
						break;

					case BNE:
						if(reg_file[IR.rs1] != reg_file[IR.rs2])
							PC += IR.immediate;
	                    if(debug) printf("%02x : %08x  : BNE R%d, R%d, %d\n", PC, IR_data, IR.rs1, IR.rs2, IR.immediate);
						break;

					case BLT:
						if(reg_file[IR.rs1] <= reg_file[IR.rs2])
							PC += IR.immediate;
	                    if(debug) printf("%02x : %08x  : BLT R%d, R%d, %d\n", PC, IR_data, IR.rs1, IR.rs2, IR.immediate);
						break;

					case BGE:
						if(reg_file[IR.rs1] >= reg_file[IR.rs2])
							PC += IR.immediate;
	                    if(debug) printf("%02x : %08x  : BGE R%d, R%d, %d\n", PC, IR_data, IR.rs1, IR.rs2, IR.immediate);
						break;

					case BLTU:
						if((unsigned)reg_file[IR.rs1] <= (unsigned)reg_file[IR.rs2])
							PC += IR.immediate;
	                    if(debug) printf("%02x : %08x  : BLTU R%d, R%d, %d\n", PC, IR_data, IR.rs1, IR.rs2, IR.immediate);
						break;

					case BGEU:
						if((unsigned)reg_file[IR.rs1] >= (unsigned)reg_file[IR.rs2])
							PC += IR.immediate;
	                    if(debug) printf("%02x : %08x  : BGEU R%d, R%d, %d\n", PC, IR_data, IR.rs1, IR.rs2, IR.immediate);
						break;
					default:
						break;
				}
				break;

	        case LOAD:
				switch(IR.funct3)
				{
					case LW:
						reg_file[IR.rd] = *((int32_t *)&MEMORY[reg_file[IR.rs1] + IR.immediate]);
						if(debug) printf("%02x : %08x  : LW R%d, %d(R%d)\n", PC, IR_data, IR.rd, IR.immediate, IR.rs1);
						break;

					case LH:
						reg_file[IR.rd] = *((int16_t *)&MEMORY[reg_file[IR.rs1] + IR.immediate]);
						if(debug) printf("%02x : %08x  : LH R%d, %d(R%d)\n", PC, IR_data, IR.rd, IR.immediate, IR.rs1);
						break;

					case LHU:
						reg_file[IR.rd] = (unsigned) *((uint16_t *)&MEMORY[reg_file[IR.rs1] + IR.immediate]);
						if(debug) printf("%02x : %08x  : LHU R%d, %d(R%d)\n", PC, IR_data, IR.rd, IR.immediate, IR.rs1);
						break;

					case LB:
						reg_file[IR.rd] = MEMORY[reg_file[IR.rs1] + IR.immediate];
						if(debug) printf("%02x : %08x  : LB R%d, %d(R%d)\n", PC, IR_data, IR.rd, IR.immediate, IR.rs1);
						break;

					case LBU:
						reg_file[IR.rd] = (unsigned) MEMORY[reg_file[IR.rs1] + IR.immediate];
						if(debug) printf("%02x : %08x  : LBU R%d, %d(R%d)\n", PC, IR_data, IR.rd, IR.immediate, IR.rs1);
						break;

					default:
						break;
				}
				break;

	        case STORE:
				switch(IR.funct3)
				{
					case SW:
						*((int32_t *)&MEMORY[reg_file[IR.rs1] + IR.immediate]) = reg_file[IR.rs2];
					    if(debug) printf("%02x : %08x  : SW R%d, %d(R%d)\n", PC, IR_data, IR.rs2, IR.immediate, IR.rs1);
						break;
					case SH:
						*((int16_t *)&MEMORY[reg_file[IR.rs1] + IR.immediate]) = (int16_t)(reg_file[IR.rs2]);
						if(debug) printf("%02x : %08x  : SH R%d, %d(R%d)\n", PC, IR_data, IR.rs2, IR.immediate, IR.rs1);
						break;
					case SB:
						MEMORY[reg_file[IR.rs1] + IR.immediate] = (int8_t)(reg_file[IR.rs2]);
						if(debug) printf("%02x : %08x  : SB R%d, %d(R%d)\n", PC, IR_data, IR.rs2, IR.immediate, IR.rs1);
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
							reg_file[IR.rd] = reg_file[IR.rs1] + reg_file[IR.rs2];
							if(debug) printf("%02x : %08x  : ADD R%d, R%d, %d\n", PC, IR_data, IR.rd, IR.rs1, IR.rs2);
						}
			 			else if(IR.funct7 == 0x20)
						{
							reg_file[IR.rd] = reg_file[IR.rs1] - reg_file[IR.rs2];
							if(debug) printf("%02x : %08x  : SUB R%d, R%d, %d\n", PC, IR_data, IR.rd, IR.rs1, IR.rs2);
						}
			 			break;

					case SLL:
						reg_file[IR.rd] = reg_file[IR.rs1] << (reg_file[IR.rs2] & 0x0000001F);
						if(debug) printf("%02x : %08x  : SLL R%d, R%d, %d\n", PC, IR_data, IR.rd, IR.rs1, IR.rs2);
						break;

					case SRL_OR_SRA:
					    if(IR.funct7 == 0x00)
	                    {
	                       reg_file[IR.rd] = reg_file[IR.rs1] >> (reg_file[IR.rs2] & 0x0000001F);
	                       if(debug) printf("%02x : %08x  : SRL R%d, R%d, %d\n", PC, IR_data, IR.rd, IR.rs1, IR.rs2);
	                    }
						else if(IR.funct7 == 0x20)
	                    {
	                        reg_file[IR.rd] = reg_file[IR.rs1] >> (reg_file[IR.rs2] & 0x0000001F);
	                        if(debug) printf("%02x : %08x  : SRA R%d, R%d, %d\n", PC, IR_data, IR.rd, IR.rs1, IR.rs2);
	                    }
						break;

					case SLT:
						reg_file[IR.rd] = (reg_file[IR.rs1] < reg_file[IR.rs2]) ? 1 : 0;
						if(debug) printf("%02x : %08x  : SLT R%d, R%d, %d\n", PC, IR_data, IR.rd, IR.rs1, IR.rs2);
						break;

					case SLTU:
						reg_file[IR.rd] = ((unsigned)reg_file[IR.rs1] < (unsigned) reg_file[IR.rs2]) ? 1 : 0;
						if(debug) printf("%02x : %08x  : SLTU R%d, R%d, %d\n", PC, IR_data, IR.rd, IR.rs1, IR.rs2);
						break;

					case XOR:
						reg_file[IR.rd] = reg_file[IR.rs1] ^ reg_file[IR.rs2];
						if(debug) printf("%02x : %08x  : XOR R%d, R%d, %d\n", PC, IR_data, IR.rd, IR.rs1, IR.rs2);
						break;


					case OR:
						reg_file[IR.rd] = reg_file[IR.rs1] | reg_file[IR.rs2];
						if(debug) printf("%02x : %08x  : OR R%d, R%d, %d\n", PC, IR_data, IR.rd, IR.rs1, IR.rs2);
						break;

					case AND:
						reg_file[IR.rd] = reg_file[IR.rs1] & reg_file[IR.rs2];
						if(debug) printf("%02x : %08x  : AND R%d, R%d, %d\n", PC, IR_data, IR.rd, IR.rs1, IR.rs2);
						break;
					default:
						break;
				}
				break;

	        case ALU_I:
				switch(IR.funct3){
					case ADD_or_SUB:
	                    reg_file[IR.rd] = reg_file[IR.rs1] + IR.immediate;
	                    if(debug) printf("%02x : %08x  : ADDI R%d, R%d, %d\n", PC, IR_data, IR.rd, IR.rs1, IR.immediate);
			 			break;

					case SLL:
						reg_file[IR.rd] = reg_file[IR.rs1] << IR.shamt;
						if(debug) printf("%02x : %08x  : SLLI R%d, R%d, %d\n", PC, IR_data, IR.rd, IR.rs1, IR.immediate);
						break;

					case SRL_OR_SRA:
					    if (IR.funct7 == 0x00)
						{
	                        reg_file[IR.rd] = reg_file[IR.rs1] >> IR.shamt;
	                        if(debug) printf("%02x : %08x  : SRLI R%d, R%d, %d\n", PC, IR_data, IR.rd, IR.rs1, IR.immediate);
						}
	                    else if (IR.funct7 == 0x20)
						{
	                        reg_file[IR.rd] = reg_file[IR.rs1] >> IR.shamt;
	                        if(debug) printf("%02x : %08x  : SRAI R%d, R%d, %d\n", PC, IR_data, IR.rd, IR.rs1, IR.immediate);
						}
						break;

					case SLT:
						reg_file[IR.rd] = (reg_file[IR.rs1] < IR.immediate) ? 1 : 0;
						if(debug) printf("%02x : %08x  : SLTI R%d, R%d, %d\n", PC, IR_data, IR.rd, IR.rs1, IR.immediate);
						break;

					case SLTU:
						reg_file[IR.rd] = ((unsigned)reg_file[IR.rs1] < (unsigned) IR.immediate) ? 1 : 0;
						if(debug) printf("%02x : %08x  : SLTUI R%d, R%d, %d\n", PC, IR_data, IR.rd, IR.rs1, IR.immediate);
						break;

					case XOR:
						reg_file[IR.rd] = reg_file[IR.rs1] ^ IR.immediate;
						if(debug) printf("%02x : %08x  : XORI R%d, R%d, %d\n", PC, IR_data, IR.rd, IR.rs1, IR.immediate);
						break;


					case OR:
						reg_file[IR.rd] = reg_file[IR.rs1] | IR.immediate;
						if(debug) printf("%02x : %08x  : ORI R%d, R%d, %d\n", PC, IR_data, IR.rd, IR.rs1, IR.immediate);
						break;

					case AND:
						reg_file[IR.rd] = reg_file[IR.rs1] & IR.immediate;
						if(debug) printf("%02x : %08x  : ANDI R%d, R%d, %d\n", PC, IR_data, IR.rd, IR.rs1, IR.immediate);
						break;

					default:
						break;
				}
				break;

	        default:
	            if(debug) printf("%02x : %08x  : LUI R%d, %d\n", PC, IR_data, IR.rd, IR.immediate);
		}
	}



	void run_program(bool debug)
	{
		PC = 0;
		int32_t *data;

		int done = 0;

		while(1)
		{
			data = (int32_t *)&PROGRAM[PC];
			IR_decoder(*data, debug);
			PC += 4;
		}

	}


#endif