#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void print_board(int game_board[3][3]) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (game_board[i][j] == 1)
                printf("X ");
            else if (game_board[i][j] == 2)
                printf("O ");
                else printf("- ");
        }
        printf("\n");
    }
    printf("\n");
}

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int game_board[3][3];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect failed");
        exit(EXIT_FAILURE);
    }

    fd_set readfds;
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        int max_fd = sock > STDIN_FILENO ? sock : STDIN_FILENO;

        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("Select error");
            break;
        }

        if (FD_ISSET(sock, &readfds)) {
            if (recv(sock, buffer, BUFFER_SIZE, 0) <= 0) {
                perror("Receive failed");
                break;
            }
            if (buffer[0] == 0x07) {
                printf("Other player or server disconnected. Game will close now.\n");
                break;
            } else if (buffer[0] == 0x03) {
                memcpy(game_board, buffer + 1, sizeof(game_board));
                print_board(game_board);
            } else if (buffer[0] == 0x04) {
                int result = buffer[1];
                if (result == 0x03) {
                    printf("The game is a draw.\n");
                } else {
                    printf("Player %d wins!\n", result);
                }
                break;
            } else if (buffer[0] == 0x05) {
                int row, col;
                while (1) {
                    printf("Your turn. Enter your move (row and column): ");
                    scanf("%d %d", &row, &col);
                    buffer[0] = 0x02;
                    buffer[1] = row;
                    buffer[2] = col;
                    if (send(sock, buffer, 3, 0) <= 0) {
                        perror("Send failed");
                        break;
                    }
                    if (recv(sock, buffer, BUFFER_SIZE, 0) <= 0) {
                        perror("Receive failed");
                        break;
                    }
                    if (buffer[0] == 0x07) {
                        printf("Other player or server disconnected. Game will close now.\n");
                        break;
                    }
                    if (buffer[0] == 0x06) {
                        printf("INVALID MOVE! TRY AGAIN!\n");
                    } else {
                        break;
                    }

                }
            }
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            //handle input, currently idk :)
        }
    }

    close(sock);
    return 0;
}