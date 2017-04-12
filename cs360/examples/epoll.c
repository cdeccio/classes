/* $begin select */
#define MAXEVENTS 64
#define MAXLINE 1024

#include<errno.h>
#include<fcntl.h>
#include<netdb.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/epoll.h>
#include<sys/socket.h>

void command(void);

int main(int argc, char **argv) 
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_in serveraddr;
    struct sockaddr_storage clientaddr;
    int efd;
    struct epoll_event event;
    struct epoll_event *events;
    int i;
    int len;

    size_t n; 
    char buf[MAXLINE]; 

    char *server;
    int port;
    int optval;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    server = "0.0.0.0";
    port = atoi(argv[1]);

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = inet_addr(server);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr_in)) < 0) {
	fprintf(stderr, "problem binding: %d\n", strerror(errno));
    }
    optval = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    if (listen(listenfd, 10) < 0) {
	fprintf(stderr, "problem listening: %d\n", errno);
    }

    /* set fd to non-blocking */
    if (fcntl(listenfd, F_SETFL, fcntl(listenfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
        fprintf(stderr, "error setting socket option\n");
        exit(1);
    }

    if ((efd = epoll_create1(0)) < 0) {
        fprintf(stderr, "error creating epoll fd\n");
        exit(1);
    }

    event.data.fd = listenfd;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &event) < 0) {
        fprintf(stderr, "error adding event\n");
        exit(1);
    }

    event.data.fd = fileno(stdin);
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(efd, EPOLL_CTL_ADD, fileno(stdin), &event) < 0) {
        fprintf(stderr, "error adding event\n");
        exit(1);
    }

    /* Buffer where events are returned */
    events = calloc(MAXEVENTS, sizeof(event));

    while (1) {
        // wait for event to happen (no timeout)
        n = epoll_wait(efd, events, MAXEVENTS, -1);

        for (i = 0; i < n; i++) {
            if ((events[i].events & EPOLLERR) ||
                  (events[i].events & EPOLLHUP)) {
                /* An error has occured on this fd */
                fprintf (stderr, "epoll error\n");
                close(events[i].data.fd);
                continue;
            }

            printf("fd: %d %d\n", i, events[i].data.fd);

            if (fileno(stdin) == events[i].data.fd) { //line:conc:select:stdinready
                command(); /* Read command line from stdin */
            } else if (listenfd == events[i].data.fd) { //line:conc:select:listenfdready
                clientlen = sizeof(struct sockaddr_storage); 
                connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);

                // set socket to non-blocking
                if (fcntl(connfd, F_SETFL, fcntl(connfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
                    fprintf(stderr, "error setting socket option\n");
                    exit(1);
                }

                // add event to epoll file descriptor
                event.data.fd = connfd;
                event.events = EPOLLIN | EPOLLET;
                if (epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &event) < 0) {
                    fprintf(stderr, "error adding event\n");
                    exit(1);
                }
            } else { //line:conc:select:listenfdready
                len = recv(events[i].data.fd, buf, MAXLINE, 0);   
                send(events[i].data.fd, buf, len, 0);
            }
        }
    }
    free(events);
}

void command(void) {
    char buf[MAXLINE];
    if (!fgets(buf, MAXLINE, stdin))
    exit(0); /* EOF */
    printf("%s", buf); /* Process the input command */
}
/* $end select */
