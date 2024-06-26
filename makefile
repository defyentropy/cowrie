# Based on this StackOverflow answer: https://stackoverflow.com/a/30602701/16396198

CC := gcc

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

OUT := $(BIN_DIR)/cwsh
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# https://gcc.gnu.org/onlinedocs/gcc/Preprocessor-Options.html
CPPFLAGS := -Iinclude -MMD -MP
CFLAGS := -g -Wall

.PHONY: all clean

all: $(OUT)

$(OUT): $(OBJ) | $(BIN_DIR)
	$(CC) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)

-include $(OBJ:.o=.d)
