#
# Makefile for libulz (stolen from musl) (requires GNU make)
#
# Use config.mak to override any of the following variables.
# Do not make changes here.
#

exec_prefix = /usr/local
bindir = $(exec_prefix)/bin

prefix = /usr/local/
includedir = $(prefix)/include
libdir = $(prefix)/lib

FLASH_SRCS = $(sort $(wildcard flashlib/*.c))
CORE_SRCS = $(sort $(wildcard neoart/flod/core/*.c))
WHITTAKER_SRCS = $(sort $(wildcard neoart/flod/whittaker/*.c))

FILELOADER_SRCS = neoart/flod/FileLoader.c
LAUNCHER_SRCS = demos/Demo5.c

SRCS = $(FLASH_SRCS) $(CORE_SRCS) $(WHITTAKER_SRCS) $(FILELOADER_SRCS) $(LAUNCHER_SRCS)

OBJS = $(SRCS:.c=.o)

CFLAGS += -O0 -g 
AR      = $(CROSS_COMPILE)ar
RANLIB  = $(CROSS_COMPILE)ranlib
OBJCOPY = $(CROSS_COMPILE)objcopy

#ALL_INCLUDES = $(sort $(wildcard include/*.h include/*/*.h))

#ALL_LIBS = libflod.a 
ALL_TOOLS = flod_demo.out

-include config.mak

all: $(ALL_LIBS) $(ALL_TOOLS)

install: $(ALL_LIBS:lib/%=$(DESTDIR)$(libdir)/%) $(ALL_INCLUDES:include/%=$(DESTDIR)$(includedir)/%) $(ALL_TOOLS:tools/%=$(DESTDIR)$(bindir)/%)

clean:
	rm -f $(OBJS)
	rm -f $(ALL_TOOLS)
#	rm -f $(LOBJS)
#	rm -f $(ALL_LIBS) lib/*.[ao] lib/*.so

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

flod_demo.out: $(OBJS)
	$(CC) -o $@ $(OBJS)

libflod.a: $(OBJS)
	rm -f $@
	$(AR) rc $@ $(OBJS)
	$(RANLIB) $@

lib/%.o:
	cp $< $@

$(DESTDIR)$(bindir)/%: tools/%
	install -D $< $@

$(DESTDIR)$(prefix)/%: %
	install -D -m 644 $< $@

.PHONY: all clean install
