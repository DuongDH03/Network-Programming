#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#define PORT 8888

int main() {

    struct sockaddr_in socket_address;

    memset(&socket_address, 0, sizeof(socket_address));

    socket_address.sin_family = AF_INET;

    socket_address.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "192.168.2.1", &socket_address.sin_addr) <= 0) {
        printf("Invalid address/ Not supported\n");
        return -1;
    }

    printf("sockaddr_in structure initialized:\n");
    printf("  Family      : AF_INET (IPv4)\n");
    printf("  Port        : %d\n", ntohs(socket_address.sin_port)); 
    printf("  IP Address  : 192.168.2.1\n");
    
    return 0;
}
