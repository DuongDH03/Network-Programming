#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define MAXLINE 1024
#define TIMEOUT 100

void xor_cipher(char *data, char key) {
    for (int i = 0; data[i] != '\0'; i++) {
        data[i] ^= key;
    }
}

int main() {
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr;
    int n;
    socklen_t len;
    fd_set readfds;
    struct timeval tv;
    char key;

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    // Connect to server
    if (connect(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Connect failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Generate a random key
    srand(time(NULL));
    key = (char)(rand() % 256);

    // Send the key to the server
    send(sockfd, &key, sizeof(key), 0);
    printf("Key sent to server: %c\n", key);

    while (1) {
        char message[MAXLINE];
        printf("Enter the message: ");
        fgets(message, MAXLINE, stdin);

        // Encrypt the message with the key
        xor_cipher(message, key);

        // Send the encrypted message to the server
        send(sockfd, message, strlen(message), 0);
        printf("Encrypted message sent to server.\n");

        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        tv.tv_sec = TIMEOUT;
        tv.tv_usec = 0;

        int activity = select(sockfd + 1, &readfds, NULL, NULL, &tv);

        if (activity < 0) {
            perror("select error");
        } else if (activity == 0) {
            printf("Timeout occurred! No data after %d seconds.\n", TIMEOUT);
        } else {
            if (FD_ISSET(sockfd, &readfds)) {
                n = recv(sockfd, buffer, MAXLINE, 0);
                buffer[n] = '\0';

                // Decrypt the message with the key
                xor_cipher(buffer, key);
                printf("Decrypted message: %s\n", buffer);
            }
        }
    }

    close(sockfd);
    return 0;
}