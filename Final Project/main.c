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
		*extended_value = offset & 0x0000ffff;;
	}
}

/* Write Register *//* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg){
	if(RegWrite==1 && MemtoReg==1) {
		Reg[RegDst] = memdata;
	} else if(RegWrite==1 && MemtoReg==0) {
		Reg[RegDst] = ALUresult;
	}
}

/* PC update *//* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC){
	*PC=*PC+4;

	if(Branch==1 && Zero==1) {
		*PC=extended_value;
	}

	if(Jump==1) {
		jsec<<2;
		*PC<<28;
		*PC = (*PC) | (*PC>>28);
	}
}
