//
// Created by Rob on 21/07/2018.
//

#include "instruction.h"

void init_instruction(
        c8_instruction * inst,
        uint16_t data) {
    inst->data = data;
    inst->x =   (uint8_t)(data & 0x0f00u) >> 8u;
    inst->y =   (uint8_t)(data & 0x00f0u) >> 4u;
    inst->n =   (uint8_t)(data & 0x000fu);
    inst->nnn =  data & (u_int16_t)0x0fff;
    inst->nn =  (uint8_t)(data & 0x00ffu);
}