//CDA Group Project
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "spimcore.h"

void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{
	switch ((int) ALUControl)
	{
        case 0: //000
            *ALUresult = A + B;break;

        case 1: //001
            *ALUresult = A - B;break;
        
        case 2: //010
        if ((signed)A < (signed)B)
            *ALUresult = 1;
        else
            *ALUresult = 0;break;
        
	case 3: //011
        if (A < B)
	*ALUresult = 1;
        else
	*ALUresult = 0;break;
		
	case 4: //100
           *ALUresult = A & B;break;

        case 5: //101
	*ALUresult = A | B;break;

        case 6: //110
            *ALUresult = B << 16;break;

        case 7: //111
            *ALUresult = ~A;break;
	}
	
	if(*ALUresult == 0)
		*Zero = 1;
	else
		*Zero = 0;
	
}

int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
	unsigned Mem_Index = PC >> 2;
	
	if (PC % 4 == 0) {
        *instruction = Mem[Mem_Index];
        return 0;
    } else
	    return 1;	
}

void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
	unsigned rPartition	= 0x1f; 
	unsigned functopPartition = 0x0000003f; 
	unsigned offsetPartition = 0x0000ffff; 
	unsigned jsecPartition = 0x03ffffff;

	*op	= (instruction >> 26) & functopPartition;// instruction [31-26] 
	*r1	= (instruction >> 21) & rPartition;      // instruction [25-21] 
	*r2	= (instruction >> 16) & rPartition;      // instruction [20-16] 
	*r3	= (instruction >> 11) & rPartition;      // instruction [15-11] 
	*funct = instruction & functopPartition;     // instruction [5-0]   
	*offset	= instruction & offsetPartition;     // instruction [15-0]  
	*jsec = instruction & jsecPartition;         // instruction [25-0]  
	
}

/* Read Register *//* 5 Points */
void read_register(unsigned r1, unsigned r2, unsigned *Reg, unsigned *data1, unsigned *data2) {
    *data1 = Reg[r1];
    *data2 = Reg[r2];
}

/* Sign Extend *//* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value){
	unsigned temp = offset >> 15;
	temp >> 15;
	if (temp == 1) {
		*extended_value = offset | 0xffff0000;
	} else {
		*extended_value = offset & 0x0000ffff;
	}
}

/* Write Register *//* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg){
	if(RegWrite==1 && MemtoReg==1) {
		Reg[RegDst] = memdata;
	} else if(RegWrite==1 && MemtoReg==0) {
		Reg[RegDst] = memdata;
	}
}

/* PC update *//* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC){
	*PC=*PC+4;

	if(Branch==1 && Zero==1) {
		*PC= *PC+extended_value<<2;
	}

	if(Jump==1) {
		jsec<<2;
		*PC<<28;
		*PC = (*PC) | (*PC>>28);
	}
}

//instruct decoder
int instruction_decode(unsigned op,struct_controls *controls)
{
	controls->ALUOp=0;
	controls->MemWrite=0;
	controls->Branch=0;
	controls->ALUSrc=0;
	controls->Jump=0;
	controls->MemRead=0;
	controls->RegDst=0;
	controls->MemtoReg=0;
	controls->RegWrite=0;
	
	switch (op) 
	{
		case 0: // add, and, or, slt, sltu, sub
			controls->RegDst = 1; // multiplexer path 1
			controls->ALUOp = 7; //r-type instruction
			controls->RegWrite = 1; //enabled
			break;
                
        case 8: // addi
            controls->ALUSrc = 1;
            controls->RegWrite = 1;
            break;
                
        	case 4: // beq
                	controls->Branch = 1; 
                	controls->ALUOp = 1; //sub
                	controls->RegDst = 2; // doesnt matter
                	controls->MemtoReg = 2; // dummy fathead
                	break;
                
        	case 15: // lui
                	controls->ALUOp = 6; // move left_extended 16
                	controls->ALUSrc = 1; 
                	controls->RegWrite = 1; 
                	break;
                
	        case 35: // lw
	                controls->MemRead = 1;
	                controls->MemtoReg = 1;
	                controls->ALUSrc = 1;
	                controls->RegWrite = 1;
	                break;
                
	        case 10: // slti
	                controls->ALUOp = 2; // set <
	                controls->ALUSrc = 1;
	                controls->RegWrite = 1;
	                break;
	                
	        case 11: // sltiu
	                controls->ALUOp = 3; // set < unsigned
	                controls->ALUSrc = 1;
	                controls->RegWrite = 1;
	                break;
	                
	        case 43: // sw
	                controls->RegDst = 2; // dont matter
	                controls->MemtoReg = 2; // cringe
	                controls->MemWrite = 1;
	                controls->ALUSrc = 1;
	                break;
	                
	        case 2: // j
	                controls->Jump = 1;
	                break;
	                
	                default: return 1; // if halt occurs
	    } 
	    return 0; // if halt condition occurs
}

//ALU operations 
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
	unsigned char ALUControl = ALUOp;
    
	switch(ALUOp){
		case 0: case 1: case 2: case 3: case 6: //i types
			ALU(data1,data2,ALUOp,ALUresult,Zero);
			break;
            case 0x7:	//R-Type Instructions
			switch(funct){
				case 0x20:	// add
					ALUControl = 0x0;
                    break;
				case 0x24:	// and
					ALUControl = 0x4;
                    break;
				case 0x25:	// or
					ALUControl = 0x5;
                    break;
				case 0x2a:	// slt
					ALUControl = 0x2;
                    break;
				case 0x2b:	// sltu
					ALUControl = 0x3;
                    break;
				default:	// halt conditions
					return 1;
			}
	ALU(data1, data2, ALUControl, ALUresult, Zero);
	
	// no halt condition 
	return 0;
	}
	
// RW mem
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
	unsigned MemZero = 0x00000000;
	
	// if MemRead or MemWrite experience halt conditions
	if ((MemRead | MemWrite) && (ALUresult % 4 != 0 | (ALUresult >> 2) > 0xFFFF | (ALUresult >> 2) < MemZero))  
	{
		// if halt condition occurs
		return 1;
        }
        
        if (MemRead)
        {
        	// reads content of memory location addressed by ALUresult to memdata
        	*memdata = Mem[ALUresult >> 2];
        }
        
            if (MemWrite)
            {
                Mem[ALUresult >> 2] = data2;
            }
            
            // halt condition occurs
            return 0;
}
