#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    fd_set readfds;

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set up the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Change this if needed

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server.\n");

    while (1) {
        // Initialize the file descriptor set
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds); // Monitor standard input
        FD_SET(sockfd, &readfds);       // Monitor the server socket

        // Determine the maximum file descriptor value
        int max_fd = sockfd > STDIN_FILENO ? sockfd : STDIN_FILENO;

        // Use select to wait for input or timeout
        int result = select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if (result == -1) {
            perror("select");
            break;
        }

        // Check for data on standard input
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            fgets(buffer, sizeof(buffer), stdin);
            // Send the input to the server
            send(sockfd, buffer, strlen(buffer), 0);
        }

        // Check for data from the server socket
        if (FD_ISSET(sockfd, &readfds)) {
            int bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received <= 0) {
                // Server disconnected or error
                perror("recv");
                break;
            }
            buffer[bytes_received] = '\0'; // Null-terminate the received data
            printf("Received from other clients: %s", buffer);
        }
    }

    // Clean up
    close(sockfd);
    return 0;
}