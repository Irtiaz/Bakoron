CC=gcc
SRC=src
OBJ=obj
OUT=out
INCLUDE=include
TARGET=$(OUT)/bakoron
CFLAGS=-Wall -Wextra -Werror -ansi -pedantic  -g3 -I$(INCLUDE)
ASAN_FLAGS=-fsanitize=address,null,undefined,leak,alignment

all: $(TARGET)

$(TARGET): $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(wildcard $(SRC)/*.c)) | $(OUT)
	$(CC) $^ $(ASAN_FLAGS) -o $@

$(OBJ)/main.o: $(SRC)/main.c | $(OBJ)
	$(CC) -c $(SRC)/main.c $(CFLAGS) -o $@

$(OBJ):
	mkdir -p $(OBJ)

$(OUT):
	mkdir -p $(OUT)

run:
	./$(TARGET)

clean:
	rm -rf $(OBJ)/* $(OUT)/*
