#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<errno.h>

int main(int argc, char *argv[]) {
	int sock, clientsock;
	int port;
	char *server;
	unsigned char buf[1024];
	unsigned char c;
	struct sockaddr_in server_addr;
	struct sockaddr_in peer_addr;
	FILE *out;
	socklen_t addrlen;
	int len;
	
	server = "0.0.0.0";
	port = atoi(argv[1]);

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(server);

	sock = socket(AF_INET, SOCK_DGRAM, 0);

	if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) < 0) {
		fprintf(stderr, "problem binding: %d\n", errno);
	}

	out = fdopen(1, "w");
	while (1) {
		addrlen = sizeof(peer_addr);
		if ((len = recvfrom(sock, buf, 1024, 0, (struct sockaddr *)&peer_addr, &addrlen)) < 0) {
			fprintf(stderr, "problem receiving: %d\n", errno);

		}
		buf[len] = 0;
		fputs(buf, out);
	}
}
