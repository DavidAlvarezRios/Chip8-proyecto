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
        
        if(mac.pc >= MEMSIZE)
            mac.pc = 0x200;

        // Process instruction from opcode
        
        // example 6a02 -> nnn = a02 kk = 02 n = 2 x = a y = 0

        uint16_t nnn = opcode & 0x0FFF;
        uint8_t kk = opcode & 0x00FF;
        uint8_t n = opcode & 0x000F;
        uint8_t x = (opcode >> 8) & 0x000F;
        uint8_t y = (opcode >> 4) & 0x000F;

        uint8_t patata = (opcode >> 12);
        
        switch (patata) {
            
            case 0:
                if(opcode == 0x00E0){
                    printf("CLS");
                }else if(opcode == 0x00EE){
                    printf("RET");
                }

                break;
            
            case 1:
                printf("JP %x", nnn);
                break;
            
            case 2:
                printf("CALL %x", nnn);
                break;

            case 3:
                printf("SE V%x, %x", x, kk);
                break;

            case 4:
                printf("SNE V%x, %x", x, kk);
                break;

            case 5:
                printf("SE V%x, V%x", x, y);
                break;

            case 6:
                printf("LD V%x, %x", x, kk);
                break;

            case 7:
                printf("ADD V%x, %x", x, kk);
                break;

            case 8:
                
                switch (n) {
                    
                    case 0:
                        printf("LD V%x, V%x", x, y);
                        break;

                    case 1:
                        printf("OR V%x, V%x", x, y);
                        break;
               
               
                    case 2:
                        printf("AND V%x, V%x", x, y);
                        break;
               
               
                    case 3:
                        printf("XOR V%x, V%x", x, y);
                        break;
               
               
                    case 4:
                        printf("ADD V%x, V%x", x, y);
                        break;
                    
                    
                    case 5:
                        printf("SUB V%x, V%x", x, y);
                        break;
               

                    case 6:
                        printf("SHR V%x, V%x", x, y);
                        break;
               

                    case 7:
                        printf("SUBN V%x, V%x", x, y);
                        break;
               
                    case 0XE:
                        printf("SHL V%x, V%x", x, y);
                        break;
                                              
                                 
               }
                
                break;
               
            case 0XA:
                printf("LD I, %x", nnn);
                break;

            case 0xB:
                printf("JP V0, %x", nnn);
                break;

            case 0xC:
                printf("RND V%x, %x", x, kk);
                break;

            case 0xD:
                printf("DRW V%x, V%x, %x", x, y, n);
                break;

            case 0xE:
                if( kk == 95 ){
                    printf("SKP V%x", x);
                }else if( kk == 0xA1 ){
                    printf("SKNP V%x", x);
                }

                break;

            case 0xF:
                switch (kk) {
                    
                    case 0x07:
                        printf("LD V%x, DT", x);
                        break;
                    case 0x0A:
                        printf("LD V%x, k", x);
                        break;
                    case 0x15:
                        printf("LD DT, V%x", x);
                        break;
                    case 0x18:
                        printf("LD ST, V%x", x);
                        break;
                    case 0x1E:
                        printf("ADD I, V%x", x);
                        break;
                    case 0x29:
                        printf("LD F, V%x", x);
                        break;
                    case 0x33:
                        printf("LD B, V%x", x);
                        break;
                    case 0x55:
                        printf("LD I, V%x", x);
                        break;
                    case 0x65:
                        printf("LD V%x, I", x);
                        break;
                    
                }
                break;

            default:
                printf("pene");
                break;


        }

        printf("\n");
        // Control


    }
    
    
    
    
    
    
    
    //printf("Hello World \n");
    return 0;
}
