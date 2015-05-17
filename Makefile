CC=gcc
CFLAGS=-std=c99 -g -O2 -flto -fstack-protector-all -D_FORTIFY_SOURCE=2 -Wall -Wextra -Werror -Wno-unused-parameter
LDFLAGS=-flto
main: main.o brain.o bitvec.o heap.o
