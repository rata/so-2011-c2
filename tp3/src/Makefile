MPICC   = mpicc
MPIEXEC = mpiexec
CFLAGS += -Wall -std=c99 -D_XOPEN_SOURCE=500 -DNDEBUG
#CFLAGS += -Wall -std=c99 -D_XOPEN_SOURCE=500

EJECUTABLE = tp3

.PHONY : all new clean run

all : $(EJECUTABLE)

tp3 : tp3.c srv.c
	$(MPICC) $(CFLAGS) -o tp3 tp3.c srv.c

run : tp3
	$(MPIEXEC) -np 2 ./$(EJECUTABLE) '.' 2 50 500 >/dev/null

clean :
	rm -f $(EJECUTABLE)
	rm -f *.o

new : clean all
