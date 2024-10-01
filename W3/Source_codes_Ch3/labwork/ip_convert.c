#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

void convert_ipv4(const char *ip_str) {
    struct in_addr ipv4_addr;    // Structure to hold the IPv4 address

    // Convert the IPv4 address from string to binary form
    if (inet_pton(AF_INET, ip_str, &ipv4_addr) == 1) {
        printf("inet_pton (IPv4): Successfully converted IP address: %s\n", ip_str);
    } else {
        printf("inet_pton (IPv4): Failed to convert IP address: %s\n", ip_str);
        exit(EXIT_FAILURE);
    }

    char ip_str_converted[INET_ADDRSTRLEN];  // Buffer to hold the converted IP address back to string

    // Convert the binary IPv4 address back to string form
    if (inet_ntop(AF_INET, &ipv4_addr, ip_str_converted, INET_ADDRSTRLEN)) {
        printf("inet_ntop (IPv4): Converted back to string IP address: %s\n", ip_str_converted);
    } else {
        printf("inet_ntop (IPv4): Failed to convert IP address back to string\n");
        exit(EXIT_FAILURE);
    }
}

int main() {

    char ipv4_str[100];
    printf("Input the IP address: ");
    scanf("%s",ipv4_str); 
    //use fgets here will make the program read the "\n", hence you have to use the scanf

    printf("IPv4 Conversion:\n");
    convert_ipv4(ipv4_str);

    printf("\n");

    return 0;
}
