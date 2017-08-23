.PHONY: all clean
objs := dbm.o calchash.o datum.o delete.o store.o bits.o key.o fetch.o

all: $(objs)

%.o: %.c
	$(CC) -c -Wall $< -o $@

clean:
	$(RM) -r $(objs)
