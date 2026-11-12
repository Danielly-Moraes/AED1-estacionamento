CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -pedantic -O2
SOURCES = main.c veiculo.c lista_estacionamento.c fila_espera.c pilha_historico.c persistencia.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = estacionamento

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
