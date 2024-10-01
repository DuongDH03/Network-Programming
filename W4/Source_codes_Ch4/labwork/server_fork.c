// concurrent_server.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT 8080
#define BUFFER_SIZE 2048

typedef struct {
    char question[256];
    char options[4][256];
    int correct_option;
} Question;


Question questions[10] = {
    {"Question 1: 1+1=?", {"A: 1", "B: 2", "C: 4", "D: 5"}, 2},
    {"Question 2: 2*2=?", {"A: 1", "B: 4", "C: 5", "D: 9"}, 2},
    {"Question 3: 3^2=?", {"A: 6", "B: 0", "C: 9", "D: 10"}, 3},
    {"Question 4: Beef is from which animals?", {"A: Cow", "B: Pig", "C: Chicken", "D: Goose"}, 1},
    {"Question 5: Who was in Paris?", {"A: Kanye", "B: Fellas", "C: Brothas", "D: Chocolate Bro"}, 2},
    {"Question 6: 2^10=?", {"A: 1001", "B: 200", "C: 1000", "D: 1024"}, 4},
    {"Question 7: What is the meaning of life?", {"A: I", "B: don't", "C: know", "D: man"}, 4},
    {"Question 8: [Oblidge] means?", {"A: Serve", "B: Withdraw", "C: Chase", "D: Fall"}, 1},
    {"Question 9: WWII ended in?", {"A: 1945", "B: 1930", "C: 2015", "D: 1999"}, 1},
    {"Question 10: Can we get a?", {"A: L", "B: O", "C: V", "D: E"}, 2}
};

// Signal handler to prevent zombie processes
void sigchld_handler(int sig) {
    (void)sig; // Ignore unused parameter warning
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handle_client(int connfd) {
    char buffer[BUFFER_SIZE];
    int score = 0;

    for (int i = 0; i < 10; i++) {
        memset(buffer, 0, BUFFER_SIZE);
        snprintf(buffer, sizeof(buffer), "%s\n1. %s\n2. %s\n3. %s\n4. %s\n",
                 questions[i].question,
                 questions[i].options[0],
                 questions[i].options[1],
                 questions[i].options[2],
                 questions[i].options[3]);
        send(connfd, buffer, strlen(buffer), 0);

        int answer;
        recv(connfd, &answer, sizeof(answer), 0);

        if (answer == questions[i].correct_option) {
            score++;
        }
    }

    memset(buffer, 0,   BUFFER_SIZE);
    snprintf(buffer, sizeof(buffer), "Your score: %d/10\n", score);
    send(connfd, buffer, strlen(buffer), 0);

    close(connfd);
    exit(0);
}

int main() {
    int listenfd, connfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    pid_t pid;

    // Create the listening socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Setup the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the listening socket to the specified port
    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(listenfd, 5) < 0) {
        perror("Listen failed");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    // Handle SIGCHLD to prevent zombie processes
    signal(SIGCHLD, sigchld_handler);

    printf("Server is listening on port %d...\n", PORT);

    // Server loop to accept multiple clients
    while (1) {
        // Accept an incoming connection
        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &addr_len);
        if (connfd < 0) {
            perror("Accept failed");
            continue;
        }

        // Fork a child process to handle the client
        pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            close(connfd);
        } else if (pid == 0) {
            // Child process: handle the client
            close(listenfd);  // Close the listening socket in the child process
            handle_client(connfd);
        } else {
            // Parent process: continue accepting new clients
            close(connfd);  // Close the client socket in the parent process
        }
    }

    return 0;
}
