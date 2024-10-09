#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define USERNAME_SIZE 32

typedef struct {
    int socket;
    char username[USERNAME_SIZE];
} Client;

void broadcast_message(int sender_fd, Client *clients, int num_clients, char *message);

int main() {
    int server_fd, new_socket, max_sd, activity, i;
    struct sockaddr_in address;
    int opt = 1;
    fd_set readfds;
    char buffer[BUFFER_SIZE];
    Client clients[MAX_CLIENTS] = {0};

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

    while (1) {
        // Clear the socket set
        FD_ZERO(&readfds);

        // Add server socket to set
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        // Add client sockets to set
        for (i = 0; i < MAX_CLIENTS; i++) {
            int sd = clients[i].socket;
            if (sd > 0) {
                FD_SET(sd, &readfds);
            }
            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        // Wait for activity on one of the sockets
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) {
            perror("select error");
        }

        // If something happened on the master socket, then it's an incoming connection
        if (FD_ISSET(server_fd, &readfds)) {
            socklen_t addrlen = sizeof(address);
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            // Request username from the new client
            char username[USERNAME_SIZE];
            memset(username, 0, USERNAME_SIZE); // Clear the username buffer
            recv(new_socket, username, USERNAME_SIZE, 0);
            username[strcspn(username, "\n")] = '\0'; // Remove newline character

            // Add new socket to array of clients
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].socket == 0) {
                    clients[i].socket = new_socket;
                    strncpy(clients[i].username, username, USERNAME_SIZE);
                    printf("New connection, socket fd is %d, username is %s\n", new_socket, username);

                    // Notify all clients about the new user
                    char notification[BUFFER_SIZE];
                    snprintf(notification, BUFFER_SIZE, "%s has entered the chat", username);
                    broadcast_message(-1, clients, MAX_CLIENTS, notification);

                    break;
                }
            }
        }

        // Check for IO operations on other sockets
        for (i = 0; i < MAX_CLIENTS; i++) {
            int sd = clients[i].socket;
            if (FD_ISSET(sd, &readfds)) {
                int valread;
                if ((valread = read(sd, buffer, BUFFER_SIZE)) == 0) {
                    // Client disconnected
                    socklen_t addrlen = sizeof(address);
                    getpeername(sd, (struct sockaddr *)&address, &addrlen);
                    printf("Client disconnected, ip %s, port %d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                    // Notify all clients about the user leaving
                    char notification[BUFFER_SIZE];
                    snprintf(notification, BUFFER_SIZE, "%s has left the chat", clients[i].username);
                    broadcast_message(-1, clients, MAX_CLIENTS, notification);

                    close(sd);
                    clients[i].socket = 0;
                    memset(clients[i].username, 0, USERNAME_SIZE);
                } else {
                    // Process the incoming message
                    buffer[valread] = '\0';
                    char message[BUFFER_SIZE + USERNAME_SIZE + 4];
                    snprintf(message, sizeof(message), "%s: %s", clients[i].username, buffer);
                    printf("Message from client %d: %s\n", i, message);

                    // Broadcast the message to other clients
                    broadcast_message(sd, clients, MAX_CLIENTS, message);
                }
            }
        }
    }

    return 0;
}

void broadcast_message(int sender_fd, Client *clients, int num_clients, char *message) {
    for (int i = 0; i < num_clients; i++) {
        int client_fd = clients[i].socket;
        if (client_fd != sender_fd && client_fd > 0) {
            send(client_fd, message, strlen(message), 0);
        }
    }
}
