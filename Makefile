CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
LDFLAGS =

SRC_DIR = src
INCLUDE_DIR = include
TEST_DIR = tests
BIN_DIR = bin
DATA_DIR = data

SOURCES = $(SRC_DIR)/data_reader.c
OBJECTS = $(SOURCES:.c=.o)
TEST_SOURCES = $(TEST_DIR)/test_data_reader.c
TEST_EXEC = $(BIN_DIR)/test_data_reader

.PHONY: all clean test

all: $(TEST_EXEC)

$(TEST_EXEC): $(OBJECTS) $(TEST_SOURCES)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJECTS) $(TEST_SOURCES) -o $@ $(LDFLAGS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_EXEC)
	./$(TEST_EXEC)

clean:
	rm -rf $(BIN_DIR) $(SRC_DIR)/*.o