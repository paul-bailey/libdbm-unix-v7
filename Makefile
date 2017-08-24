.PHONY: all clean

topsrcdir:=$(shell pwd)

CFLAGS += -I$(topsrcdir)/include
export CFLAGS

all clean:
	$(MAKE) -C libdbm $@
