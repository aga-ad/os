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
               "forking <port 1> <port 2>\n");
        return 0;
    }
    if (str_to_port(argv[1]) == -1 || str_to_port(argv[2]) == -1) {
        printf("wrong port\n");
        return 0;
    }

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
    if (getaddrinfo("localhost", argv[2], NULL, &info) == -1)
        err("getaddrinfo");
    if (bind(listener2, info->ai_addr, info->ai_addrlen) == -1)
        err("bind");
    freeaddrinfo(info);
    if (listen(listener1, 1) == -1 || listen(listener2, 1) == -1)
        err("listen");

    struct sockaddr_in client1, client2;
    socklen_t sz;
    int clientfd1, clientfd2;
    while (1) {
        sz = sizeof(client1);
        clientfd1 = accept(listener1, (struct sockaddr*)&client1, &sz);
        sz = sizeof(client2);
        clientfd2 = accept(listener2, (struct sockaddr*)&client2, &sz);

        if (clientfd1 == -1 || clientfd2 == -1)
            err("accept");

        if (fork() == 0) {
            close(listener1);
            close(listener2);
            struct buf_t* buf = buf_new(4096);
            if (buf == NULL)
                err("create buffer");
            ssize_t read_size;
            while (1) {
                read_size = buf_fill_something(clientfd1, buf);
                if (read_size == -1)
                    err("read file");
                if (read_size == 0)
                    break;
                if (buf_flush(clientfd2, buf, read_size) == -1)
                    err("write");
            }
            close(clientfd1);
            close(clientfd2);
            buf_free(buf);
            return 0;
        }

        if (fork() == 0) {
            close(listener1);
            close(listener2);
            struct buf_t* buf = buf_new(4096);
            if (buf == NULL)
                err("create buffer");
            ssize_t read_size;
            while (1) {
                read_size = buf_fill_something(clientfd2, buf);
                if (read_size == -1)
                    err("read file");
                if (read_size == 0)
                    break;
                if (buf_flush(clientfd1, buf, read_size) == -1)
                    err("write");
            }
            close(clientfd1);
            close(clientfd2);
            buf_free(buf);
            return 0;
        }

        close(clientfd1);
        close(clientfd2);
    }
    return 0;
}
