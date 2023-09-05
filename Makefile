CC = gcc
CFLAGS =
LDFLAGS = -pthread

SRC = ex3.c
OBJ = $(SRC:.c=.o)

OUTPUT = ex3.out

all: $(OUTPUT)

$(OUTPUT): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(OUTPUT)