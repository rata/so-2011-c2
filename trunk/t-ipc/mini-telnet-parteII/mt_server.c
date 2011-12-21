#include "mt.h"

int main()
{
	int r = 0;
	int sock;
	struct sockaddr_in name;
	char buf[MAX_MSG_LENGTH];

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		perror("socket");
		return -1;
	}

	int optval = 1;
	r = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if (r == -1) {
		perror("setsockopt");
		goto out;
	}

	name.sin_family = AF_INET;
	name.sin_addr.s_addr = INADDR_ANY;
	name.sin_port = htons(PORT);
	r = bind(sock, (void*) &name, sizeof(name));
	if (r == -1) {
		perror("bind");
		goto out;
	}

	r = listen(sock, 10);
	if (r == -1) {
		perror("listen");
		goto out;
	}

	int fd = accept(sock, NULL, NULL);
	if (fd == -1) {
		r = -1;
		goto out;
	}

	/* Recibimos mensajes hasta que alguno sea el que marca el final. */
	for (;;) {

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
		r = recv_total(fd, buf, pkt_size, 0);
		if (r)
			goto out;

		if (strncmp(buf, END_STRING, pkt_size) == 0) {
			r = 0;
			break;
		}

		printf("Comando: %s", buf);

		// Corremos el comando capturando stdout y stderr para mandarselo al cliente
		int cmd_stderr[2];
		r = pipe(cmd_stderr);
		if (r == -1) {
			perror("pipe");
			goto out;
		}

		r = dup2(cmd_stderr[1], 2);
		if (r == -1) {
			perror("dup2");
			goto out;
		}

		FILE *cmd_stdout = popen(buf, "r");
		if (cmd_stdout == NULL) {
			printf("opoen failed\n");
			goto out;
		}
		fflush(cmd_stdout);

		// Ponemos buf_stdout[0] en '\0' asi empieza a escribir desde el principio el fgets
		char buf_stdout[MAX_MSG_LENGTH];
		memset(buf_stdout, 0, sizeof(buf_stdout));
		buf_stdout[0] = '\0';
		while (fgets(buf_stdout + strlen(buf_stdout),
					MAX_MSG_LENGTH - strlen(buf_stdout), cmd_stdout) != NULL);


		// Lo hacemos no bloqueante para que no bloquee el read si no
		// hay nada salida de stderr
		int flags = fcntl(cmd_stderr[0], F_GETFL, 0);
		r = fcntl(cmd_stderr[0], F_SETFL, flags | O_NONBLOCK);
		if (r == -1) {
			perror("fcntl");
			goto out;
		}

		// Leemos la salida de stderr (si hay)
		char buf_stderr[MAX_MSG_LENGTH];
		memset(buf_stderr, 0, sizeof(buf_stderr));
		r = read(cmd_stderr[0], buf_stderr, sizeof(buf_stderr));
		if (r == -1 && errno != EWOULDBLOCK && errno != EAGAIN) {
			perror("read");
			goto out;
		}

		close(cmd_stderr[1]);
		close(cmd_stderr[0]);
		pclose(cmd_stdout);

		//printf("la salida de stdout es: %s\n", buf_stdout);
		//printf("la salida de stderr es: %s\n", buf_stderr);

		// Concatenamos la salida de stdout y stderr y se la mandamos al cliente
		strncat(buf_stdout, buf_stderr, MAX_MSG_LENGTH);
		//printf("La salida de stdout y stderr concatenada es: %s\n", buf_stdout);


		// Mando el tamaño del paquete primero
		size_t len = strlen(buf_stdout) + 1;
		uint16_t buf_stdout_len = htons(len);
		r = send_total(fd, &buf_stdout_len, sizeof(buf_stdout_len), 0);
		if (r == -1)
			goto out;

		// Mandamos la salida del comando ahora
		r = send_total(fd, buf_stdout, len, 0);
		if (r == -1)
			goto out;

	}

out:
	close(fd);
	close(sock);
	return r;
}
