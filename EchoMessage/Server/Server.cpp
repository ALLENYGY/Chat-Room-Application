#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define DEFAULT_PORT    5019

int main(int argc, char **argv) {
    char szBuff[100];
    int msg_len;
    socklen_t addr_len;
    struct sockaddr_in local, client_addr;

    int sock, msg_sock;

    // Set up the address structure
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(DEFAULT_PORT);

    sock = socket(AF_INET, SOCK_STREAM, 0);  // TCP socket

    if (sock < 0) {
        perror("socket() failed");
        return -1;
    }

    if (bind(sock, (struct sockaddr *)&local, sizeof(local)) < 0) {
        perror("bind() failed");
        close(sock);
        return -1;
    }

    // Waiting for connections
    if (listen(sock, 5) < 0) {
        perror("listen() failed");
        close(sock);
        return -1;
    }

    printf("Waiting for connections ........\n");

    addr_len = sizeof(client_addr);
    msg_sock = accept(sock, (struct sockaddr *)&client_addr, &addr_len);
    if (msg_sock < 0) {
        perror("accept() failed");
        close(sock);
        return -1;
    }

    printf("Accepted connection from %s, port %d\n",
           inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port));

    while (1) {
        msg_len = recv(msg_sock, szBuff, sizeof(szBuff), 0);

        if (msg_len < 0) {
            perror("recv() failed");
            close(msg_sock);
            close(sock);
            return -1;
        }

        if (msg_len == 0) {
            printf("Client closed connection\n");
            close(msg_sock);
            continue;
        }

        szBuff[msg_len] = '\0'; // Ensure null-termination
        printf("Bytes Received: %d, message: %s from %s\n", msg_len, szBuff, inet_ntoa(client_addr.sin_addr));

        msg_len = send(msg_sock, szBuff, msg_len, 0);  // Echo back the received message
        if (msg_len < 0) {
            perror("send() failed");
            close(msg_sock);
            close(sock);
            return -1;
        }
    }

    close(msg_sock);
    close(sock);
    return 0;
}
