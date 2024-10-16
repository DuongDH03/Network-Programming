#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define MAXLINE 1024
#define TIMEOUT 100

typedef struct {
    struct sockaddr_in addr;
    char key;
} Client;

#define MAX_CLIENTS 10
Client clients[MAX_CLIENTS];
int client_count = 0;

void xor_cipher(char *data, char key) {
    for (int i = 0; data[i] != '\0'; i++) {
        data[i] ^= key;
    }
}

int find_client(struct sockaddr_in *cliaddr) {
    for (int i = 0; i < client_count; i++) {
        if (memcmp(&clients[i].addr, cliaddr, sizeof(struct sockaddr_in)) == 0) {
            return i;
        }
    }
    return -1;
}

void add_client(struct sockaddr_in *cliaddr, char key) {
    if (client_count < MAX_CLIENTS) {
        clients[client_count].addr = *cliaddr;
        clients[client_count].key = key;
        client_count++;
    } else {
        printf("Max clients reached. Cannot add more clients.\n");
    }
}

int main() {
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;
    int n;
    socklen_t len;
    fd_set readfds;
    struct timeval tv;

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    while (1) {
        len = sizeof(cliaddr);
        printf("Server is running...\n");

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
                n = recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr *)&cliaddr, &len);
                buffer[n] = '\0';

                int client_index = find_client(&cliaddr);
                if (client_index == -1) {
                    // New client, add to list with the received key
                    char key = buffer[0];
                    add_client(&cliaddr, key);
                    client_index = client_count - 1;
                    printf("Received key from new client: %c\n", key);
                } else {
                    printf("Client message: %s\n", buffer);

                    // Decrypt the message using the sender's key
                    xor_cipher(buffer, clients[client_index].key);
                    printf("Decrypted message: %s\n", buffer);

                    // Encrypt and send to all other clients
                    for (int i = 0; i < client_count; i++) {
                        if (i != client_index) {
                            char encrypted_msg[MAXLINE];
                            strcpy(encrypted_msg, buffer);
                            xor_cipher(encrypted_msg, clients[i].key);
                            sendto(sockfd, encrypted_msg, strlen(encrypted_msg), 0, (const struct sockaddr *)&clients[i].addr, len);
                        }
                    }
                }
            }
        }
    }

    close(sockfd);
    return 0;
}