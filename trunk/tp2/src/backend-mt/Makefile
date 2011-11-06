CC = g++
CFLAGS = -g -Wall -Werror
LDFLAGS = -lpthread

BINARIES = backend

all: $(BINARIES)

backend: Backend_mono.cpp Backend_mono.h Enviar_recibir.o Enviar_recibir.h Encabezado.h Casillero.h RWLock.o
	$(CC) $(LDFLAGS) $(CFLAGS) Backend_mono.cpp Enviar_recibir.o RWLock.o -o backend

Enviar_recibir.o: Enviar_recibir.cpp Enviar_recibir.h Encabezado.h
	$(CC) $(CFLAGS) -c Enviar_recibir.cpp

test-rwlock: RWLock.o test-rwlock.o
	$(CXX) $(LDFLAGS) $(CFLAGS) $^ -o $@

clean:
	rm -f *.o
	rm -f $(BINARIES)
