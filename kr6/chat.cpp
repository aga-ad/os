#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>

using namespace std;

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
    if (res >= (1 << 16) || res == 0) {
        return -1;
    }
    return res;
}

void sigpipe_off(int sig) {}

struct Client {
    size_t last;
    char* nick;
    short fails;//fails: 0 - ok, 1 - read, 2 - write, 3 - read&write
    Client() {
        last = 0;
        fails = 0;
        nick = 0;
    }
    void kill() {
        if (nick != 0)
            free(nick);
        nick = 0;
    }
};

int main(int argn, char** argv) {
    const size_t CLIENT_LIMIT = 256;//client maximum is CLIENT_LIMIT - 1
    const size_t MAX_MESSAGE = 4096;
    const char* NEW_CLIENT_MESSAGE = "Welcome, ";

    if (argn != 2) {
        printf("usage: "
               "polling <port>\n");
        return 1;
    }
    if (str_to_port(argv[1]) == -1) {
        printf("wrong port\n");
        return 1;
    }

    struct sigaction sigact;
    memset(&sigact, '\0', sizeof(sigact));
    sigact.sa_handler = &sigpipe_off;
    if (sigaction(SIGPIPE, &sigact, NULL) < 0)
        err("sigaction");

    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == -1)
        err("sockets");
    int one = 1;
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1)
        err("setsockopt");
    struct addrinfo* info;
    if (getaddrinfo("0.0.0.0", argv[1], NULL, &info) == -1)
        err("getaddrinfo");
    if (bind(listener, info->ai_addr, info->ai_addrlen) == -1)
        err("bind");
    freeaddrinfo(info);
    if (listen(listener, 1) == -1)
        err("listen");

    size_t oldsize, i, j;
    ssize_t msgsize, size, readsize;
    struct pollfd fds[CLIENT_LIMIT];
    memset(fds, 0, (CLIENT_LIMIT) * sizeof(struct pollfd));
    fds[0].fd = listener;
    fds[0].events = POLLIN;
    vector<Client> clients;
    vector<char*> msgs;
    char buf[MAX_MESSAGE];
    while (1) {
        //printf("%zu clients\n", clients.size());
        if (poll(fds, clients.size() + 1, -1) == -1) {
            if (errno == EINTR) {
                printf("EINTR\n");
                continue;
            } else {
                err("poll");
            }
        }

        oldsize = clients.size();
        if (clients.size() + 1 < CLIENT_LIMIT && (fds[0].revents & POLLIN) > 0) {
            //printf("New client\n");
            fds[clients.size() + 1].fd = accept(fds[0].fd, NULL, NULL);
            if (fds[clients.size() + 1].fd == -1)
                err("accept");
            fds[clients.size() + 1].events = POLLIN;
            clients.push_back(Client());
            clients.back().last = msgs.size();
        }
        for (i = 1; i <= oldsize; i++) {
            if ((fds[i].revents & POLLOUT) > 0) {
                //printf("%zu POLLOUT\n", i - 1);
                for (j = clients[i - 1].last; j < msgs.size(); j++) {
                    if (write(fds[i].fd, msgs[j], strlen(msgs[j])) <= 0) {
                        shutdown(fds[i].fd, SHUT_WR);
                        clients[i - 1].fails |= 2;
                        //printf("huevo write\n");
                        break;
                    }
                }
                clients[i - 1].last = msgs.size();
            }
        }
        for (i = 1; i <= oldsize; i++) {
            if ((fds[i].revents & POLLIN) > 0) {
                //printf("%zu POLLIN\n", i - 1);
                if (clients[i - 1].nick == 0) {
                    // get nick
                    readsize = read(fds[i].fd, buf, MAX_MESSAGE);
                    if (readsize <= 0) {
                        shutdown(fds[i].fd, SHUT_RD);
                        clients[i - 1].fails |= 3; //can't write nick - so go away
                        continue;
                    }
                    if (buf[readsize - 1] == '\n')
                        readsize--;
                    buf[readsize] = 0;
                    clients[i - 1].nick = strndup(buf, readsize);

                    if (clients[i - 1].nick == 0)
                        err("malloc");
                    //printf("%s logged in\n", clients[i - 1].nick);

                    msgs.push_back((char*)malloc(strlen(clients[i - 1].nick) + strlen(NEW_CLIENT_MESSAGE) + 3));
                    if (msgs.back() == 0)
                        err("malloc");
                    strcpy(msgs.back(), NEW_CLIENT_MESSAGE);
                    size = strlen(NEW_CLIENT_MESSAGE);
                    strcpy(msgs.back() + size, clients[i - 1].nick);
                    strcpy(msgs.back() + size + strlen(clients[i - 1].nick), "\n");
                    printf("%s", msgs.back());
                }
                else {
                    //get message
                    //printf("reading from client %zu...\n", i - 1);
                    msgsize = read(fds[i].fd, buf, MAX_MESSAGE);
                    if (msgsize <= 0) {
                        shutdown(fds[i].fd, SHUT_RD);
                        clients[i - 1].fails |= 1;
                        //printf("huevo read\n");
                        continue;
                    }
                    if (buf[msgsize - 1] == '\n')
                        msgsize--;
                    buf[msgsize] = 0;
                    //printf("%s\n", buf);
                    size = strlen(clients[i - 1].nick);
                    msgs.push_back((char*)malloc(msgsize + 4 + size));
                    if (msgs.back() == 0)
                        err("malloc");
                    strcpy(msgs.back(), clients[i - 1].nick);
                    strcpy(msgs.back() + size, ": ");
                    strncpy(msgs.back() + size + 2, buf, msgsize);
                    strcpy(msgs.back() + size + 2 + msgsize, "\n");
                    printf("%s", msgs.back());
                    //printf("reading finished %zu\n", msgsize);
                }
            }
        }
        for (i = 1; i <= oldsize; i++) {
            if ((fds[i].revents & POLLERR) > 0 || (fds[i].revents & POLLHUP) > 0) {
                clients[i - 1].fails = 3;
            }
        }
        for (i = 1; i <= clients.size(); i++) {
            fds[i].events = 0;
            if (clients[i - 1].last < msgs.size() && (clients[i - 1].fails & 2) == 0) {
                fds[i].events |= POLLOUT;
            }
            if ((clients[i - 1].fails & 1) == 0) {
                fds[i].events |= POLLIN;
            }
        }
        for (i = clients.size(); i > 0; i--) {
            if (clients[i - 1].fails == 3) {
                printf("BAD CLIENT\n");
                close(fds[i].fd);
                fds[i] = fds[clients.size()];
                clients[i - 1].kill();
                clients[i - 1] = clients.back();
                clients.pop_back();
            }
        }
    }
    return 0;
}
