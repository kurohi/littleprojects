CC = gcc
CFLAGS = -I ../include -g -Wall -D USEOPENCV

LDFLAGS = -lm -lfprint -lcv -lhighgui

OBJ = enhance.o image.o minutia.o matching.o crypto.o
TESTE = teste2.o
TAKEPHOTO = takephoto.o
STATISTIC = statistic.o
BIN = FPMatch

vpath %.h ../include
vpath %.c ../src

all: $(BIN)
$(BIN): $(OBJ)

teste: $(OBJ) $(TESTE)

takephoto: $(OBJ) $(TAKEPHOTHO)

statistic: $(OBJ) $(STATISTIC)

.PHONY: clean
clean:
	rm -vf $(BIN) $(OBJ) $(TESTE) $(TAKEPHOTO) 
