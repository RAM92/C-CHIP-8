//
// Created by Rob on 21/07/2018.
//
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "operation_definition.h"

static bool operation_definition_responds(
        operation_definition * op,
        c8_instruction * inst
);

static int8_t upper_hex_ch_to_int(char const ch) {
    switch (ch) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        case 'A': return 10;
        case 'B': return 11;
        case 'C': return 12;
        case 'D': return 13;
        case 'E': return 14;
        case 'F': return 15;
        default:
            return -1;
    }
}

operation_definition get_operation_definition(
        char const * format_str,
        void (*cb)(
                c8_cpu *,
                c8_instruction *
        ),
        char const * description // todo: Use this for logging and errors
) {
    operation_definition op = {
        .str = format_str,
        .cb = cb
    };

    size_t len = strlen(format_str);
    for (uint8_t i = 0; i < len; ++i) {
        char const * ch = format_str + len-1 - i;
        uint8_t bit_position = (uint8_t)(i * 4);

        int8_t const x_ = upper_hex_ch_to_int(*ch);
        if (x_ == -1)
            continue;

        uint8_t x = (uint8_t)x_;

        op.mask |= (0xfu << bit_position);
        op.match_number |= (x << bit_position);
    }

    return op;
}

bool call_operation_definition(
        operation_definition * op,
        c8_cpu * cpu,
        c8_instruction * inst
        ) {
    if (operation_definition_responds(op, inst)) {
        op->cb(cpu, inst);
        return true;
    }
    return false;
}

static bool operation_definition_responds(
        operation_definition * op,
        c8_instruction * inst
) {
    return (inst->data & op->mask) == op->match_number;
}