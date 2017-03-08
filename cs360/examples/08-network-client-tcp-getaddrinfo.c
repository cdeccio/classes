#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<errno.h>
#include<sys/types.h>
#include<netdb.h>
#include<unistd.h>

int main(int argc, char *argv[]) {
	int sock;
	char *port;
	char *server;
	unsigned char buf[1024];
	struct addrinfo *p, *listp, hints;
	struct sockaddr_in my_addr;
	FILE *in;
	int len;
	int rc;
	
	server = argv[1];
	port = argv[2];

	/* Get a list of addrinfo records */
	memset(&hints, 0, sizeof(struct addrinfo));                         
	hints.ai_family = AF_UNSPEC;       /* IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM; /* Connections only */
	hints.ai_flags = AI_NUMERICSERV;
	if ((rc = getaddrinfo(server, port, &hints, &listp)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rc));
		_exit(1);
	}


	for (p = listp; p; p = p->ai_next) {
		sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (connect(sock, p->ai_addr, p->ai_addrlen) < 0) {
			fprintf(stderr, "problem connecting: %d\n", errno);
			close(sock);
		}
		break;
	} 
	freeaddrinfo(listp);

	in = fdopen(0, "r");
	while (1) {
		fgets(buf, 1024, in);
		len = strlen(buf);
		send(sock, buf, len, 0);
	}
}
