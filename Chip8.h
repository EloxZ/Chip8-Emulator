#pragma once

#include <stack>
#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <bitset>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <ctime>


const unsigned int FONTSET_SIZE { 80 };
const unsigned int MEMORY_SIZE { 4096 };
const unsigned int REGISTERS_SIZE { 16 };
const unsigned int DISPLAY_WIDTH { 64 }, DISPLAY_HEIGHT { 32 };
const uint8_t fontset[FONTSET_SIZE] =
        {
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

class Chip8
{
    public:
        unsigned int video[DISPLAY_WIDTH][DISPLAY_HEIGHT] {0};
        bool drawFlag;
        bool halt;
        bool shiftQuirk, loadStoreQuirk;
        unsigned char key[16] = {0x00};
        uint8_t soundTimer;

        Chip8();

        void loadROM(std::string romName);
        void startCycle(float period);
        void setQuirks(bool value);

    private:
        uint8_t memory[MEMORY_SIZE] = {0};
        uint8_t V[REGISTERS_SIZE]  = {0};
        uint16_t I;
        uint8_t delayTimer;
        
        uint16_t pc;
        uint16_t sp;

        std::stack<uint16_t> stack;

        
        void DecrementDelay(auto delayStart, auto delayDuration);

        void executeNextInstruction();
        void executeInstruction(uint16_t instruction);
        
        void op_00E0();
        void op_00EE();
        void op_1NNN(uint16_t instruction);
        void op_2NNN(uint16_t instruction);
        void op_3XNN(uint16_t instruction);
        void op_4XNN(uint16_t instruction);
        void op_5XY0(uint16_t instruction);
        void op_6XNN(uint16_t instruction);
        void op_7XNN(uint16_t instruction);
        void op_8XY0(uint16_t instruction);
        void op_8XY1(uint16_t instruction);
        void op_8XY2(uint16_t instruction);
        void op_8XY3(uint16_t instruction);
        void op_8XY4(uint16_t instruction);
        void op_8XY5(uint16_t instruction);
        void op_8XY6(uint16_t instruction);
        void op_8XY7(uint16_t instruction);
        void op_8XYE(uint16_t instruction);
        void op_9XY0(uint16_t instruction);

        void op_ANNN(uint16_t instruction);
        void op_BNNN(uint16_t instruction);
        void op_CXNN(uint16_t instruction);
        void op_DXYN(uint16_t instruction);

        void op_EX9E(uint16_t instruction);
        void op_EXA1(uint16_t instruction);

        void op_FX07(uint16_t instruction);
        void op_FX0A(uint16_t instruction);
        void op_FX15(uint16_t instruction);
        void op_FX18(uint16_t instruction);
        void op_FX1E(uint16_t instruction);
        void op_FX29(uint16_t instruction);
        void op_FX33(uint16_t instruction);
        void op_FX55(uint16_t instruction);
        void op_FX65(uint16_t instruction);
};