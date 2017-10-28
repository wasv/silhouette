CXX = gcc

LDFLAGS = -lOpenCL -lm -lblas -lclBLAS
CXXFLAGS = -march=native -O0 -std=c11 -Wall -Wextra -pedantic-errors -pthread 

# ignore warnings when compiling if warn=0
ifeq ($(warn), 0)
	CXXFLAGS += -w
endif


.PHONY: all
all: main

%: %.c
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

%.bin: %.hd
	xxd -c 10 -l 80 -r $< | tee $@ | hexdump -v -e '10/1 "%02x " "\n"'
