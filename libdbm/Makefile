.PHONY: all clean
objs := \
 forder.o \
 calchash.o \
 datum.o \
 delete.o \
 store.o \
 bits.o \
 key.o \
 fetch.o \
 dbminit.o

CFLAGS += -Wall -fvisibility=hidden

all: $(objs)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	$(RM) -r $(objs)
