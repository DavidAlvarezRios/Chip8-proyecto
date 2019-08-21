#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL.h>

#define MEMSIZE 4096

char hexcodes[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

struct machine_t{

    uint8_t mem[MEMSIZE];   // Memory RAM of the chip

    uint8_t v[16];          // 16 general purpose registers
    uint16_t I;             // Address register
    
    uint8_t dt, st;         // Delay Timer and Sound timer

    uint16_t pc;            // Program Counter
    
    uint8_t sp;             // Stack Pointer
    uint16_t stack[16];     // The Stack of the processor. 16 subroutines max

    char screen[2048];      //Array to hold screen information. size -> 64 * 32 = 2048

};

void step_machine(struct machine_t* mac);

static void expansion(char* from, Uint32* to)
{
    for(int i = 0; i < 2048; i++)
    {
        to[i] = (from[i]) ? -1 : 0; // -1 in Uint32 is 0xFFFFFFFF so the pixel is white.
    }
}

void init_machine(struct machine_t* machine)
{
    //machine->sp = machine->I = machine->dt = machine->st = 0;
    machine->pc = 0x200;

    memset(machine, 0x00, sizeof(struct machine_t));
    //memset(machine->mem, 0, MEMSIZE);
    //memset(machine->v, 0, 16);
    //memset(machine->stack, 0, 16);
    memcpy(machine->mem + 0x50, hexcodes, 80); //16 numeros * 5 filas sprites

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
    struct machine_t mac;
    init_machine(&mac);
    load_rom(&mac);

    
    //srand(time(NULL));
    for(int i = 0; i < 2048; i++)
    {
        //Rand produces random numbers of 32 bits. With this mask will produce random numbers of 1 bit
        //mac.screen[i] = (rand() & 1); 
        mac.screen[i] = 0;
    }
    

    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* win = SDL_CreateWindow("Chip8",
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        640, 320,
                                        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

    SDL_Renderer* rnd = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* tex = SDL_CreateTexture(rnd, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
    SDL_Surface* surf = SDL_CreateRGBSurface(0, 64, 32, 32,
                                                0x00FF0000,
                                                0x0000FF00,
                                                0X000000FF,
                                                0XFF000000);

    //int pitch;
    //Uint32* pixels;
    
    // Draw pixels
    //memset(surf->pixels, 0xFF, 32 * surf->pitch);
    
    SDL_LockTexture(tex, NULL, &surf->pixels, &surf->pitch);
    expansion(mac.screen, (Uint32 *) surf->pixels);
    SDL_UnlockTexture(tex);

    int quit = 0; // Flag to shut down the chip8 emulator.
    SDL_Event ev;
    Uint32 last_delta = 0;

    while(!quit)
    {
        SDL_RenderClear(rnd);
        SDL_RenderCopy(rnd, tex, NULL, NULL);
        SDL_RenderPresent(rnd);
        
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
                case SDL_QUIT:
                    quit = 1;
                    break;
            }
        }
        
        step_machine(&mac);
        
        if(SDL_GetTicks() - last_delta > (1000/60))
        {
            SDL_LockTexture(tex, NULL, &surf->pixels, &surf->pitch);
            expansion(mac.screen, (Uint32 *) surf->pixels);
            SDL_UnlockTexture(tex);
            
            SDL_RenderCopy(rnd, tex, NULL, NULL);
            SDL_RenderPresent(rnd);
            last_delta = SDL_GetTicks();
        }

    }
    
    SDL_DestroyRenderer(rnd);
    SDL_DestroyWindow(win);
    SDL_Quit(); 
    
    return 0;
}




void step_machine(struct machine_t* mac)
{


    // Read opcode
    uint16_t opcode = (mac->mem[mac->pc] << 8) | mac->mem[mac->pc+1];

    mac->pc = (mac->pc + 2) & 0xFFF;
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
                //printf("CLS");
                memset(mac->screen, 0, 2048);
            }else if(opcode == 0x00EE){
                //printf("RET");
                if(mac->sp > 0){
                    mac->pc = mac->stack[mac->sp];
                    mac->sp = mac->sp - 1;
                }
            }

            break;
        
        case 1:
            //printf("JP %x", nnn);
            mac->pc = nnn;
            break;
        
        case 2:
            //printf("CALL %x", nnn);

            if(mac->sp < 16){
                mac->stack[mac->sp] = mac->pc;
                mac->sp++;
            }
            mac->pc = nnn;
            
            break;

        case 3:
            //printf("SE V%x, %x", x, kk);
            if(mac->v[x] == kk)
                mac->pc = (mac->pc + 2) & 0xFFF;

            break;

        case 4:
            //printf("SNE V%x, %x", x, kk);
            if(mac->v[x] != kk)
                mac->pc = (mac->pc + 2) & 0xFFF;

            break;

        case 5:
            //printf("SE V%x, V%x", x, y);
            if(mac->v[x] == mac->v[y])
                mac->pc = (mac->pc + 2) & 0xFFF;

            break;

        case 6:
            //printf("LD V%x, %x", x, kk);
            mac->v[x] = kk;
            break;

        case 7:
            //printf("ADD V%x, %x", x, kk);
            mac->v[x] = (mac->v[x] + kk) & 0xFF;
            break;

        case 8:
            
            switch (n) {
                case 0:
                    /*
                     * 8XY0: LD X, Y
                     * Set V[x] = V[y]
                     */
                    mac->v[x] = mac->v[y];
                    break;

                case 1:
                    /*
                     * 8XY1: OR X, Y
                     * Set V[x] to V[x] OR V[y].
                     */
                    mac->v[x] |= mac->v[y];
                    break;

                case 2:
                    /*
                     * 8XY2: AND X, Y
                     * Set V[x] to V[x] AND V[y].
                     */
                    mac->v[x] &= mac->v[y];
                    break;

                case 3:
                    /*
                     * 8XY3: XOR X, Y
                     * Set V[x] to V[x] XOR V[y]
                     */
                    mac->v[x] ^= mac->v[y];
                    break;

                case 4:
                    /*
                     * 8XY4: ADD X, Y
                     * Add V[y] to V[x]. V[15] is used as carry flag: if
                     * there is a carry, V[15] must be set to 1, else to 0.
                     */
                    mac->v[0xf] = (mac->v[x] > mac->v[x] + mac->v[y]);
                    mac->v[x] += mac->v[y];
                    break;

                case 5:
                    /*
                     * 8XY5: SUB X, Y
                     * Substract V[y] from V[x]. V[15] is used as borrow flag:
                     * if there is a borrow, V[15] must be set to 0, else
                     * to 1. Which in practice is easier to check as if
                     * V[x] is greater than V[y].
                     */
                    mac->v[0xF] = (mac->v[x] > mac->v[y]);
                    mac->v[x] -= mac->v[y];
                    break;

                case 6:
                    /*
                     * 8X06: SHR X
                     * Shifts right V[x]. Least significant bit from V[x]
                     * before shifting will be moved to V[15]. Thus, V[15]
                     * will be set to 1 if V[x] was odd before shifting.
                     */
                    mac->v[0xF] = (mac->v[x] & 1);
                    mac->v[x] >>= 1;
                    break;

                case 7:
                    /*
                     * 8XY7: SUBN X, Y
                     * Substract V[x] from V[y] and store the result in V[x].
                     * V[15] is used as a borrow flag in the same sense than
                     * SUB X, Y did: V[15] is set to 0 if there is borrow,
                     * else to 1. Which is easier to check as if V[y] is
                     * greater than V[x].
                     */
                    mac->v[0xF] = (mac->v[y] > mac->v[x]);
                    mac->v[x] = mac->v[y] - mac->v[x];
                    break;

                case 0xE:
                    /*
                     * 8X0E: SHL X
                     * Shifts left V[x]. Most significant bit from V[x] before
                     * shifting will be moved to V[15].
                     */
                    mac->v[0xF] = ((mac->v[x] & 0x80) != 0);
                    mac->v[x] <<= 1;
                    break;
            }
            
            break;
            
        case 9:
            //printf("SNE V%x, V%x", x, y);

            if(mac->v[x] != mac->v[y]){
                mac->pc = (mac->pc + 2) & 0xFFF;
            }
        case 0XA:
            //printf("LD I, %x", nnn);
            mac->I = nnn;
            break;

        case 0xB:
            //printf("JP V0, %x", nnn);
            mac->pc = (mac->v[0] + nnn) & 0xFFF;
                                                            
            break;

        case 0xC:
            //printf("RND V%x, %x", x, kk);
            mac->v[x] = rand() & kk;
            break;

        case 0xD:
            /*
             * Draws a sprite wich is stored in the addres that I points to I + n. The coordinates of the
             * screen is stored in V[x] and V[y]. If the position in wich the sprite is being drawn exceeds
             * the height or widh of the screen it will continue drawing in the opposite position of the
             * screen. The sprites are XORed against the actual values of the screen. If the value changes VF
             * is set to 1.           
             */

            //printf("DRW V%x, V%x, %x", x, y, n);
            mac->v[0xF] = 0;
            for(int j = 0; j < n; j++)
            {
                uint8_t sprite = mac->mem[mac->I + j];
                for(int i = 0; i < 8; i++)
                {
                    int px = (mac->v[x] + i) & 63;
                    int py = (mac->v[y] + j) & 31;

                    int pos = 64 * py + px;
                    int pixel = (sprite & (1 << (7-i))) != 0;

                    mac->v[0xF] = (mac->screen[pos] & pixel);
                    
                    mac->screen[pos] ^= pixel;
                    
                }
                
            }

            break;

        case 0xE:
            if( kk == 95 ){
                //printf("SKP V%x", x);
            }else if( kk == 0xA1 ){
                //printf("SKNP V%x", x);
            }

            break;

        case 0xF:
            switch (kk) {
                
                case 0x07:
                    //printf("LD V%x, DT", x);
                    mac->v[x] = mac->dt;
                    break;
                case 0x0A:
                    printf("LD V%x, k", x);
                    //Stores value of key in Vx
                    break;
                case 0x15:
                    //printf("LD DT, V%x", x);
                    mac->dt = mac->v[x];
                    break;
                case 0x18:
                    //printf("LD ST, V%x", x);
                    mac->st = mac->v[x];
                    break;
                case 0x1E:
                    //printf("ADD I, V%x", x);
                    mac->I = mac->I + mac->v[x];
                    break;
                case 0x29:
                    //printf("LD F, V%x", x);
                    mac->I = 0x50 + (mac->v[x] & 0xF) * 5; // 5 de 5 filas
                    break;
                case 0x33:
                    //printf("LD B, V%x", x);
                    //Hundreds digits in I,  tens in I+1 and ones I+2

                    mac->mem[mac->I + 2] = mac->v[x] % 10;
                    mac->mem[mac->I + 1] = (mac->v[x] / 10) % 10;
                    mac->mem[mac->I] = (mac->v[x] / 100);
                    break;
                case 0x55:
                    //printf("LD I, V%x", x);
                    for(int i = 0; i <= x; i++)
                        mac->mem[mac->I + i] = mac->v[i];     
                    break;
                case 0x65:
                    //printf("LD V%x, I", x);
                    for(int i = 0; i <= x; i++)
                        mac->v[i] = mac->mem[mac->I + i];
                        
                    break;
                
            }
            break;

        default:
            //printf("pene");
            break;


    }

    //printf("\n");
    // Control
    
    
}
