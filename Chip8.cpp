#include "Chip8.h"

Chip8::Chip8()
{
    pc = 512;
    sp = 0;
    I = 0;
    halt = false;
    srand(time(nullptr));
    drawFlag = false;
    delayTimer = 0;
    soundTimer = 0;
    shiftQuirk = false;
    loadStoreQuirk = false;

    int i = 0;
    while (i < FONTSET_SIZE) {
        memory[i] = fontset[i];
        i++;
    }
}


void Chip8::loadROM(std::string romName)
{
    std::cout << "Trying to load rom " << romName << '\n';
    std::ifstream input(".\\roms\\" + romName, std::ios::binary);

    if (input.fail()) {
        std::cout << "Error trying to open " << romName << '\n';
    } else {
        std::vector<char> bytes((std::istreambuf_iterator<char>(input)), (std::istreambuf_iterator<char>()));
        std::cout << "Roam loaded." << '\n' << '\n';
        int index = 0;
        int i = pc;
        for (char c: bytes)
        {
            std::bitset<8> x(c);
            //std::cout << x;
            memory[i] = c;
            index++;
            i++;
            if (index == 2)
            {
                index = 0;
                //std::cout << '\n';
            }
        }
   
        input.close();
    }
   
}

void Chip8::setQuirks(bool value) {
    shiftQuirk = value;
    loadStoreQuirk = value;
}

void Chip8::DecrementDelay(auto delayStart, auto delayDuration) {
    while (!halt) {
        auto now = std::chrono::steady_clock::now();
        if ((now - delayStart).count() / 1000000.0 >= delayDuration.count()) {
            //std::cout << (std::chrono::steady_clock::now() - delayStart).count() / 1000000.0 << " ms" << std::endl;
            delayStart = now;
            if (delayTimer > 0) delayTimer--;
        }
    }
}

void Chip8::startCycle(float period) {
    auto periodDuration = std::chrono::duration<float, std::milli>(period);
    auto delayDuration = std::chrono::duration<float, std::milli>(100.0/6.0);
    
    auto delayStart = std::chrono::steady_clock::now();

    std::thread delayThread([delayStart, delayDuration, this]() {
        DecrementDelay(delayStart, delayDuration);
    });

    std::cout << "Cycled started" << '\n';
    while(!halt) {
        auto start = std::chrono::steady_clock::now();

        //std::cout << std::hex << "PC " << pc << '\n';
        //std::cout << std::hex << "I " << I << '\n';
        //std::cout << std::hex << "SP "<< sp << '\n';
        //std::cout << "DrawFlag " << drawFlag << '\n';

        executeNextInstruction();

        while ((periodDuration - (std::chrono::steady_clock::now() - start)).count() > 0) {}
        //std::cout << (std::chrono::steady_clock::now() - start).count() / 1000000.0 << " ms" << std::endl;
    }

    std::cout << "Cycled stopped" << '\n';
}

void Chip8::executeNextInstruction() {
    // Fetch
    if (pc+1 < MEMORY_SIZE) {
        uint16_t instruction = (uint16_t(memory[pc]) << 8) | uint16_t(memory[pc+1]);
        pc += 2;
        //std::cout << "INSTRUCTION " << std::hex << instruction << '\n';
        executeInstruction(instruction);
    } else {
        halt = true;
    }
}

void Chip8::executeInstruction(uint16_t instruction) {
    uint8_t opcode = (instruction >> 12) & 0xF;
    uint8_t x = (instruction >> 8) & 0xF;
    uint8_t y = (instruction >> 4) & 0xF;
    uint8_t n = instruction & 0xF;
    uint8_t kk = instruction & 0xFF;
    uint16_t nnn = instruction & 0xFFF;

    switch (opcode) {
        case 0x0:
            // CLS or RET
            if (instruction == 0x00E0) {
                op_00E0();
                break;
            } else if (instruction == 0x00EE) {
                op_00EE();
                break;
            }
            halt = true;
            break;
        case 0x1:
            op_1NNN(instruction);
            break;
        case 0x2:
            op_2NNN(instruction);
            break;
        case 0x3:
            // SE Vx, byte
            op_3XNN(instruction);
            break;
        case 0x4:
            // SNE Vx, byte
            op_4XNN(instruction);
            break;
        case 0x5:
            // SE Vx, Vy
            op_5XY0(instruction);
            break;
        case 0x6:
            // LD Vx, byte
            op_6XNN(instruction);
            break;
        case 0x7:
            // ADD Vx, byte
            op_7XNN(instruction);
            break;
        case 0x8:
            switch (n) {
                case 0x0:
                    // LD Vx, Vy
                    op_8XY0(instruction);
                    break;
                case 0x1:
                    // OR Vx, Vy
                    op_8XY1(instruction);
                    break;
                case 0x2:
                    // AND Vx, Vy
                    op_8XY2(instruction);
                    break;
                case 0x3:
                    // XOR Vx, Vy
                    op_8XY3(instruction);
                    break;
                case 0x4:
                    // ADD Vx, Vy
                    op_8XY4(instruction);
                    break;
                case 0x5:
                    // SUB Vx, Vy
                    op_8XY5(instruction);
                    break;
                case 0x6:
                    // SHR Vx {, Vy}
                    op_8XY6(instruction);
                    break;
                case 0x7:
                    // SUBN Vx, Vy
                    op_8XY7(instruction);
                    break;
                case 0xE:
                    // SHL Vx {, Vy}
                    op_8XYE(instruction);
                    break;
            }
            break;
        case 0x9:
            // SNE Vx, Vy
            op_9XY0(instruction);
            break;
        case 0xA:
            // LD I, addr
            op_ANNN(instruction);
            break;
        case 0xB:
            // JP V0, addr
            op_BNNN(instruction);
            break;
        case 0xC:
            // RND Vx, byte
            op_CXNN(instruction);
            break;
        case 0xD:
            // DRW Vx, Vy, nibble
            op_DXYN(instruction);
            break;
        case 0xE:
            switch (kk) {
                case 0x9E:
                    // SKP Vx
                    op_EX9E(instruction);
                    break;
                case 0xA1:
                    // SKNP Vx
                    op_EXA1(instruction);
                    break;
            }
            break;
        case 0xF:
            switch (kk) {
                case 0x07:
                    // LD Vx, DT
                    op_FX07(instruction);
                    break;
                case 0x0A:
                    // LD Vx, K
                    op_FX0A(instruction);
                    break;
                case 0x15:
                    // LD DT, Vx
                    op_FX15(instruction);
                    break;
                case 0x18:
                    // LD ST, Vx
                    op_FX18(instruction);
                    break;
                case 0x1E:
                    // ADD I, Vx
                    op_FX1E(instruction);
                    break;
                case 0x29:
                    // LD F, Vx
                    op_FX29(instruction);
                    break;
                case 0x33:
                    // LD B, Vx
                    op_FX33(instruction);
                    break;
                case 0x55:
                    // LD [I], Vx
                    op_FX55(instruction);
                    break;
                case 0x65:
                    // LD Vx, [I]
                    op_FX65(instruction);
                    break;
            }
            break;
        default:
            halt = true;
            break;
    }
}

// Clear screen
void Chip8::op_00E0()
{
    //std::cout << "op_00E0" << '\n';
    for (unsigned int y=0; y < DISPLAY_HEIGHT; y++)
    {
        for (unsigned int x=0; x < DISPLAY_WIDTH; x++)
        {   
            video[x][y] = 0;
        }
    }

    drawFlag = true;
}

// Return from subroutine call
void Chip8::op_00EE()
{
    //std::cout << "op_00EE" << '\n';
    if (!stack.empty()) {
        pc = stack.top();
        stack.pop();
    } 
}

// Jump to address NNN
void Chip8::op_1NNN(uint16_t instruction)
{
    //std::cout << "op_1NNN" << '\n';
    uint16_t address = instruction & 0x0FFF;
    pc = address;
}

// Skip the next instruction if register X equals value NN
void Chip8::op_3XNN(uint16_t instruction) {
    //std::cout << "op_3XNN" << '\n';
    // Extract register index X from instruction
    uint8_t X = (instruction >> 8) & 0x0F;
    // Extract value NN from instruction
    uint8_t NN = instruction & 0xFF;
    // Compare value in register X to value NN
    if (V[X] == NN) {
        // If they are equal, skip the next instruction
        pc += 2;
    }
}

// Skip the next instruction if register X is not equal to value NN
void Chip8::op_4XNN(uint16_t instruction) {
    //std::cout << "op_4XNN" << '\n';
    uint8_t X = (instruction >> 8) & 0x0F;
    uint8_t RR = instruction & 0xFF;

    if (V[X] != RR) {
        pc += 2;
    }
}

// Skip the next instruction if the value in register X equals the value in register Y
void Chip8::op_5XY0(uint16_t instruction) {
    //std::cout << "op_5XY0" << '\n';
    uint8_t X = (instruction >> 8) & 0x0F;
    // Extract register index Y from instruction
    uint8_t Y = (instruction >> 4) & 0x0F;

    if (V[X] == V[Y]) {
        pc += 2;
    }
}

void Chip8::op_8XY0(uint16_t instruction) {
    //std::cout << "op_8XY0" << '\n';
    uint8_t X = (instruction >> 8) & 0x0F;
    uint8_t Y = (instruction >> 4) & 0x0F;
    V[X] = V[Y];
}

void Chip8::op_8XY1(uint16_t instruction) {
    //std::cout << "op_8XY1" << '\n';
    uint8_t X = (instruction >> 8) & 0x0F;
    uint8_t Y = (instruction >> 4) & 0x0F;
    V[X] |= V[Y];
}

void Chip8::op_8XY2(uint16_t instruction) {
    //std::cout << "op_8XY2" << '\n';
    uint8_t X = (instruction >> 8) & 0x0F;
    uint8_t Y = (instruction >> 4) & 0x0F;
    V[X] &= V[Y];
}

void Chip8::op_8XY3(uint16_t instruction) {
    //std::cout << "op_8XY3" << '\n';
    uint8_t X = (instruction >> 8) & 0x0F;
    uint8_t Y = (instruction >> 4) & 0x0F;
    V[X] ^= V[Y];
}

void Chip8::op_8XY4(uint16_t instruction) {
    //std::cout << "op_8XY4" << '\n';
    uint8_t X = (instruction >> 8) & 0x0F;
    uint8_t Y = (instruction >> 4) & 0x0F;
    uint16_t sum = V[X] + V[Y];
    V[X] = sum & 0xFF;
    V[0xF] = (sum > 0xFF) ? 1 : 0;
}

void Chip8::op_8XY6(uint16_t instruction) {
    //std::cout << "op_8XY6" << '\n';
    uint8_t X = (instruction >> 8) & 0x0F;
    uint8_t Y = (shiftQuirk)? X : ((instruction >> 4) & 0x0F);
    uint8_t t = V[Y] & 0x1;
    V[X] = V[Y] >> 1;
    V[0xF] = t;
}

void Chip8::op_8XY7(uint16_t instruction) {
    //std::cout << "op_8XY7" << '\n';
    uint8_t X = (instruction >> 8) & 0x0F;
    uint8_t Y = (instruction >> 4) & 0x0F;
    V[X] = V[Y] - V[X];
    V[0xF] = (V[Y] > V[X]) ? 1 : 0;
}

void Chip8::op_8XYE(uint16_t instruction) {
    //std::cout << "op_8X0E" << '\n';
    uint8_t X = (instruction >> 8) & 0x0F;
    uint8_t Y = (shiftQuirk)? X : ((instruction >> 4) & 0x0F);
    uint8_t t = V[Y] >> 7;
    V[X] = V[Y] << 1;
    V[0xF] = t;
}

void Chip8::op_9XY0(uint16_t instruction) {
    //std::cout << "op_9XY0" << '\n';
    uint8_t X = (instruction >> 8) & 0x0F;
    uint8_t Y = (instruction >> 4) & 0x0F;
    if (V[X] != V[Y]) {
        pc += 2;
    }
}

void Chip8::op_ANNN(uint16_t instruction) {
    //std::cout << "op_ANNN" << '\n';
    uint16_t address = instruction & 0x0FFF;
    I = address; // 726
}

void Chip8::op_BNNN(uint16_t instruction) {
    //std::cout << "op_BNNN" << '\n';
    uint16_t address = instruction & 0x0FFF;
    pc = address + V[0];
}

void Chip8::op_CXNN(uint16_t instruction) {
    //std::cout << "op_CXNN" << '\n';
    uint8_t X = (instruction >> 8) & 0x0F;
    uint8_t byte = instruction & 0xFF;
    uint8_t random_byte = rand() & 0xFF;
    
    V[X] = random_byte & byte;

}

void Chip8::op_DXYN(uint16_t instruction) {
    //std::cout << "op_DXYN" << '\n';
    uint16_t x = V[(instruction & 0x0F00u) >> 8u];
    uint16_t y = V[(instruction & 0x00F0u) >> 4u];
    uint16_t height = instruction & 0x000Fu;
    uint16_t pixel;

    V[0xF] = 0;
    for (uint16_t yline = 0; yline < height; ++yline) {
        pixel = memory[I + yline];
        for (uint16_t xline = 0; xline < 8; ++xline) {
            if ((pixel & (0x80u >> xline)) != 0) {
                if (video[(x + xline) % 64][(y + yline) % 32] == 1) {
                    V[0xF] = 1;
                }
                video[(x + xline) % 64][(y + yline) % 32] ^= 1;
            }
        }
    }

    drawFlag = true;
}


// FX07: Set VX to the value of the delay timer
void Chip8::op_FX07(uint16_t instruction) {
    //std::cout << "op_FX07" << '\n';
    uint16_t x = (instruction & 0x0F00u) >> 8u;
    V[x] = delayTimer;
}

// FX0A: A key press is awaited, and then stored in VX
void Chip8::op_FX0A(uint16_t instruction) {
    //std::cout << "op_FX0A" << '\n';
    uint16_t x = (instruction & 0x0F00u) >> 8u;

    for (int i = 0; i < 16; ++i) {
        if (key[i] != 0) {
            V[x] = i;
            return;
        }
    }

    pc -= 2;
}

// FX15: Set the delay timer to VX
void Chip8::op_FX15(uint16_t instruction) {
    //std::cout << "op_FX15" << '\n';
    unsigned short x = (instruction & 0x0F00u) >> 8u;
    delayTimer = V[x];
}

// FX18: Set the sound timer to VX
void Chip8::op_FX18(uint16_t instruction) {
    //std::cout << "op_FX18" << '\n';
    uint16_t x = (instruction & 0x0F00u) >> 8u;
    soundTimer = V[x];
}

// FX1E: Add VX to I
void Chip8::op_FX1E(uint16_t instruction) {
    //std::cout << "op_FX1E" << '\n';
    uint16_t x = (instruction & 0x0F00u) >> 8u;
    I += V[x];
}

// FX29: Set I to the location of the sprite for the character in VX
void Chip8::op_FX29(uint16_t instruction) {
    //std::cout << "op_FX29" << '\n';
    uint16_t x = (instruction & 0x0F00u) >> 8u;
    I = V[x] * 5;
}

// FX33: Store the binary-coded decimal representation of VX at the addresses I, I+1, and I+2
void Chip8::op_FX33(uint16_t instruction) {
    //std::cout << "op_FX33" << '\n';
    uint16_t x = (instruction & 0x0F00u) >> 8u;
    memory[I] = V[x] / 100;
    memory[I + 1] = (V[x] / 10) % 10;
    memory[I + 2] = V[x] % 10;
}
// FX55: Store V0 to VX (inclusive) in memory starting at address I
void Chip8::op_FX55(uint16_t instruction) {
    //std::cout << "op_FX55" << '\n';
    uint16_t x = (instruction & 0x0F00u) >> 8u;
    for (int i = 0; i <= x; ++i) {
        memory[I + i] = V[i];
    }
    if (!loadStoreQuirk) I += x + 1;
}

// FX65: Fill V0 to VX (inclusive) with values from memory starting at address I
void Chip8::op_FX65(uint16_t instruction) {
    //std::cout << "op_FX65" << '\n';
    uint16_t x = (instruction & 0x0F00u) >> 8u;
    for (int i = 0; i <= x; ++i) {
        V[i] = memory[I + i];
    }
    if (!loadStoreQuirk) I += x + 1;
}

void Chip8::op_6XNN(uint16_t instruction) {
    //std::cout << "op_6XNN" << '\n';
    uint8_t x = (instruction >> 8) & 0x0F;
    uint8_t nn = instruction & 0xFF;
    V[x] = nn;
}

void Chip8::op_7XNN(uint16_t instruction) {
    //std::cout << "op_7XNN" << '\n';
    uint8_t x = (instruction >> 8) & 0x0F;
    uint8_t nn = instruction & 0xFF;
    V[x] += nn;
}

void Chip8::op_2NNN(uint16_t instruction) {
    //std::cout << "op_2NNN" << '\n';
    uint16_t address = instruction & 0x0FFF; // extract NNN  724
    
    // push current pc to the stack and update pc to address
    stack.push(pc);
    pc = address;
}

void Chip8::op_EX9E(uint16_t instruction) {
    //std::cout << "op_EK9E" << '\n';
    uint8_t x = (instruction >> 8) & 0x0F; // extract X
    
    if (key[V[x]]) {
        pc += 2;
    }
}

void Chip8::op_EXA1(uint16_t instruction) {
    //std::cout << "op_EKA1" << '\n';
    uint8_t x = (instruction >> 8) & 0x0F; // extract X
    
    if (!key[V[x]]) {
        pc += 2;
    }
}

void Chip8::op_8XY5(uint16_t instruction) {
    //std::cout << "op_8XY5" << '\n';
    uint8_t x = (instruction >> 8) & 0x0F; // extract X
    uint8_t y = (instruction >> 4) & 0x0F; // extract Y
    uint8_t t = (V[x] >= V[y])? 1 : 0;
    V[x] -= V[y];
    V[0xF] = t;
}

