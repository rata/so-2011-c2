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
	struct sockaddr_in srv;
	socklen_t srv_len = sizeof(struct sockaddr_in);
	char buf[MAX_MSG_LENGTH];

	fd = socket(AF_INET, SOCK_DGRAM, 0);
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

	/* Mandamos mensajes hasta que alguno sea el que marca el final. */
	for (;;) {
		char *err = fgets(buf, MAX_MSG_LENGTH, stdin);
		if (err == NULL) {
			printf("Error al leer de stdin\n");
			r = -1;
			goto out;
		}

		ssize_t sent = sendto(fd, buf, strlen(buf) + 1, 0, (struct sockaddr *) &srv, srv_len);

		// XXX: checkear por sent = strlen(buf) + 1 ?
		if (sent == -1) {
			perror("sendto");
			r = -1;
			goto out;
		}

		r = strncmp(buf, END_STRING, strlen(END_STRING));
		if (r == 0) {
			// Notar que r == 0, asique devolvemos 0
			break;
		}
	}

out:
	close(fd);
	return r;
}
