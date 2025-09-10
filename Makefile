CC=gcc
CFLAGS=-Wall -Wextra -Werror -ansi -pedantic  -ggdb
ASAN_FLAGS=-fsanitize=address,null,undefined,leak,alignment

all: lib/bakoron/libbakoron.a $(patsubst examples/%/, build/%, $(wildcard examples/*/))

lib/bakoron/libbakoron.a: build/bakoron/bakoron.o build/bakoron/stb_ds.o | lib/bakoron
	ar rcs $@ $^

build/bakoron/bakoron.o: src/bakoron.c include/bakoron/bakoron.h | build/bakoron
	$(CC) $(CFLAGS) -Iinclude/bakoron -Iinclude/stb_ds -c $< -o $@

build/bakoron/stb_ds.o: src/stb_ds.c include/stb_ds/stb_ds.h | build/bakoron
	$(CC) $(CFLAGS) -Iinclude/stb_ds -c $< -o $@

build/%: examples/%/* lib/bakoron/libbakoron.a | build
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -Iinclude/bakoron -Iexamples/$* examples/$*/*.c -o $@ lib/bakoron/libbakoron.a

build:
	mkdir -p $@

lib/bakoron:
	mkdir -p $@

build/bakoron:
	mkdir -p $@

clean:
	rm -rf build
	rm -rf lib
