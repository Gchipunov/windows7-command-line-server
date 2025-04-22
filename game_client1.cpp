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

#define SERVER_IP "127.0.0.1" // Replace with your server's IP
#define SERVER_PORT 8080
#define MAXLINE 1024

int main() {
    SOCKET sockfd;
    struct sockaddr_in servaddr;
    char buffer[MAXLINE];
    const char *hello = "Hello from client";
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
    servaddr.sin_port = htons(SERVER_PORT);
#ifdef _WIN32
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
#else
    if (inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr) <= 0) {
        perror("Invalid address");
        closesocket(sockfd);
        return 1;
    }
#endif

    printf("UDP Game Client started. Type 'exit' to quit.\n");

    while (1) {
        // Get user input (for game commands, e.g., movement)
        printf("Enter message to send (or 'exit' to quit): ");
        fgets(buffer, MAXLINE, stdin);
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline

        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        // Send message to server
        result = sendto(sockfd, buffer, strlen(buffer), 0,
                        (struct sockaddr*)&servaddr, sizeof(servaddr));
        if (result == SOCKET_ERROR) {
#ifdef _WIN32
            printf("Send failed: %d\n", WSAGetLastError());
#else
            perror("Send failed");
#endif
            break;
        }

        printf("Message sent: %s\n", buffer);

        // Receive response from server
     //   socklen_t len = sizeof(servaddr);
		 int len = sizeof(servaddr);

        n = recvfrom(sockfd, buffer, MAXLINE - 1, 0,
                     (struct sockaddr*)&servaddr, &len);
        if (n == SOCKET_ERROR) {
#ifdef _WIN32
            printf("Receive failed: %d\n", WSAGetLastError());
#else
            perror("Receive failed");
#endif
            break;
        }

        buffer[n] = '\0'; // Null-terminate the received data
        printf("Server response: %s\n", buffer);
    }

    // Cleanup
    closesocket(sockfd);
#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}