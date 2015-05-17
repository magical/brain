CC := gcc
CFLAGS := -std=c99 -g -O2 -fstack-protector-all -D_FORTIFY_SOURCE=2 -Wall -Wextra -Werror -Wno-unused-parameter
LDFLAGS :=

ifeq "$(CC)" "gcc"
    CFLAGS += -flto
    LDFLAGS += -flto
endif
ifeq "$(CC)" "clang"
    CFLAGS += -Wconversion
endif

main: main.o brain.o bitvec.o heap.o
