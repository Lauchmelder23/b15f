# Name: Makefile
# Project: B15F (board15 Famulus Edition)
# Author: Robert Altner
# Creation Date: 2022-12-6

# Environment
COMPILER_PATH = gcc

# Options
CFLAGS   = -std=c11 -O3 -Wall -Wextra
LDFLAGS  = -lb15fwrp -lb15fdrv -lstdc++
OBJECTS  = main.o
OUT      = main.elf

COMPILE = $(COMPILER_PATH) $(CFLAGS)

main: $(OBJECTS)
	$(COMPILE) $(OBJECTS) -o $(OUT) $(LDFLAGS)

help:
	@echo "This Makefile has the following targets:"
	@echo "make main .... to compile"
	@echo "make clean ... to delete objects and executables"
	
clean:
	@echo "Cleaning..."
	rm -f $(OBJECTS) $(OUT) *.bin gnuplotscript.gp

.cpp.o:
	$(COMPILE) -c $< -o $@
