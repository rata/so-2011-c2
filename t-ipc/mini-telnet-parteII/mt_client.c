#include "mt.h"
#include <stdio.h>

int main(int argc, char **argv)
{
	if (argc != 2) {
		printf("Usage %s SERVER IP\n", argv[0]);
		return -1;
	}

	int fd;
	int r = 0;
	char buf[MAX_MSG_LENGTH];
	struct sockaddr_in srv;
	socklen_t srv_len = sizeof(srv);

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		perror("socket");
		return -1;
	}

	srv.sin_family = AF_INET;
	srv.sin_port = htons(PORT);
	r = inet_aton(argv[1], (struct in_addr *) &srv.sin_addr);
	if (r == 0) {
		printf("inet_aton error!\n");
		r = -1;
		goto out;
	}

	r = connect(fd, (struct sockaddr *) &srv, srv_len);
	if (r == -1) {
		perror("connect");
		goto out;
	}

	/* Mandamos mensajes hasta que alguno sea el que marca el final. */
	for (;;) {
		char *err = fgets(buf, MAX_MSG_LENGTH, stdin);
		if (err == NULL) {
			printf("Error al leer de stdin\n");
			r = -1;
			goto out;
		}

		// Mando el tamaño del paquete primero
		size_t len = strlen(buf) + 1;
		uint16_t buf_len = htons(len);
		r = send_total(fd, &buf_len, sizeof(buf_len), 0);
		if (r == -1)
			goto out;

		// Mandamos el paquete
		r = send_total(fd, buf, len, 0);
		if (r == -1)
			goto out;

		r = strncmp(buf, END_STRING, strlen(END_STRING));
		if (r == 0) {
			printf("Chaus!\n");
			goto out;
		}

		// Esperamos la salida del comando
		char cmd_output[MAX_MSG_LENGTH];
		memset(cmd_output, 0, sizeof(cmd_output));

		// Me fijo el tamaño del paquete
		uint16_t pkt_size;
		r = recv_total(fd, &pkt_size, sizeof(pkt_size), 0);

		/* Check that the buffer is long enough */
		pkt_size = ntohs(pkt_size);
		if (MAX_MSG_LENGTH < pkt_size) {
			printf("paquete mayor al maximo!\n");
			r = -1;
			goto out;
		}
		
		/* Receive the packet */
		r = recv_total(fd, cmd_output, pkt_size, 0);
		if (r)
			goto out;

		printf("La salida del comando es: %s\n", cmd_output);

	}

out:
	close(fd);
	return r;
}

