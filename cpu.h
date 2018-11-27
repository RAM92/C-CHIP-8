//
// Created by Rob on 21/07/2018.
//

#ifndef SIMPLETOOLS_CPU_H
#define SIMPLETOOLS_CPU_H

#include <stdint.h>

typedef struct c8_cpu c8_cpu;
struct c8_cpu {
    uint16_t pc;

    uint8_t v[16];
    uint8_t * vf;

    uint16_t i;

    uint8_t memory[4096];
    // todo: delay & sound timers + screen & keypad

    uint16_t stack[32];
    uint8_t stack_ptr;
};

void c8_cpu_init(c8_cpu *cpu);
void c8_cpu_run(c8_cpu *cpu); // Calls init first

#endif //SIMPLETOOLS_CPU_H
