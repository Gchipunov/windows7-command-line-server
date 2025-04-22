#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib") // Link Winsock library for Visual Studio
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

#define SERVER_PORT 8080
#define MAXLINE 1024
#define MAX_CLIENTS 10 // Maximum number of unique clients to track

int main() {
    SOCKET sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[MAXLINE];
    const char *hello = "Hello from server";
    int n, result;

#ifdef _WIN32
    // Initialize Winsock for Windows
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }
#endif

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == INVALID_SOCKET) {
#ifdef _WIN32
        printf("Socket creation failed: %d\n", WSAGetLastError());
        WSACleanup();
#else
        perror("Socket creation failed");
#endif
        return 1;
    }

    // Fill server information
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(SERVER_PORT);

    // Bind the socket
    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
#ifdef _WIN32
        printf("Bind failed: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
#else
        perror("Bind failed");
        closesocket(sockfd);
#endif
        return 1;
    }

    printf("UDP Game Server started on port %d. Waiting for clients...\n", SERVER_PORT);

    while (1) {
        // Receive message from client
      //  socklen_t len = sizeof(cliaddr);
		int len = sizeof(cliaddr);

        n = recvfrom(sockfd, buffer, MAXLINE - 1, 0,
                     (struct sockaddr*)&cliaddr, &len);
        if (n == SOCKET_ERROR) {
#ifdef _WIN32
            printf("Receive failed: %d\n", WSAGetLastError());
#else
            perror("Receive failed");
#endif
            continue;
        }

        buffer[n] = '\0'; // Null-terminate the received data
        printf("Client message: %s\n", buffer);

        // Send response to client
        result = sendto(sockfd, hello, strlen(hello), 0,
                        (struct sockaddr*)&cliaddr, len);
        if (result == SOCKET_ERROR) {
#ifdef _WIN32
            printf("Send failed: %d\n", WSAGetLastError());
#else
            perror("Send failed");
#endif
            continue;
        }

        printf("Response sent: %s\n", hello);
    }

    // Cleanup
    closesocket(sockfd);
#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}