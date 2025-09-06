CC=gcc
SRC=src
OBJ=obj
OUT=out
TARGET=$(OUT)/bakoron

all: $(TARGET)

$(TARGET): $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(wildcard $(SRC)/*.c)) | $(OUT)
	$(CC) $^ -o $@

$(OBJ)/main.o: $(SRC)/main.c | $(OBJ)
	$(CC) -c $(SRC)/main.c -o $@

$(OBJ):
	mkdir -p $(OBJ)

$(OUT):
	mkdir -p $(OUT)

run:
	./$(TARGET)

clean:
	rm -rf $(OBJ)/* $(OUT)/*
