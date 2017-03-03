#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<errno.h>

int main(int argc, char *argv[]) {
	int sock;
	int port;
	char *server;
	unsigned char buf[1024];
	struct sockaddr_in server_addr;
	struct sockaddr_in my_addr;
	FILE *in;
	int len;
	
	server = argv[1];
	port = atoi(argv[2]);

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(server);

	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) < 0) {
		fprintf(stderr, "problem connecting: %d\n", errno);
	}

	in = fdopen(0, "r");
	while (1) {
		fgets(buf, 1024, in);
		len = strlen(buf);
		send(sock, buf, len, 0);
	}
}
