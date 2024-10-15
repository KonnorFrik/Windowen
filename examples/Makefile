CC = gcc

CFLAGS = -Wall -Wextra -std=c11 -Wpedantic
CFLAGS += -g 

LDFLAGS = -lncurses

SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)
NAMES = $(SRC:.c=)

LIB_DIR = ..
LIB_SRC = $(wildcard $(LIB_DIR)/*.c)
LIB_OBJ = $(LIB_SRC:.c=.o)

all: $(NAMES)
	@echo "THE ALL TARGET"

# %.o: %.c
# 	$(CC) $(CFLAGS) -c $^ -o

paint: paint.o $(LIB_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

show:
	@echo $(LIB_SRC)
	@echo $(LIB_OBJ)
	@echo $(NAMES)

clean:
	$(RM) $(OBJ)
	$(RM) $(NAMES)
	$(RM) $(LIB_OBJ)
