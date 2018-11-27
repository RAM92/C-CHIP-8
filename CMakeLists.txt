cmake_minimum_required(VERSION 3.10)
project(chip-8 C)
set(CMAKE_C_STANDARD 11)

add_executable(
        chip8
        main.c
        operation_definition.c
        instruction.c
        cpu.c
        screen.c
)
target_link_libraries(chip8 ncurses)