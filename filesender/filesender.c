#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>

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

int main(int argn, char** argv) {
    if (argn != 3) {
        printf("usage: "
               "filesender <port> <filename>\n");
        return 0;
    }
    if (str_to_port(argv[1]) == -1) {
        printf("wrong port\n");
        return 0;
    }

    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
        err("socket");
    //printf("listener = %d\n", listener);
    int one = 1;
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1)
        err("setsockopt");
    struct addrinfo* info;
    if (getaddrinfo("localhost", argv[1], NULL, &info) == -1)
        err("getaddrinfo");
    if (bind(listener, info->ai_addr, info->ai_addrlen) == -1)
        err("bind");
    freeaddrinfo(info);
    if (listen(listener, 1) == -1)
        err("listen");

    struct sockaddr_in client;
    socklen_t sz;
    int clientfd;
    while (1) {
        sz = sizeof(client);
        clientfd = accept(listener, (struct sockaddr*)&client, &sz);
        //printf("accept = %d\n", fd);
        //printf("from %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        if (clientfd == -1)
            err("accept");

        if (fork() == 0) {
            close(listener);
            int file = open(argv[2], O_RDONLY);
            if (file == -1)
                err("open file");
            struct buf_t* buf = buf_new(4096);
            if (buf == NULL)
                err("create buffer");
            ssize_t read_size;
            while (1) {
                read_size = buf_fill(file, buf, buf_capacity(buf));
                if (read_size == -1)
                    err("read file");
                if (read_size == 0)
                    break;
                if (buf_flush(clientfd, buf, read_size) == -1)
                    err("write");
            }
            close(file);
            close(clientfd);
            buf_free(buf);
            return 0;
        }
        close(clientfd);
    }
    return 0;
}
