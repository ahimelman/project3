### Makefile for Project 3, COS 318


# C compiler and linker
CC = gcc
LD = ld

# Entry points of the kernel and the processes
KERNEL_ADDR = 0x1000
PROC1_ADDR = 0x10000
PROC2_ADDR = 0x20000

## Assembler flags
# 32-bit code generation
ASFLAGS += -m32

## C compiler flags
# Normally, we would select c99, but we need inline assembly
CFLAGS += -std=gnu99
# 32-bit code generation
CFLAGS += -m32
# No external environment
CFLAGS += -ffreestanding
# Turn on all warnings and make them errors
CFLAGS += -Wall #-Wextra -Werror
# Turn on optimization
CFLAGS += -O2 -fomit-frame-pointer
# -fstack-protector is not compatible with -ffreestanding
CFLAGS += -fno-stack-protector
# -funit-at-time reorders functions---we need the entry point to be first in the object file
CFLAGS += -fno-unit-at-a-time
# Specify the memory layout with #defines
CFLAGS += -DKERNEL_ADDR=$(KERNEL_ADDR)
CFLAGS += -DPROC1_ADDR=$(PROC1_ADDR)
CFLAGS += -DPROC2_ADDR=$(PROC2_ADDR)
CFLAGS += -DPROC3_ADDR=$(PROC3_ADDR)
# Add debugging symbols
CFLAGS += -g

## Linker flags
# 32-bit code generation
LDFLAGS += -melf_i386
# Don't use the standard startup files or libraries
LDFLAGS += -nostdlib
# Specify a location on a per-target basis
LDFLAGS += -Ttext


## First Makefile target
# Create a 1.44M floppy image for bochs etc.
floppy.img : image
	dd if=/dev/zero of=floppy.img bs=512 count=2880
	dd if=image of=floppy.img conv=notrunc
# Create a generic image to put on a boot medium
image : createimage bootblock kernel process1 process2 
	./createimage --extended bootblock kernel process1 process2 

createimage : createimage.c
	$(CC) -Wall $^ -o $@

# Put the boot block at the specified address
bootblock : LDFLAGS += 0x0000
bootblock : bootblock.o

kernel : LDFLAGS += $(KERNEL_ADDR)
# Important: the entry point is in kernel.o, so it must come first
kernel : kernel.o entry.o interrupt.o queue.o scheduler.o sync.o util.o printf.o barrier_test.o philosophers.o th1.o th2.o

process1 : LDFLAGS += $(PROC1_ADDR)
# Important: the entry point is in process1.o, so it must come first
process1 : process1.o syslib.o util.o printf.o

process2 : LDFLAGS += $(PROC2_ADDR)
process2 : process2.o syslib.o util.o printf.o helper.o


# Override the default implicit rule to call $(LD) directly
% : %.o
	$(LD) $(LDFLAGS) $^ -o $@


# Copy the image to the USB drive in the fishbowl lab
# WARNING: make sure this is right before you make this target on another system
boot : image
	cat image > /dev/sdg

# Dependency management
depend : .depend
.depend :
	$(CC) -MM *.c *.S > $@

# Clean up!
clean :
	-rm -f floppy.img image createimage bootblock kernel process1 process2  *.o .depend serial.out log.out
	# your code here
distclean : clean
	-find . '(' -name '*~' -o -name '\#*' -o -name '*.bak' -o -lname '*' ')' -print0 | xargs -0 rm -f
	-rm -f *~ \#* *.bak bochsout.txt

.PHONY : boot depend clean distclean

# Inform make of the dependencies found by gcc
# The dash causes make to make .depend if it doesn't exist
-include .depend

# your code here
