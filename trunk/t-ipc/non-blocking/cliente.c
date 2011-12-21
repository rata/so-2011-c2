#include "header.h"

int main(void)
{
    int s, len;
    struct sockaddr_un remote;
    char str[MENSAJE_MAXIMO];

	/* Crear un socket de tipo UNIX con TCP (SOCK_STREAM). */
    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("creando socket");
        exit(1);
    }

	/* Establecer la dirección a la cual conectarse. */
    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, SOCK_PATH);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);

   	/* Conectarse. */
    if (connect(s, (struct sockaddr *)&remote, len) == -1) {
        perror("conectandose");
        exit(1);
    }

	/* Establecer la dirección a la cual conectarse para escuchar. */
    while(printf("> "), fgets(str, MENSAJE_MAXIMO, stdin), !feof(stdin)) {
        if (send(s, str, strlen(str), 0) == -1) {
            perror("enviando");
            exit(1);
        }
    }

	/* Cerrar el socket. */
    close(s);

    return 0;
}



