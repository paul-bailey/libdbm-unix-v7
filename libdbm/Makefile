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

all: $(objs)

%.o: %.c
	$(CC) -c -Wall $< -o $@

clean:
	$(RM) -r $(objs)
