//
// Created by Rob on 21/07/2018.
//

#ifndef SIMPLETOOLS_OPERATION_DEFINITION_H
#define SIMPLETOOLS_OPERATION_DEFINITION_H

#include <stdint.h>
#include <stdbool.h>

#include "cpu.h"
#include "instruction.h"

typedef struct operation_definition operation_definition;
struct operation_definition {
    uint16_t match_number;
    uint16_t mask;

    char const * str;
    void (*cb)(c8_cpu *, c8_instruction *);
};

operation_definition get_operation_definition(
        char const * format_str,
        void (*cb)(c8_cpu *, c8_instruction *),
        char const * description
        );

bool call_operation_definition(
        operation_definition * o,
        c8_cpu * cpu,
        c8_instruction * inst
        );
#endif //SIMPLETOOLS_OPERATION_DEFINITION_H
