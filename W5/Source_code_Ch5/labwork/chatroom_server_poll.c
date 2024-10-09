#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <errno.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

void broadcast_message(int sender_fd, struct pollfd *fds, int num_clients, char *message);

int main() {
    int server_fd, new_socket, i;
    struct sockaddr_in address;
    int opt = 1;
    struct pollfd fds[MAX_CLIENTS + 1];
    char buffer[BUFFER_SIZE];

    // Initialize all client sockets to 0
    for (i = 0; i < MAX_CLIENTS + 1; i++) {
        fds[i].fd = -1;
    }

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Define server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Listening on port %d...\n", PORT);

    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    while (1) {
        int activity = poll(fds, MAX_CLIENTS + 1, -1);

        if (activity < 0 && errno != EINTR) {
            perror("poll error");
        }

        // Check for new connections
        if (fds[0].revents & POLLIN) {
            socklen_t addrlen = sizeof(address);
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            printf("New connection, socket fd is %d, ip is : %s, port : %d\n", 
                   new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            // Add new socket to array of sockets
            for (i = 1; i < MAX_CLIENTS + 1; i++) {
                if (fds[i].fd == -1) {
                    fds[i].fd = new_socket;
                    fds[i].events = POLLIN;
                    printf("Adding to list of sockets as %d\n", i);
                    break;
                }
            }
        }

        // Check for IO operations on other sockets
        for (i = 1; i < MAX_CLIENTS + 1; i++) {
            int sd = fds[i].fd;

            if (sd > 0 && (fds[i].revents & POLLIN)) {
                int valread = read(sd, buffer, BUFFER_SIZE);
                if (valread == 0) {
                    // Client disconnected
                    socklen_t addrlen = sizeof(address);
                    getpeername(sd, (struct sockaddr *)&address, &addrlen);
                    printf("Client disconnected, ip %s, port %d\n", 
                           inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                    close(sd);
                    fds[i].fd = -1;
                } else {
                    // Process the incoming message
                    buffer[valread] = '\0';
                    printf("Message from client %d: %s\n", i, buffer);
                    broadcast_message(sd, fds, MAX_CLIENTS + 1, buffer);
                }
            }
        }
    }

    return 0;
}

void broadcast_message(int sender_fd, struct pollfd *fds, int num_clients, char *message) {
    for (int i = 1; i < num_clients; i++) {
        int client_fd = fds[i].fd;
        if (client_fd != sender_fd && client_fd > 0) {
            send(client_fd, message, strlen(message), 0);
        }
    }
}