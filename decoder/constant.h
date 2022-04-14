/*  constant.h - constant header file  */

#ifndef CONSTANT_H
#define CONSTANT_H

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

#endif