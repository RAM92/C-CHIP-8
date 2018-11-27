//
// Created by Rob on 21/07/2018.
//

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include "cpu.h"
#include "font.h"
#include "instruction.h"
#include "operation_definition.h"

#define c8_START_ADDRESS 0x200

static void write_i_reg(c8_cpu * cpu, uint16_t x);
static void inc_pc(c8_cpu * cpu);
static void bool_vf(c8_cpu * cpu, bool b);
static void set_pc_new_address(c8_cpu *cpu, uint16_t address);
static void push_stack(c8_cpu * cpu);
static void pop_stack(c8_cpu * cpu);
static void double_inc_pc_when(c8_cpu *cpu, bool b);
static void bcd(unsigned char x, uint8_t * htu);

////////Instructions///////////

static void store_nn_in_vx(c8_cpu *cpu, c8_instruction *inst) {
    cpu->v[inst->x] = inst->nn;
    inc_pc(cpu);
}

static void store_vy_in_vx(c8_cpu *cpu, c8_instruction *inst) {
    cpu->v[inst->x] = cpu->v[inst->y];
    inc_pc(cpu);
}

static void add_nn_to_vx(c8_cpu *cpu, c8_instruction *inst) {
    cpu->v[inst->x] += inst->nn;
    inc_pc(cpu);
}

static void add_vy_to_vx(c8_cpu *cpu, c8_instruction *inst) {
    uint8_t vx_pre_op = cpu->v[inst->x];
    cpu->v[inst->x] += cpu->v[inst->y];
    bool_vf(cpu, cpu->v[inst->x] < vx_pre_op);
    inc_pc(cpu);
}

static void subtract_vy_from_vx(c8_cpu *cpu, c8_instruction *inst) {
    uint8_t vx_pre_op = cpu->v[inst->x];
    cpu->v[inst->x] -= cpu->v[inst->y];
    bool_vf(cpu, cpu->v[inst->x] > vx_pre_op);
    inc_pc(cpu);
}

static void store_vy_sub_vx_in_vx(c8_cpu *cpu, c8_instruction *inst) {
    // This looks wrong in the python version.
    // Amend it before implementing this function
    // http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#8xy7
    inc_pc(cpu);
}

static void vx_and_vy_store_in_vx(c8_cpu *cpu, c8_instruction *inst) {
    cpu->v[inst->x] = cpu->v[inst->x] & cpu->v[inst->y];
    inc_pc(cpu);
}

static void vx_or_vy_store_in_vx(c8_cpu *cpu, c8_instruction *inst) {
    cpu->v[inst->x] = cpu->v[inst->x] | cpu->v[inst->y];
    inc_pc(cpu);
}

static void vx_xor_vy_store_in_vx(c8_cpu *cpu, c8_instruction *inst) {
    cpu->v[inst->x] = cpu->v[inst->x] ^ cpu->v[inst->y];
    inc_pc(cpu);
}

static void shift_vy_right_store_in_vx(c8_cpu *cpu, c8_instruction *inst) {
    uint8_t vy = cpu->v[inst->y];
    bool_vf(cpu, (vy & 1u) ? true : false);
    cpu->v[inst->x] = vy >> 1u;
    inc_pc(cpu);
}

static void shift_vy_left_store_in_vx(c8_cpu *cpu, c8_instruction *inst) {
    uint8_t vy = cpu->v[inst->y];
    bool_vf(cpu, (vy & 0x80u) ? true : false);
    cpu->v[inst->x] = vy << 1u;
    inc_pc(cpu);
}

static void set_vx_random_masked(c8_cpu *cpu, c8_instruction *inst) {
    static bool has_seeded = false;
    if (!has_seeded) {
        srand((unsigned int)time(0));
        has_seeded = true;
    }
    cpu->v[inst->x] = (uint8_t)rand() & inst->nn;
    inc_pc(cpu);
}

static void jump_to_nnn(c8_cpu *cpu, c8_instruction *inst) {
    set_pc_new_address(cpu, inst->nnn);
}

static void jump_to_nnn_plus_v0(c8_cpu *cpu, c8_instruction *inst) {
    set_pc_new_address(cpu, cpu->v[0] + inst->nnn);
}

static void exec_subroutine(c8_cpu *cpu, c8_instruction *inst) {
    push_stack(cpu);
    set_pc_new_address(cpu, inst->nnn);
}

static void return_from_subroutine(c8_cpu *cpu, c8_instruction *inst) {
    pop_stack(cpu);
    inc_pc(cpu);
}

static void skip_vx_eq_nn(c8_cpu *cpu, c8_instruction *inst) {
    double_inc_pc_when(cpu, (bool)(cpu->v[inst->x] == inst->nn));
}

static void skip_vx_eq_vy(c8_cpu *cpu, c8_instruction *inst) {
    double_inc_pc_when(cpu, (bool)(cpu->v[inst->x] == cpu->v[inst->y]));
}

static void skip_vx_neq_nn(c8_cpu *cpu, c8_instruction *inst) {
    double_inc_pc_when(cpu, (bool)(cpu->v[inst->x] != inst->nn));
}

static void skip_vx_neq_vy(c8_cpu *cpu, c8_instruction *inst) {
    double_inc_pc_when(cpu, (bool)(cpu->v[inst->x] != cpu->v[inst->y]));
}

static void set_delay_timer(c8_cpu *cpu, c8_instruction *inst) {
    inc_pc(cpu); // todo: Implement this!
}

static void set_sound_timer(c8_cpu *cpu, c8_instruction *inst) {
    inc_pc(cpu); // todo: Implement this!
}

static void delay_timer_to_vx(c8_cpu *cpu, c8_instruction *inst) {
    inc_pc(cpu); // todo: Implement this!
}

static void store_nnn_in_i(c8_cpu *cpu, c8_instruction *inst) {
    write_i_reg(cpu, inst->nnn);
    inc_pc(cpu);
}

static void add_vx_to_i(c8_cpu *cpu, c8_instruction *inst) {
    write_i_reg(cpu, cpu->i + cpu->v[inst->x]);
    inc_pc(cpu);
}

static void convert_vx_to_bcd(c8_cpu *cpu, c8_instruction *inst) {
    uint8_t result [3];

    if (cpu->i+2 < sizeof(cpu->memory)/sizeof(cpu->memory[0])) {
        bcd(cpu->v[inst->x], result);
        cpu->memory[cpu->i+0] = result[0];
        cpu->memory[cpu->i+1] = result[1];
        cpu->memory[cpu->i+2] = result[2];
    } else {
        // todo: Do some sort of logging here - This shouldn't happen
    }

    inc_pc(cpu);
}

static void v0_to_vx_to_memory(c8_cpu *cpu, c8_instruction *inst) {
    const uint16_t mem_len = sizeof(cpu->memory)/ sizeof(cpu->memory[0]);
    for (int i = 0; i < sizeof(cpu->v)/ sizeof(cpu->v[0]); ++i) {
        uint16_t address = cpu->i + i;
        if (address >= mem_len) break; // todo: log this!
        cpu->memory[address] = cpu->v[i];
        if (i == inst->x) break;
    }
    inc_pc(cpu);
}

static void memory_to_v0_to_vx(c8_cpu *cpu, c8_instruction *inst) {
    const uint16_t mem_len = sizeof(cpu->memory)/ sizeof(cpu->memory[0]);
    for (int i = 0; i < sizeof(cpu->v)/ sizeof(cpu->v[0]); ++i) {
        uint16_t address = cpu->i + i;
        if (address >= mem_len) break; // todo: log this!
        cpu->v[i] = cpu->memory[address];
        if (i == inst->x) break;
    }
    inc_pc(cpu);
}

static void draw_sprite(c8_cpu *cpu, c8_instruction *inst) {
    // todo
    inc_pc(cpu);
}

static void clear_screen(c8_cpu *cpu, c8_instruction *inst) {
    // todo
    inc_pc(cpu);
}

static void set_i_to_font_for_vx(c8_cpu *cpu, c8_instruction *inst) {
    cpu->i = inst->x * 5;
    inc_pc(cpu);
}

static void wait_for_keypad_store_in_vx(c8_cpu *cpu, c8_instruction *inst) {
    inc_pc(cpu);
}

static void skip_if_vx_eq_key_pressed(c8_cpu *cpu, c8_instruction *inst) {
    inc_pc(cpu);
}

static void skip_if_vx_neq_key_pressed(c8_cpu *cpu, c8_instruction *inst) {
    inc_pc(cpu);
}

static void unsupported_operation(c8_cpu *cpu, c8_instruction *inst) {
    // todo: Log this!
    inc_pc(cpu);
}

////////End of Instructions////


static void write_i_reg(c8_cpu * cpu, uint16_t x) {
    cpu->i = (uint16_t)(x & 0xfffu);
}

static void inc_pc(c8_cpu * cpu) {
    cpu->pc = (cpu->pc + 2) % sizeof(cpu->memory)/sizeof(cpu->memory[0]);
    // todo: Detect and log overflow here
}

static void bool_vf(c8_cpu * cpu, bool b) {
    if (b)
        *cpu->vf = 1;
    else
        *cpu->vf = 0;
}

void c8_cpu_init(c8_cpu *cpu) {
    if (!cpu) return;

    cpu->vf = cpu->v+0xf;

    uint8_t font_len = sizeof(c8_FONT)/sizeof(c8_FONT[0]);
    for (int i = 0; i < font_len; ++i) {
        cpu->memory[i] = c8_FONT[i];
    }
    for (int i = font_len; i < c8_START_ADDRESS; ++i) {
        cpu->memory[i] =   0x12; // jump to 0x200
        cpu->memory[++i] = 0x00;
    }
    cpu->pc = c8_START_ADDRESS;
}

static bool find_and_run_operation(
        c8_cpu * cpu,
        operation_definition *operations,
        size_t len,
        c8_instruction * inst) {
    for (operation_definition * ptr = operations; ptr < operations + len; ++ptr) {
        if (call_operation_definition(ptr, cpu, inst)) {
            return true;
        }
    }
    return false;
}

static void set_pc_new_address(c8_cpu *cpu, uint16_t address) {
    cpu->pc = address % sizeof(cpu->memory)/sizeof(cpu->memory[0]);
}

static void push_stack(c8_cpu * cpu) {
    if (cpu->stack_ptr == sizeof(cpu->stack)/sizeof(cpu->stack[0])) {
        // Stack pointer already at the end of the stack
        // todo: Handle stack overflow here! Halt?
    }
    cpu->stack[cpu->stack_ptr++] = cpu->pc;
}

static void pop_stack(c8_cpu * cpu) {
    if (cpu->stack_ptr != 0) {
        cpu->stack_ptr--;
    }
    cpu->i = cpu->stack[cpu->stack_ptr];
}

static void double_inc_pc_when(c8_cpu *cpu, bool b) {
    if (b)
        inc_pc(cpu);
    inc_pc(cpu);
}

static void bcd(unsigned char x, uint8_t * htu) {
    uint8_t u = (uint8_t)(x % 10);
    x -= u;
    uint8_t t = (uint8_t)((x % 100) / 10);
    x -= t * 10;
    uint8_t h = (uint8_t)(x / 100);
    htu[0] = h;
    htu[1] = t;
    htu[2] = u;
}

void c8_cpu_run(c8_cpu *cpu) {
    if (!cpu) return;

    c8_cpu_init(cpu);

    operation_definition operations [] = {
            get_operation_definition("6XNN", store_nn_in_vx,                "Store NN in VX"),
            get_operation_definition("8XY0", store_vy_in_vx,                "Store VY in VX"),
            get_operation_definition("7XNN", add_nn_to_vx,                  "Add NN to VX"),
            get_operation_definition("8XY4", add_vy_to_vx,                  "Add VY to VX"),
            get_operation_definition("8XY5", subtract_vy_from_vx,           "Subtract VY from VX"),
            get_operation_definition("8XY7", store_vy_sub_vx_in_vx,         "Store VY - VX in VX"),

            get_operation_definition("8XY2", vx_and_vy_store_in_vx,         "Store VX & VY in VX"),
            get_operation_definition("8XY1", vx_or_vy_store_in_vx,          "Store VX | VY in VX"),
            get_operation_definition("8XY3", vx_xor_vy_store_in_vx,         "Store VX ^ VY in VX"),

            get_operation_definition("8XY6", shift_vy_right_store_in_vx,    "Shift VY right, store in VX"),
            get_operation_definition("8XYE", shift_vy_left_store_in_vx,     "Shift VY left, store in VX"),

            get_operation_definition("CXNN", set_vx_random_masked,          "Set VX to random masked by NN"),

            get_operation_definition("1NNN", jump_to_nnn,                   "Jump to NNN"),
            get_operation_definition("BNNN", jump_to_nnn_plus_v0,           "Jump to NNN plus V0"),

            get_operation_definition("2NNN", exec_subroutine,               "Execute subroutine at NNN"),
            get_operation_definition("00EE", return_from_subroutine,        "Return from subroutine"),

            get_operation_definition("3XNN", skip_vx_eq_nn,                 "Skip if VX == NN"),
            get_operation_definition("5XY0", skip_vx_eq_vy,                 "Skip if VX == VY"),
            get_operation_definition("4XNN", skip_vx_neq_nn,                "Skip if VX != NN"),
            get_operation_definition("9XY0", skip_vx_neq_vy,                "Skip if VX != VY"),

            get_operation_definition("FX15", set_delay_timer,               "Set delay timer to VX"),
            get_operation_definition("FX18", set_sound_timer,               "Set sound timer to VX"),
            get_operation_definition("FX07", delay_timer_to_vx,             "Set VX to value in delay timer"),

            get_operation_definition("ANNN", store_nnn_in_i,                "Set I to NNN"),
            get_operation_definition("FX1E", add_vx_to_i,                   "Add VX to I"),

            get_operation_definition("FX33", convert_vx_to_bcd,             "Store BCD of VX at I, I+1 and I+2"),

            get_operation_definition("FX55", v0_to_vx_to_memory,            "Dump V0-VX at address I"),
            get_operation_definition("FX65", memory_to_v0_to_vx,            "Restore V0-VX from address I"),

            get_operation_definition("DXYN", draw_sprite,                   "Draw sprite at address I at VX VY"),
            get_operation_definition("00E0", clear_screen,                  "Clear the screen"),
            get_operation_definition("FX29", set_i_to_font_for_vx,          "Set I to the font character for VX"),

            get_operation_definition("FX0A", wait_for_keypad_store_in_vx,   "Wait for keypad input, store result in VX"),
            get_operation_definition("EX9E", skip_if_vx_eq_key_pressed,     "Skip if VX == key pressed"),
            get_operation_definition("EXA1", skip_if_vx_neq_key_pressed,    "Skip if VX != key pressed"),

            get_operation_definition("0NNN", unsupported_operation,         "Execute native code - UNSUPPORTED"),
    };

    c8_instruction inst;
    init_instruction(&inst, 0x7123);
    find_and_run_operation(
            cpu,

            operations,
            sizeof(operations)/sizeof(operations[0]),

            &inst
    );
}