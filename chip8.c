#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define MEMSIZE 4096

struct machine_t{

    uint8_t mem[MEMSIZE];   // Memory RAM of the chip

    uint8_t v[16];          // 16 general purpose registers
    uint16_t I;             // Address register
    
    uint8_t dt, st;         // Delay Timer and Sound timer

    uint16_t pc;            // Program Counter
    
    uint8_t sp;             // Stack Pointer
    uint16_t stack[16];     // The Stack of the processor. 16 subroutines max

};


void init_machine(struct machine_t* machine)
{
    machine->sp = machine->I = machine->dt = machine->st = 0;
    machine->pc = 0x200;
    
    for(int i = 0; i < MEMSIZE; i++)
    {
        machine->mem[i] = 0x00;    
    }
    
    for(int i = 0; i < 16; i++)
    {
        machine->v[i] = 0x00;
        machine->stack[i] = 0x00;
    }

}

void load_rom(struct machine_t* machine)
{
	FILE* fp = fopen("PONG", "r");
	if(fp == NULL)
	{
		fprintf(stderr, "Cannot open ROM\n");
        exit(1);
	}
    
    // We get the length of the file by going to the end, looking the bytes and back to the start.
    fseek(fp, 0, SEEK_END);
    int length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // In chip8 the programs start at 0x200.
    fread(machine->mem + 0x200, length, 1, fp);

	fclose(fp);
}

int main(int argc, char** argv){

    struct machine_t mac;
    init_machine(&mac);
    load_rom(&mac);
    
    int quit = 0; // Flag to shutdown the chip8

    while(!quit)
    {
        // Read opcode
        uint16_t opcode = (mac.mem[mac.pc] << 8) | mac.mem[mac.pc+1];
        mac.pc = mac.pc + 2;
    
        // Process instruction from opcode
        
        
        // Control


    }
    
    
    
    
    
    
    
    //printf("Hello World \n");
    return 0;
}
