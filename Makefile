PROGNAME = Sandtris
OFILES := $(shell find . -name '*.c' | sed 's/\.c/\.o/g')

.PHONY: all
all: $(PROGNAME).gba
	@echo "[FINISH] Created $(PROGNAME).gba"

GCC_VERSION = $(shell arm-none-eabi-gcc -dumpversion)
LINKSCRIPT_DIR = ./cs2110-tools

CROSS   := arm-none-eabi-
AS      := $(CROSS)as
CC      := $(CROSS)gcc
LD      := $(CROSS)ld
OBJCOPY := $(CROSS)objcopy

ARMINC = /usr/arm-none-eabi/include
ARMLIB = /usr/arm-none-eabi/lib
GCCLIB = /usr/lib/gcc/arm-none-eabi/$(GCC_VERSION)

CRELEASE = -O2
LDRELEASE = -s

MODEL   = -mthumb-interwork -mthumb
CFLAGS  = -Wall -Werror -std=c99 -pedantic -Wextra -fno-common $(MODEL) -mlong-calls -I $(ARMINC) -g
LDFLAGS = -nostartfiles -lc -lgcc -L $(ARMLIB) \
	  -L $(ARMLIB)/thumb \
	  -L $(GCCLIB) \
	  -T $(LINKSCRIPT_DIR)/arm-gba.ld

crt0.o : $(LINKSCRIPT_DIR)/crt0.s
	@$(AS) $(MODEL) $^ -o crt0.o

LDFLAGS += --specs=nosys.specs

# Adjust default compiler warnings and errors
CFLAGS += -Wstrict-prototypes -Wold-style-definition -Werror=vla
CFLAGS += -O2

$(PROGNAME).gba: clean $(PROGNAME).elf
	@echo "[LINK] Linking objects together to create $(PROGNAME).gba"
	@$(OBJCOPY) -O binary $(PROGNAME).elf $(PROGNAME).gba

$(PROGNAME).elf: crt0.o $(GCCLIB)/crtbegin.o $(GCCLIB)/crtend.o $(GCCLIB)/crti.o $(GCCLIB)/crtn.o $(OFILES)
	$(CC) -o $(PROGNAME).elf $^ $(LDFLAGS)

.PHONY: clean
clean:
	@echo "[CLEAN] Removing all compiled files"
	rm -f *.o *.elf *.gba *.log */*.o *.sav */*/*.sav client
