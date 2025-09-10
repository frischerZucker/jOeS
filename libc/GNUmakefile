CFLAGS?=-O2 -g
CPPFLAGS?=
LDFLAGS?=
LIBS?=

# User controllable toolchain and toolchain prefix.
TOOLCHAIN :=
TOOLCHAIN_PREFIX :=
ifneq ($(TOOLCHAIN),)
    ifeq ($(TOOLCHAIN_PREFIX),)
        TOOLCHAIN_PREFIX := $(TOOLCHAIN)-
    endif
endif

# User controllable C compiler command.
ifneq ($(TOOLCHAIN_PREFIX),)
    CC := $(TOOLCHAIN_PREFIX)gcc
else
    CC := cc
endif

# User controllable linker command.
LD := $(TOOLCHAIN_PREFIX)ld

# Defaults overrides for variables if using "llvm" as toolchain.
ifeq ($(TOOLCHAIN),llvm)
    CC := clang
    LD := ld.lld
endif

DESTDIR?=
PREFIX?=usr/
EXEC_PREFIX?=$(PREFIX)
INCLUDEDIR?=$(PREFIX)/include
LIBDIR?=$(EXEC_PREFIX)/lib

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra -I../sysroot/usr/include 
CPPFLAGS:=$(CPPFLAGS) -D__is_libc -I../sysroot/usr/include 
LIBK_CFLAGS:=$(CFLAGS)
LIBK_CPPFLAGS:=$(CPPFLAGS) -D__is_libk

CFLAGS:=$(CFLAGS) $(ARCH_CFLAGS)
CPPFLAGS:=$(CPPFLAGS) $(ARCH_CPPFLAGS)
LIBK_CFLAGS:=$(LIBK_CFLAGS) $(KERNEL_ARCH_CFLAGS) -m64 -mcmodel=kernel -fno-pic
LIBK_CPPFLAGS:=$(LIBK_CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS) -m64 -mcmodel=kernel -fno-pic

BINDIR := bin
OBJDIR := obj

SRCFILES :=  $(shell find -L -type f 2>/dev/null)

CFILES := $(filter %.c, $(SRCFILES))

FREEOBJS = $(CFILES:.c=.o)

HOSTEDOBJS=\
$(ARCH_HOSTEDOBJS) \

OBJS=\
$(FREEOBJS) \
$(HOSTEDOBJS) \

LIBK_OBJS=$(addprefix $(OBJDIR)/, $(FREEOBJS:.o=.libk.o))
OBJS := $(addprefix $(OBJDIR)/, $(OBJS))

#BINARIES=libc.a libk.a # Not ready for libc yet.
BINARIES=libk.a

# .PHONY: all clean install install-headers install-libs
.PHONY: all
.SUFFIXES: .o .libk.o .c .S

all: bin/$(BINARIES)

$(BINDIR)/libc.a: $(OBJS)
	@mkdir --parents $(dir $@)
	$(AR) rcs $@ $(OBJS)

$(BINDIR)/libk.a: $(LIBK_OBJS)
	@mkdir --parents $(dir $@)
	$(AR) rcs $@ $(LIBK_OBJS)

$(OBJDIR)/%.o: %.c
	@mkdir --parents $(dir $@)
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)

$(OBJDIR)/%.libk.o: %.c
	@mkdir --parents $(dir $@)
	$(CC) -MD -c $< -o $@ -std=gnu11 $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

$(OBJDIR)/%.o: %.S
	@mkdir --parents $(dir $@)
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

$(OBJDIR)/%.libk.o: %.S
	@mkdir --parents $(dir $@)
	$(CC) -MD -c $< -o $@ $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

clean:
	rm -rf $(OBJDIR) $(BINDIR)

-include $(OBJS:.o=.d)
-include $(LIBK_OBJS:.o=.d)