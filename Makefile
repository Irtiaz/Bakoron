CC=gcc
CFLAGS=-Wall -Wextra -Werror -pedantic  -ggdb
ASAN_FLAGS=-fsanitize=address,null,undefined,leak,alignment

BAKORON=lib/bakoron/libbakoron.a

all: $(BAKORON) $(patsubst examples/%/, build/%, $(wildcard examples/*/))

$(BAKORON): build/bakoron/bakoron.o build/bakoron/stb_ds.o | lib/bakoron
	ar rcs $@ $^

build/bakoron/bakoron.o: src/bakoron.c include/bakoron/bakoron.h | build/bakoron
	$(CC) $(CFLAGS) -Iinclude/bakoron -Iinclude/stb_ds -c $< -o $@

build/bakoron/stb_ds.o: src/stb_ds.c include/stb_ds/stb_ds.h | build/bakoron
	$(CC) $(CFLAGS) -Iinclude/stb_ds -c $< -o $@

build/%: examples/%/* $(BAKORON) | build
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -Iinclude/bakoron -Iexamples/$* examples/$*/*.c -o $@ $(BAKORON)

build:
	mkdir -p $@

lib/bakoron:
	mkdir -p $@

build/bakoron:
	mkdir -p $@

clean:
	rm -rf build
	rm -rf lib
