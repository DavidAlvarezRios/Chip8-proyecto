#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>

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

    memset(machine->mem, 0, MEMSIZE);
    memset(machine->v, 0, 16);
    memset(machine->stack, 0, 16);

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



int main(int argc, char** argv)
{
    SDL_Window* win = SDL_CreateWindow("Chip8",
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        640, 320,
                                        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

    SDL_Renderer* rnd = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);


    int quit = 0;
    SDL_Event ev;

    while(!quit)
    {
        SDL_WaitEvent(&ev);
        if(ev.type == SDL_QUIT)
        {
            quit = 1;
            //break;
        }
            
    }


    SDL_DestroyRenderer(rnd);
    SDL_DestroyWindow(win);
    SDL_Quit();


/*
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

        // mac.pc = (mac.pc + 2) & 0xFFF;

        // Process instruction from opcode
        
        // example 6a02 -> nnn = a02 kk = 02 n = 2 x = a y = 0

        uint16_t nnn = opcode & 0x0FFF;
        uint8_t kk = opcode & 0x00FF;
        uint8_t n = opcode & 0x000F;
        uint8_t x = (opcode >> 8) & 0x000F;
        uint8_t y = (opcode >> 4) & 0x000F;

        uint8_t patata = (opcode >> 12);

        int i = 0;
        
        switch (patata) {
            
            case 0:
                if(opcode == 0x00E0){
                    printf("CLS");
                }else if(opcode == 0x00EE){
                    printf("RET");

                    mac.pc = mac.stack[mac.sp];
                    mac.sp = mac.sp - 1;
                    if(mac.sp < 0){
                        fprintf(stderr, "Stack pointer < 0\n");
                        exit(1);
                    }
                        

                }

                break;
            
            case 1:
                printf("JP %x", nnn);
                mac.pc = nnn;
                break;
            
            case 2:
                printf("CALL %x", nnn);
                mac.sp = mac.sp + 1;
                if(mac.sp >= 16){
                    fprintf(stderr, "Stack Pointer overflow\n");
                    exit(1);
                }
                mac.stack[mac.sp] = mac.pc;
                mac.pc = nnn;
                break;

            case 3:
                printf("SE V%x, %x", x, kk);
                if(mac.v[x] == kk)
                    mac.pc = mac.pc + 2;
                if(mac.pc >= MEMSIZE)
                    mac.pc = 0x200;

                break;

            case 4:
                printf("SNE V%x, %x", x, kk);
                if(mac.v[x] != kk)
                    mac.pc = mac.pc + 2;

                if(mac.pc >= MEMSIZE)
                    mac.pc = 0x200;
                break;

            case 5:
                printf("SE V%x, V%x", x, y);
                if(mac.v[x] == mac.v[y])
                    mac.pc = mac.pc + 2;

                if(mac.pc >= MEMSIZE)
                    mac.pc = 0x200;
                break;

            case 6:
                printf("LD V%x, %x", x, kk);
                mac.v[x] = kk;
                break;

            case 7:
                printf("ADD V%x, %x", x, kk);
                mac.v[x] = mac.v[x] + kk;
                break;

            case 8:
                
                switch (n) {
                    
                    case 0:
                        printf("LD V%x, V%x", x, y);
                        mac.v[x] = mac.v[y];
                        break;

                    case 1:
                        printf("OR V%x, V%x", x, y);
                        mac.v[x] = mac.v[x] | mac.v[y];
                        break;
               
               
                    case 2:
                        printf("AND V%x, V%x", x, y);
                        mac.v[x] = mac.v[x] & mac.v[y];
                        break;
               
               
                    case 3:
                        printf("XOR V%x, V%x", x, y);
                        mac.v[x] = mac.v[x] ^ mac.v[y];
                        break;
               
               
                    case 4:
                        printf("ADD V%x, V%x", x, y);
                        //set vf to 1 if overflow 0 otherwise
                        mac.v[0xF] = (mac.v[x] > mac.v[x] + mac.v[y]);

                        mac.v[x] = mac.v[x] + mac.v[y];
                        break;
                    
                    
                    case 5:
                        printf("SUB V%x, V%x", x, y);

                        if(mac.v[x] > mac.v[y])
                            mac.v[0xF] = 1;

                        mac.v[x] = mac.v[x] - mac.v[y];
                        break;
               

                    case 6:
                        printf("SHR V%x, V%x", x, y);

                        if(mac.v[x] && 1)
                            mac.v[0xF] = 1;
                        else
                            mac.v[0xF] = 0;

                        mac.v[x] = (mac.v[x] >> 1);

                        break;
               

                    case 7:
                        printf("SUBN V%x, V%x", x, y);

                        if(mac.v[y] > mac.v[x])
                            mac.v[0xF] = 1;
                        else
                            mac.v[0xF] = 0;

                        mac.v[x] -= mac.v[y];
                        
                        break;
               
                    case 0XE:
                        printf("SHL V%x, V%x", x, y);

                        if(mac.v[x] && 0x8)
                            mac.v[0xF] = 1;
                        else
                            mac.v[0xF] = 0;

                        mac.v[x] = (mac.v[x] << 1);

                        break;
                                              
                                 
               }
                
                break;
               
            case 9:
                printf("SNE V%x, V%x", x, y);

                if(mac.v[x] != mac.v[y]){
                    mac.pc = mac.pc + 2;
                    if(mac.pc >= MEMSIZE)
                        mac.pc = 0x200;
                }
            case 0XA:
                printf("LD I, %x", nnn);
                mac.I = nnn;
                break;

            case 0xB:
                printf("JP V0, %x", nnn);
                mac.pc = mac.v[0] + nnn;
                                                                
                break;

            case 0xC:
                printf("RND V%x, %x", x, kk);
                mac.v[x] = (rand() % 256) + kk;
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
                        mac.v[x] = mac.dt;
                        break;
                    case 0x0A:
                        printf("LD V%x, k", x);
                        //Stores value of key in Vx
                        break;
                    case 0x15:
                        printf("LD DT, V%x", x);
                        mac.dt = mac.v[x];
                        break;
                    case 0x18:
                        printf("LD ST, V%x", x);
                        mac.st = mac.v[x];
                        break;
                    case 0x1E:
                        printf("ADD I, V%x", x);
                        mac.I = mac.I + mac.v[x];
                        break;
                    case 0x29:
                        printf("LD F, V%x", x);
                        //TODO
                        break;
                    case 0x33:
                        printf("LD B, V%x", x);
                        break;
                    case 0x55:
                        printf("LD I, V%x", x);
                        for(i = 0; i < x; i++){
                            mac.mem[mac.I + i] = mac.v[x];
                            //mac.I = mac.I + 1;
                        }
                        break;
                    case 0x65:
                        printf("LD V%x, I", x);
                        for(i = 0; i < x; i++){
                            mac.v[x] = mac.mem[mac.I + i];
                            //mac.I = mac.I + 1;
                        }
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
    
    */
    
    return 0;
}

