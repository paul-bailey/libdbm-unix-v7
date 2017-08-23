.PHONY: all clean
objs := dbm.o

all: $(objs)

%.o: %.c
	$(CC) -c -Wall $< -o $@

clean:
	$(RM) -r $(objs)
