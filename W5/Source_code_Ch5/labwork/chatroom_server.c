#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/poll.h>
#include <errno.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

void broadcast_message(int sender_fd, int *client_sockets, int num_clients, char *message);
int get_activity_select(int server_fd, int *client_sockets, fd_set *readfds);
int get_activity_poll(int server_fd, int *client_sockets, struct pollfd *fds);
int get_activity_pselect(int server_fd, int *client_sockets, fd_set *readfds, sigset_t *orig_mask);

int main() {
    int server_fd, new_socket, client_sockets[MAX_CLIENTS], max_sd, activity, i;
    struct sockaddr_in address;
    int opt = 1;
    fd_set readfds;
    struct pollfd fds[MAX_CLIENTS + 1];
    sigset_t orig_mask;
    char buffer[BUFFER_SIZE];


    // Initialize all client sockets to 0
    for (i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = 0;
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

    while (1) {
        // Clear the socket set
        FD_ZERO(&readfds);
        // Add server socket to set
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        int max_sd = server_fd;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];
            if (sd > 0) {
                FD_SET(sd, &readfds);
            }
            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        // If something happened on the master socket, then it's an incoming connection
        if (FD_ISSET(server_fd, &readfds)) {
            socklen_t addrlen = sizeof(address);
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            printf("New connection, socket fd is %d, ip is : %s, port : %d\n", 
                   new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            // Add new socket to array of sockets
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = new_socket;
                    printf("Adding to list of sockets as %d\n", i);
                    break;
                }
            }
        }

        // Check for IO operations on other sockets
        for (i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];
            if (FD_ISSET(sd, &readfds)) {
                int valread;
                if ((valread = read(sd, buffer, BUFFER_SIZE)) == 0) {
                    socklen_t addrlen = sizeof(address);
                    getpeername(sd, (struct sockaddr *)&address, &addrlen);
                    printf("Client disconnected, ip %s, port %d\n", 
                           inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                    close(sd);
                    client_sockets[i] = 0;
                } else {
                    buffer[valread] = '\0';
                    printf("Message from client %d: %s\n", i, buffer);
                    broadcast_message(sd, client_sockets, MAX_CLIENTS, buffer);
                }
            }
        }
    }

    return 0;
}

void broadcast_message(int sender_fd, int *client_sockets, int num_clients, char *message) {
    for (int i = 0; i < num_clients; i++) {
        int client_fd = client_sockets[i];
        if (client_fd != sender_fd && client_fd > 0) {
            send(client_fd, message, strlen(message), 0);
        }
    }
}

/*
int get_activity_poll(int server_fd, int *client_sockets, struct pollfd *fds) {
    int num_fds = 1; 
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        int sd = client_sockets[i];
        if (sd > 0) {
            fds[num_fds].fd = sd;
            fds[num_fds].events = POLLIN;
            num_fds++;
        }
    }
    return poll(fds, num_fds, -1);
}
*/

int get_activity_pselect(int server_fd, int *client_sockets, fd_set *readfds, sigset_t *orig_mask) {
    int max_sd = server_fd;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        int sd = client_sockets[i];
        if (sd > 0) {
            FD_SET(sd, readfds);
        }
        if (sd > max_sd) {
            max_sd = sd;
        }
    }
    return pselect(max_sd + 1, readfds, NULL, NULL, NULL, orig_mask);
}