## This is a simple Makefile with lots of comments 
## Check Unix Programming Tools handout for more info.

# Define what compiler to use and the flags.
CC=gcc
CXX=CC
CCFLAGS= -g -std=c99 -D_POSIX_C_SOURCE=200809L -Wall -Werror -pthread -lrt

all: candykids

# Compile all .c files into .o files
# % matches all (like * in a command)
# $< is the source file (.c file)
%.o : %.c
	$(CC) -c $(CCFLAGS) $<



candykids: candykids.o bbuff.o stats.o
	$(CC) $(CCFLAGS) -o candykids candykids.o bbuff.o stats.o	

clean:
	rm -f core *.o candykids

