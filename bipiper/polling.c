#define _GNU_SOURCE
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <netdb.h>
#include <limits.h>
#include <signal.h>
#include <poll.h>
#include <errno.h>

#include "bufio.h"

#define err(s) {perror(s);return 1;}

int str_to_port(char* s) {
    ssize_t res = 0;
    while (*s != '\0') {
        if (*s < '0' || *s > '9') {
            return -1;
        }
        res = res * 10 + *s - '0';
        s++;
    }
    if (res > (1 << 16) || res == 0) {
        return -1;
    }
    return res;
}

void swapp(struct buf_t** a, struct buf_t** b) {
    struct buf_t* t;
    t = *a;
    *a = *b;
    *b = t;
}

void sigpipe_off(int sig) {

}

int main(int argn, char** argv) {
    if (argn != 3) {
        printf("usage: "
               "polling <port 1> <port 2>\n");
        return 0;
    }
    if (str_to_port(argv[1]) == -1 || str_to_port(argv[2]) == -1) {
        printf("wrong port\n");
        return 0;
    }

    struct sigaction sigact;
    memset(&sigact, '\0', sizeof(sigact));
    sigact.sa_handler = &sigpipe_off;

    if (sigaction(SIGPIPE, &sigact, NULL) < 0)
        err("sigaction");

    int listener1 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int listener2 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener1 == -1 || listener2 == -1)
        err("sockets");
    int one = 1;
    if (setsockopt(listener1, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1 || setsockopt(listener2, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1)
        err("setsockopt");
    struct addrinfo* info;
    if (getaddrinfo("localhost", argv[1], NULL, &info) == -1)
        err("getaddrinfo");
    if (bind(listener1, info->ai_addr, info->ai_addrlen) == -1)
        err("bind");
    freeaddrinfo(info);
    if (getaddrinfo("localhost", argv[2], NULL, &info) == -1)
        err("getaddrinfo");
    if (bind(listener2, info->ai_addr, info->ai_addrlen) == -1)
        err("bind");
    freeaddrinfo(info);
    if (listen(listener1, 1) == -1 || listen(listener2, 1) == -1)
        err("listen");

    struct sockaddr_in client;
    socklen_t sz;
    int clientfd, i;
    struct pollfd fds[256];
    memset(fds, 0, 256 * sizeof(struct pollfd));
    fds[0].fd = listener1;
    fds[0].events = POLLIN;
    fds[1].fd = listener2;
    fds[1].events = POLLIN;
    nfds_t nfds = 2;
    int polls, oldnfds;
    struct buf_t* bufs[127][2];
    for (i = 0; i < 127; i++) {
        bufs[i][0] = buf_new(4096);
        bufs[i][1] = buf_new(4096);
        if (bufs[i][0] == 0 || bufs[i][1] == 0) {
            err("buf malloc");
        }
    }
    short fails[256];//0 - ok, 1 - read, 2 - write, 3 - read&write
    int buf, number;
    while (1) {
        polls = poll(fds, nfds, -1);
        if (polls == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                err("poll");
            }
        }

        oldnfds = nfds;
        if (nfds <= 254 && (fds[0].revents & POLLIN) > 0 && (fds[1].revents & POLLIN) > 0) {
            sz = sizeof(client);
            clientfd = accept(listener1, (struct sockaddr*)&client, &sz);
            fds[nfds].fd = clientfd;
            fds[nfds].events = POLLIN;
            fails[nfds] = 0;
            buf_clear(bufs[nfds / 2][0]);
            buf_clear(bufs[nfds / 2][1]);
            nfds++;
            sz = sizeof(client);
            clientfd = accept(listener2, (struct sockaddr*)&client, &sz);
            fds[nfds].fd = clientfd;
            fds[nfds].events = POLLIN;
            fails[nfds] = 0;
            nfds++;
        }
        for (i = 2; i < oldnfds; i++) {
            if ((fds[i].revents & POLLIN) > 0) {
                buf = i / 2 - 1;
                number = i % 2;
                if (buf_fill_something(fds[i].fd, bufs[buf][number]) <= 0) {
                    shutdown(fds[i].fd, SHUT_RD);
                    fails[i] |= 1;
                    if (i % 2 == 0) {
                        fails[i + 1] |= 2;
                    } else {
                        fails[i - 1] |= 2;
                    }
                }
            }
        }
        for (i = 2; i < oldnfds; i++) {
            if ((fds[i].revents & POLLOUT) > 0) {
                buf = i / 2 - 1;
                number = i % 2;
                if (buf_flush(fds[i].fd, bufs[buf][1 - number], buf_size(bufs[buf][1 - number])) <= 0) {
                    shutdown(fds[i].fd, SHUT_WR);
                    fails[i] |= 2;
                    if (i % 2 == 0) {
                        fails[i + 1] |= 1;
                    } else {
                        fails[i - 1] |= 1;
                    }
                }
            }
        }
        for (i = oldnfds / 2 - 1; i >= 0; i--) {
            fds[2 + 2 * i].events = 0;
            fds[3 + 2 * i].events = 0;
            if (buf_size(bufs[i][0]) > 0 && (fails[3 + 2 * i] & 2) == 0) {
                fds[3 + 2 * i].events |= POLLOUT;
            }
            if (buf_size(bufs[i][0]) < buf_capacity(bufs[i][0]) && (fails[2 + 2 * i] & 1) == 0) {
                fds[2 + 2 * i].events |= POLLIN;
            }
            if (buf_size(bufs[i][1]) > 0 && (fails[2 + 2 * i] & 2) == 0) {
                fds[2 + 2 * i].events |= POLLOUT;
            }
            if (buf_size(bufs[i][1]) < buf_capacity(bufs[i][1]) && (fails[3 + 2 * i] & 1) == 0) {
                fds[3 + 2 * i].events |= POLLIN;
            }
        }
        for (i = 2; i < nfds; i += 2) {
            if (fails[i] == 3 || fails[i + 1] == 3 || (fails[i] == fails[i + 1] && (fails[i] == 1 || fails[i] == 2))) {
                close(fds[i].fd);
                close(fds[i + 1].fd);
                fds[i] = fds[nfds - 2];
                fds[i + 1] = fds[nfds - 1];
                fails[i] = fails[nfds - 2];
                fails[i + 1] = fails[nfds - 1];
                swapp(&bufs[(i - 2) / 2][0], &bufs[(nfds - 3) / 2][0]);
                swapp(&bufs[(i - 2) / 2][1], &bufs[(nfds - 3) / 2][1]);
                nfds -= 2;
            }
        }
    }
    return 0;
}
