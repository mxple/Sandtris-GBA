GCC_VERSION = $(shell arm-none-eabi-gcc -dumpversion)
LINKSCRIPT_DIR = /opt/cs2110-tools

CROSS   := arm-none-eabi-
AS      := $(CROSS)as
CC      := $(CROSS)gcc
LD      := $(CROSS)ld
OBJCOPY := $(CROSS)objcopy

ARMINC = /usr/arm-none-eabi/include
ARMLIB = /usr/arm-none-eabi/lib
GCCLIB = /usr/lib/gcc/arm-none-eabi/$(GCC_VERSION)

CRELEASE = -O2
CDEBUG = -g -DDEBUG
LDRELEASE = -s
LDDEBUG = -g

MODEL    = -mthumb-interwork -mthumb
CFLAGS   = -Wall -Werror -std=c99 -pedantic -Wextra -fno-common $(MODEL) -mlong-calls -I $(ARMINC)
LDFLAGS = -nostartfiles -lc -lgcc -L $(ARMLIB) \
	  -L $(ARMLIB)/thumb \
	  -L $(GCCLIB) \
	  -T $(LINKSCRIPT_DIR)/arm-gba.ld

crt0.o : $(LINKSCRIPT_DIR)/crt0.s
	@$(AS) $(MODEL) $^ -o crt0.o

libc_sbrk.o : $(LINKSCRIPT_DIR)/libc_sbrk.c
	@$(CC) $(CFLAGS) $(CRELEASE) -c $< -o libc_sbrk.o
