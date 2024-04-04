PROGNAME = Sandtris

OFILES := $(shell find . -name '*.c' | sed 's/\.c/\.o/g')

.PHONY: all
all: CFLAGS += $(CDEBUG) -I../shared
all: LDFLAGS += $(LDDEBUG)
all: $(PROGNAME).gba
	@echo "[FINISH] Created $(PROGNAME).gba"

GCC_VERSION = $(shell arm-none-eabi-gcc -dumpversion)
LINKSCRIPT_DIR = ./cs2110-tools

CROSS   := arm-none-eabi-
AS      := $(CROSS)as
CC      := $(CROSS)gcc
LD      := $(CROSS)ld
OBJCOPY := $(CROSS)objcopy

ARMINC = /usr/local/include/arm-none-eabi/
ARMLIB = /usr/local/lib/arm-none-eabi/
GCCLIB = /usr/local/lib/gcc/arm-none-eabi/$(GCC_VERSION)

CRELEASE = -O2
CDEBUG = -g -DDEBUG
LDRELEASE = -s
LDDEBUG = -g

MODEL   = -mthumb-interwork -mthumb
CFLAGS  = -Wall -std=c99 -pedantic -Wextra -fno-common $(MODEL) -mlong-calls -I $(ARMINC)
LDFLAGS = -nostartfiles -lc -lgcc -L $(ARMLIB) \
	 -L $(ARMLIB)/thumb \
	 -L $(GCCLIB) \
	 -T $(LINKSCRIPT_DIR)/arm-gba.ld

crt0.o : $(LINKSCRIPT_DIR)/crt0.s
	@$(AS) $(MODEL) $^ -o crt0.o

libc_sbrk.o : $(LINKSCRIPT_DIR)/libc_sbrk.c
	@$(CC) $(CFLAGS) $(CRELEASE) -c $< -o libc_sbrk.o

LDFLAGS += --specs=nosys.specs

# Adjust default compiler warnings and errors
CFLAGS += -Wstrict-prototypes -Wold-style-definition -Werror=vla
CFLAGS += -O2

$(PROGNAME).gba: $(PROGNAME).elf 
	@echo "[LINK] Linking objects together to create $(PROGNAME).gba"
	@$(OBJCOPY) -O binary $(PROGNAME).elf $(PROGNAME).gba

$(PROGNAME).elf: crt0.o $(GCCLIB)/crtbegin.o $(GCCLIB)/crtend.o $(GCCLIB)/crti.o $(GCCLIB)/crtn.o $(OFILES) #libc_sbrk.o
	$(CC) -o $(PROGNAME).elf $^ $(LDFLAGS)

.PHONY: mgba
mgba:
	@mgba ${PROGNAME}.gba
	@echo "[EXECUTE] Running emulator mGBA"

.PHONY: clean
clean:
	@echo "[CLEAN] Removing all compiled files"
	rm -f *.o *.elf *.gba *.log */*.o *.sav */*/*.sav client
