#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <_ctype.h>

#define DEFAULT_PORT 5019

int main(int argc, char **argv) {
    char szBuff[100];
    int msg_len;
    struct sockaddr_in server_addr;
    struct hostent *hp;
    int connect_sock;
    char *server_name = "localhost";
    unsigned short port = DEFAULT_PORT;
    unsigned int addr;

    if (argc != 3) {
        printf("echoscln [server name] [port number]\n");
        return -1;
    } else {
        server_name = argv[1];
        port = atoi(argv[2]);
    }

    if (isalpha(server_name[0]))
        hp = gethostbyname(server_name);
    else {
        addr = inet_addr(server_name);
        hp = gethostbyaddr((char *)&addr, 4, AF_INET);
    }

    if (hp == NULL) {
        fprintf(stderr, "Cannot resolve address: %s\n", hstrerror(h_errno));
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    memcpy(&(server_addr.sin_addr), hp->h_addr, hp->h_length);
    server_addr.sin_family = hp->h_addrtype;
    server_addr.sin_port = htons(port);

    connect_sock = socket(AF_INET, SOCK_STREAM, 0);

    if (connect_sock < 0) {
        perror("socket() failed");
        return -1;
    }

    printf("Client connecting to: %s\n", hp->h_name);

    if (connect(connect_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect() failed");
        close(connect_sock);
        return -1;
    }

    while (1) {
        printf("input character string:\n");
        fgets(szBuff, sizeof(szBuff), stdin);

        msg_len = send(connect_sock, szBuff, strlen(szBuff), 0);

        if (msg_len < 0) {
            perror("send() failed");
            close(connect_sock);
            return -1;
        }

        if (msg_len == 0) {
            printf("server closed connection\n");
            close(connect_sock);
            return -1;
        }

        msg_len = recv(connect_sock, szBuff, sizeof(szBuff), 0);

        if (msg_len < 0) {
            perror("recv() failed");
            close(connect_sock);
            return -1;
        }

        if (msg_len == 0) {
            printf("server closed connection\n");
            close(connect_sock);
            return -1;
        }

        szBuff[msg_len] = '\0'; // Null-terminate whatever we received and print it.
        printf("Echo from the server: %s\n", szBuff);
    }

    close(connect_sock);
    return 0;
}
