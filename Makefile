objects = main.o cpu.o instruction.o operation_definition.o

chip8 : $(objects)
	cc -o chip8 $(objects)

main.o : cpu.h screen.h
cpu.o : cpu.h font.h instruction.h operation_definition.h
instruction.o : instruction.h
operation_definition.o : instruction.h operation_definition.h

.PHONY : clean
clean :
	-rm chip8 $(objects)




# Example of grouping by prerequisites:
#main.o cpu.o operation_definition.o : cpu.h
#cpu.o : font.h
#cpu.o instruction.o operation_definition.o : instruction.h
#cpu.o operation_definition.o : operation_definition.h