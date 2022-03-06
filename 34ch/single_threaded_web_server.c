#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "helpers.h"
#define BACKLOG 10
#define MAXLINE 100

static void process_request(int connfd);
static _Noreturn void handler(int signum);
unsigned int processed = 0;
static ssize_t rio_writen(int fd, void* usrbuf, size_t n);

int main(int argc, char* argv[argc+1]) {
    long port = 0L;
    char hostname[MAXLINE] = { 0 };
    char clientport[MAXLINE] = { 0 };
    int listenfd = 0, connfd = 0, tmperr = 0;
    struct sockaddr_in6* address = { 0 };
    struct sockaddr_storage clientaddr = { 0 };
    struct sockaddr_storage ss = { 0 };
    socklen_t clientlen = 0;
    size_t addrlen = 0;
    struct sigaction sa = { 0 };

    if (argc != 2) {
        fputs("Usage: single_threaded_web_server PORT\n", stderr);
        exit(EXIT_FAILURE);
    }
    port = num_convert(argv[1]); 
    sa.sa_handler = handler;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = SA_RESTART;
   if (sigaction(SIGINT, &sa, NULL) == -1) {
        tmperr = errno;
        print_error(tmperr);
        exit(EXIT_FAILURE);
    }
    if ((listenfd = socket(AF_INET6, SOCK_STREAM, 0)) == 0) {
        tmperr = errno;
        print_error(tmperr);
        exit(EXIT_FAILURE);
    }
    address = (struct sockaddr_in6*) &ss;
    address->sin6_family = AF_INET6;
    address->sin6_port = htons(port);
    inet_pton(AF_INET6, "::1", &address->sin6_addr);
    addrlen = sizeof(struct sockaddr_in6);
    if (bind(listenfd, (struct sockaddr*) address, addrlen)) {
        tmperr = errno;
        print_error(tmperr);
        exit(EXIT_FAILURE);
    }
    if (listen(listenfd, BACKLOG)) {
        tmperr = errno;
        print_error(tmperr);
        exit(EXIT_FAILURE);
    }
    while (1) {
        clientlen = sizeof(clientaddr);
        if ((connfd = accept(listenfd, (struct sockaddr*) &clientaddr, &clientlen)) < 0) {
            tmperr = errno;
            print_error(tmperr);
            exit(EXIT_FAILURE);
        }
        if ((tmperr = getnameinfo((struct sockaddr*) &clientaddr, clientlen, hostname, MAXLINE, clientport, MAXLINE, 0))) {
            fprintf(stderr, "Error in getnameinfo: %s.\n", gai_strerror(tmperr));
            exit(EXIT_FAILURE);
        }
        printf("Accepted connection from %s port %s\n", hostname, clientport);
        process_request(connfd);
        processed++;
        if (close(connfd)) {
            tmperr = errno;
            print_error(tmperr);
            exit(EXIT_FAILURE);
        }
    }
}

static void process_request(int connfd) {
    int tmperr = 0;
    size_t n = 0;
    struct timespec tp = { 0 };
    char response[MAXLINE] = { 0 };

    if (clock_gettime(CLOCK_REALTIME, &tp)){
        tmperr = errno;
        print_error(tmperr);
        exit(EXIT_FAILURE);
    }
    ctime_r(&tp.tv_sec, response); 
    n = sizeof(response);
    if ((size_t) rio_writen(connfd, &response, n) != n) {
            tmperr = errno;
            print_error(tmperr);
            exit(EXIT_FAILURE);
    }

}

static _Noreturn void handler(int signum) {
    printf("Got signal %d, shutting down. Requests processed: %u\n", signum, processed);
    exit(EXIT_SUCCESS);
}

// Copied from CS:APP "tiny" web server
// http://csapp.cs.cmu.edu/3e/tiny.tar
static ssize_t rio_writen(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = usrbuf;

    while (nleft > 0) {
        if ((nwritten = write(fd, bufp, nleft)) <= 0) {
            if (errno == EINTR)  /* Interrupted by sig handler return */
                nwritten = 0;    /* and call write() again */
            else
                return -1;       /* errno set by write() */
        }
        nleft -= nwritten;
        bufp += nwritten;
    }
    return n;
}
