#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int game_board[3][3];
int current_player = 1;
int move_count = 0;

void initialize_board() {
    memset(game_board, 0, sizeof(game_board));
}

void print_board() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            printf("%d ", game_board[i][j]);
        }
        printf("\n");
    }
}

int check_winner() {
    //3x rows and columns
    for (int i = 0; i < 3; i++) {
        if (game_board[i][0] == game_board[i][1] && game_board[i][1] == game_board[i][2] && game_board[i][0] != 0)
            return game_board[i][0];
        if (game_board[0][i] == game_board[1][i] && game_board[1][i] == game_board[2][i] && game_board[0][i] != 0)
            return game_board[0][i];
    }
    // 3x diagonals
    if (game_board[0][0] == game_board[1][1] && game_board[1][1] == game_board[2][2] && game_board[0][0] != 0)
        return game_board[0][0];
    if (game_board[0][2] == game_board[1][1] && game_board[1][1] == game_board[2][0] && game_board[0][2] != 0)
        return game_board[0][2];
    return 0;
}

void notify_turn(int client_sock) {
    char buffer[BUFFER_SIZE] = {0x05};
    send(client_sock, buffer, 1, 0);
}

void send_state_update(int client1_sock, int client2_sock) {
    char buffer[BUFFER_SIZE] = {0x03};
    memcpy(buffer + 1, game_board, sizeof(game_board));
    if (send(client1_sock, buffer, sizeof(buffer), 0) < 0) {
        perror("Send to client 1 failed!");
    }
    if (send(client2_sock, buffer, sizeof(buffer), 0) < 0) {
        perror("Send to client 2 failed!");
    }
}

void send_result(int client1_sock, int client2_sock, int result) {
    char buffer[BUFFER_SIZE] = {0x04, result};
    send(client1_sock, buffer, 2, 0);
    send(client2_sock, buffer, 2, 0);
}

void send_invalid_move(int client_sock) {
    char buffer[BUFFER_SIZE] = {0x06}; // Invalid move message
    send(client_sock, buffer, 1, 0);
}

int main() {
    int server_sock, client1_sock, client2_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    int opt = 1;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock <= 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    //set sock opt for reuse addr
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0 ) {
        perror("Bind failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    if(listen(server_sock, 2) < 0 ) {
        perror("Listen failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Waiting for players to connect...\n");
    client1_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);
    if (client1_sock < 0) {
        perror("Accept failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }
    printf("Player 1 connected\n");

    client2_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);
    if (client2_sock < 0) {
        perror("Accept failed");
        close(client1_sock);
        close(server_sock);
        exit(EXIT_FAILURE);
    }
    printf("Player 2 connected\n");

    printf("Both players connected. Starting the game...\n");

    initialize_board();

    while (1) {
        int row, col;
        notify_turn(current_player == 1 ? client1_sock : client2_sock);
        if (recv(current_player == 1 ? client1_sock : client2_sock, buffer, BUFFER_SIZE, 0) <= 0 ) {
            perror("Recieve failed!");
            printf("Client %d disconnected.\n", current_player);
            char notification[BUFFER_SIZE] = {0x07}; // Disconnection notification
            send(client1_sock, notification, 1, 0);
            send(client2_sock, notification, 1, 0);
            break;
        }

        if (buffer[0] == 0x02) {
            row = buffer[1];
            col = buffer[2];
            if (row >= 0 && row < 3 && col >= 0 && col < 3 && game_board[row][col] == 0) { //check valid
                game_board[row][col] = current_player;
                move_count++;
                send_state_update(client1_sock, client2_sock); //somehow the current player doesn't get notified

                int winner = check_winner();
                if (winner > 0 || move_count == 9) {
                    send_result(client1_sock, client2_sock, winner > 0 ? winner : 0x03);
                    break;
                }
                current_player = 3 - current_player;
            } else {
                send_invalid_move(current_player == 1 ? client1_sock : client2_sock);
            }
        }
    }

    close(client1_sock);
    close(client2_sock);
    close(server_sock);
    return 0;
}