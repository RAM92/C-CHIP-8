//
// Created by Rob on 21/07/2018.
//

#ifndef SIMPLETOOLS_INTSTRUCTION_H
#define SIMPLETOOLS_INTSTRUCTION_H

#include <stdlib.h>

typedef struct c8_instruction c8_instruction;
struct c8_instruction {
    uint16_t data;
    uint8_t x;
    uint8_t y;
    uint8_t n;
    uint16_t nnn;
    uint8_t nn;
};

void init_instruction(
        c8_instruction * inst,
        uint16_t data
        );

#endif //SIMPLETOOLS_INTSTRUCTION_H
