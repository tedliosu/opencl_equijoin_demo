
# Get absolute filepath of "data_structures_opencl.h" header file for updating the "#include"
# directive to include the correct file within the OpenCL kernel definitions file
DATA_STRUCTURES_HEADER_FILE := $(shell find $(shell pwd) -name "data_structures_opencl\.h")
# The OpenCL kernel definitions file in which to update the "#include" directive
CL_FILE_TO_BE_UPDATED := ./src/equijoin_program.cl

# Expands to list of all source files
main_c_files := $(wildcard src/*.c)
# Name of main executable
main_prog_file = equijoin_gpu-vs-cpu

all: $(main_c_files)
	@# Before compiling to the executable, update the "#include" directive within the OpenCL
	@# kernel definitions file to match current absolute filepath of "data_structures_opencl.h"
	sed -i -E "s,^#include.+,#include \"$(DATA_STRUCTURES_HEADER_FILE)\",g" $(CL_FILE_TO_BE_UPDATED)
	gcc -g -O3 -o $(main_prog_file) $? -I./include $(CPPFLAGS) -lm -lbsd -lOpenCL $(LDFLAGS)

clean:
	rm -f $(main_prog_file)

