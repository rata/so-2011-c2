#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define PORT 5555
#define MAX_MSG_LENGTH 1024
#define END_STRING "chau\n"
#define COMPLETE_STRING "fin-respuesta"


int send_total(int s, const void *buf, size_t len, int flags)
{
        ssize_t sent;
        while (len) {
                sent = send(s, buf, len, flags);
                if (sent == -1) {
                        perror("send");
                        return -1;
                }
                buf = (char *) buf + sent;
                len -= sent;
        }
        return 0;
}

int recv_total(int s, void *buf, size_t len, int flags)
{
        ssize_t rec;
        while (len) {
                rec = recv(s, buf, len, flags);
                if (rec == -1) {
                        perror("recv");
                        return -1;
                }
                buf = (char *) buf + rec;
                len -= rec;
        }
        return 0;
}
