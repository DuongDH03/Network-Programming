#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define USERNAME_SIZE 32

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char username[USERNAME_SIZE];
    int n;

    // Create a socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Setup the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Convert IPv4 address from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Prompt for username
    printf("Enter your username: ");
    fgets(username, USERNAME_SIZE, stdin);
    username[strcspn(username, "\n")] = '\0'; // Remove \n

    // Send the username to the server
    send(client_socket, username, strlen(username), 0);

    printf("Connected to server as %s. Type a message and press enter:\n", username);

    fd_set readfds;
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(client_socket, &readfds);

        int max_sd = client_socket;
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0 && errno != EINTR) {
            perror("select error");
        }

        // Check if there is input from the user
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            fgets(buffer, BUFFER_SIZE, stdin);
            buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline character

            // Check if the string is empty. If it is, stop the program
            if (buffer[0] == '\0') {
                break;
            }

            // Send the message to the server
            send(client_socket, buffer, strlen(buffer), 0);
        }

        // Check if there is a message from the server
        if (FD_ISSET(client_socket, &readfds)) {
            n = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if (n > 0) {
                buffer[n] = '\0'; // Null-terminate the received string
                printf("%s\n", buffer);
            } else if (n == 0) {
                printf("Server disconnected.\n");
                break;
            } else {
                perror("recv failed");
                break;
            }
        }
    }

    // Close the socket
    close(client_socket);
    return 0;
}