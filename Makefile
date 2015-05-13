CC=gcc
CFLAGS=-std=c99 -ggdb -O2 -flto -fstack-protector-all -Wall -Wextra -Werror -Wno-unused-parameter
LDFLAGS=-flto
main: main.o brain.o bitvec.o
