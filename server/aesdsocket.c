#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <syslog.h>

#define PORT "9000"  // the port users will be connecting to
#define BACKLOG 10
#define MAX_DAT 100  // Maximum no. of bytes receivable, +1
#define OUTPUT_FILE "/var/tmp/aesdsocketdata"

char buf[MAX_DAT];

static volatile int keepRunning = 1;

void write_stream(char* streamdata) {
    FILE* fstream = fopen(OUTPUT_FILE, "ab+");
    if (fstream != NULL) {
        fputs(streamdata, fstream);
        fclose(fstream);
    }
}

void sig_intrpt(int s) {
    keepRunning = 0;
    syslog(LOG_INFO, "Caught signal, exiting\n");
    remove(OUTPUT_FILE);
    printf("Removed data file from /var/tmp/");
    closelog();
    exit(0);
}

void sigchld_handler(int s) {
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void* get_in_addr(struct sockaddr* sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void handle_connection(int new_fd) {
    while (1) {
        memset(buf, 0, sizeof(buf));
        long int inc_bytes = recv(new_fd, buf, MAX_DAT - 1, 0);
        if (inc_bytes == -1) {
            exit(1);
        }
        write_stream(buf);
        if (buf[inc_bytes - 1] == '\n') {
            break;
        }
    }

    FILE* content_send = fopen(OUTPUT_FILE, "r");
    if (content_send != NULL) {
        char buf[MAX_DAT];
        while (fgets(buf, MAX_DAT, content_send) != NULL) {
            send(new_fd, buf, strnlen(buf, MAX_DAT), 0);
        }
        fclose(content_send);
    }
    syslog(LOG_INFO, "Closed connection\n");
    printf("Closed connection.\n");
    exit(0);
}

int main(int c, char** argv) {
    signal(SIGINT, sig_intrpt);
    signal(SIGTERM, sig_intrpt);

    while (keepRunning == 1) {
        openlog(NULL, 0, LOG_USER);
        int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
        struct addrinfo hints, *servinfo, *p;
        struct sockaddr_storage their_addr; // connector's address information
        socklen_t sin_size;
        struct sigaction sa;
        int yes = 1;
        int d_test;
        char s[INET6_ADDRSTRLEN];
        int rv;

        if (c == 2) {
            daemon(0, 0);
            d_test = strcmp(argv[1], "-d");
            if (d_test != 0) {
                printf("Daemon error\n");
            }
        } else {
            printf("Server running in daemon\n");
        }

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE; // use my IP

        if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
            return 1;
        }

        // loop through all the results and bind to the first one we can
        for (p = servinfo; p != NULL; p = p->ai_next) {
            if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
                perror("server: socket");
                continue;
            }

            if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
                perror("setsockopt");
                exit(1);
            }

            if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
                close(sockfd);
                perror("server: bind");
                continue;
            }

            break;
        }

        freeaddrinfo(servinfo); // all done with this structure

        if (p == NULL) {
            fprintf(stderr, "server: failed to bind\n");
            exit(1);
        }

        if (listen(sockfd, BACKLOG) == -1) {
            perror("listen");
            exit(1);
        }

        sa.sa_handler = sigchld_handler; // reap all dead processes
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        if (sigaction(SIGCHLD, &sa, NULL) == -1) {
            perror("sigaction");
            exit(1);
        }

        printf("server: waiting for connections...\n");

        while (1) {
            sin_size = sizeof their_addr;
            new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
            if (new_fd == -1) {
                perror("accept");
                continue;
            }

            inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr), s, sizeof s);
            printf("Accepted connection from %s\n", s);
            syslog(LOG_INFO, "Accepted connection from %s\n", s);

            if (!fork()) { // this is the child process
                close(sockfd); // child doesn't need the listener
                handle_connection(new_fd);
                close(new_fd);
            }

            close(new_fd); // parent doesn't need this
        }
    } // End of 1st while loop

    return 0;
}
