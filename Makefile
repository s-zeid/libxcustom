NAME = xcustom
CFLAGS = `pkg-config --cflags x11`
LDFLAGS = `pkg-config --libs x11`

all: lib${NAME}.so

32: lib${NAME}32.so
64: lib${NAME}.so

lib${NAME}.so: lib${NAME}-native.so
	mv $^ $@

lib${NAME}%.so: main.c
	$(CC) $(CFLAGS) -Wall -fPIC -DPIC -c main.c -o ${NAME}.o \
	 $(if $(filter 32,$*),-m$*,)
	$(LD) $(LDFLAGS) -ldl -shared ${NAME}.o -o $@ \
	 $(if $(filter 32,$*),-m elf_i386,)
	rm -f ${NAME}.o

.PHONY: clean

clean:
	rm -f lib${NAME}*.so*
