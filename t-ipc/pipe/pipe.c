#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#define DATOS "la vaca es un poco flaca"
int main() {
	int pipes[2], child;
	/* Crear un pipe. */
	if (pipe(pipes) < 0) {
		perror("creando el pipe");
		exit(10);
	}
	if ((child = fork()) == -1)
		perror("realizando el fork");
	else if (child != 0) {
		/* El padre lee lo que manda su hijo a través del pipe. */
		char buf[1024];
		/* Se cierra el socket de escritura porque no lo usamos en el padre. */
		close(pipes[1]);
		/* Leemos los datos que manda el hijo desde el pipe. */
		if (read(pipes[0], buf, 1024) < 0)
			perror("leyendo datos");
		/* Escribimos en pantalla la cantidad de bytes leídos. */
		printf("caracteres de la frase \"%s\": %d\n", buf, (int) strlen(buf));
		/* Cerramos el pipe de lectura. */
		close(pipes[0]);
	} else {
		/* El hijo le manda datos a su padre a través del pipe. */
		/* Se cierra el socket de lectura porque no lo usamos en el padre. */
		close(pipes[0]);
		/* Escribimos los datos que le mandamos al padre en el pipe. */
		if (write(pipes[1], DATOS, sizeof(DATOS)) < 0)
			perror("escribiendo datos");
		/* Cerramos el pipe de escritura. */
		close(pipes[1]);
	}
	return 0;
}

