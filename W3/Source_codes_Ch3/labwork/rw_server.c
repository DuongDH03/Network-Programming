#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAXLINE 1024
#define PORT 8080
#define MAX_ADDR_STRLEN 128

ssize_t readline(int fd, void *vptr, size_t maxlen);

// Custom sock_ntop function to convert a socket address into a string (IP and port)
char *sock_ntop(const struct sockaddr *sa, socklen_t salen) {
    static char str[MAX_ADDR_STRLEN];   // Buffer to hold the string representation
    char portstr[8];                    // Buffer to hold the port as a string

    // Check if the address is IPv4
    if (sa->sa_family == AF_INET) {
        struct sockaddr_in *sin = (struct sockaddr_in *) sa;
        if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL) {
            return NULL;   // Return NULL on failure
        }
        snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin->sin_port)); // Convert port to string
        strcat(str, portstr);   // Append the port to the IP string
        return str;
    }
    // Check if the address is IPv6
    else if (sa->sa_family == AF_INET6) {
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) sa;
        if (inet_ntop(AF_INET6, &sin6->sin6_addr, str, sizeof(str)) == NULL) {
            return NULL;   // Return NULL on failure
        }
        snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin6->sin6_port)); // Convert port to string
        strcat(str, portstr);   // Append the port to the IP string
        return str;
    } else {
        return NULL;  // Unsupported address family
    }
}


int main() {
    int listenfd, connfd;
    struct sockaddr_in servaddr,client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[MAXLINE];
    ssize_t n;

    // Create a listening socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // Bind the socket to the address and port
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(listenfd, 10) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Accept incoming connection
    connfd = accept(listenfd, (struct sockaddr *)NULL, NULL);
    if (connfd < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    if (getpeername(connfd, (struct sockaddr *)&client_addr, &client_len) == -1) {
        perror("getpeername failed");
        exit(EXIT_FAILURE);
    }

    printf("Client connected from %s\n", sock_ntop((struct sockaddr *)&client_addr, client_len));

    // Read the message from the client using readline
    while ((n = readline(connfd, buffer, MAXLINE)) > 0) {
        buffer[n] = '\0';
        printf("Received message: %s", buffer);

        // Send the received message back to the client
        if (write(connfd, buffer, n) < 0) {
            perror("Write failed");
            exit(EXIT_FAILURE);
        }
    }

    if (n < 0) {
        perror("Readline failed");
        exit(EXIT_FAILURE);
    }

    close(connfd);
    close(listenfd);
    return 0;
}

// Function to read a line (up to \n) from a descriptor
ssize_t readline(int fd, void *vptr, size_t maxlen) {
    ssize_t n, rc;
    char c, *ptr;

    ptr = vptr;
    for (n = 1; n < maxlen; n++) {
        if ((rc = read(fd, &c, 1)) == 1) {
            *ptr++ = c;
            if (c == '\n') {
                break;  // Stop at newline
            }
        } else if (rc == 0) {
            if (n == 1) {
                return 0; // No data read
            } else {
                break;  // Some data was read
            }
        } else {
            return -1; // Error in read
        }
    }
    *ptr = 0;
    return n;
}
