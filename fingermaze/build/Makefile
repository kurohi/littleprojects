CC = gcc
CFLAGS = -g -Wall -I ../include
FPRINT = -lfprint
FPRINT_OBJ = imagem.o
LDFLAGS = -lm #$(FPRINT)
BIN = gooys
OBJ = enhance.o #$(FPRINT_OBJ)

vpath %.c ../src
vpath %.h ../include

$(BIN): $(OBJ)

.PHONY: clean
clean:
	rm -vf $(BIN) $(OBJ)

