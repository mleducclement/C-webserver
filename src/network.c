//
// Created by pres-mleducclement on 2024-01-29.
//

#include "network.h"

void startWebserver() {
    const char resp[] = "HTTP/1.0 200 OK\r\n"
                  "Server: webserver-c\r\n"
                  "Content-type: text/html\r\n\r\n"
                  "<html>Hello! You've reached your very own webserver!</html>\r\n";

    // Create the socket
    const int socketFeed = socket(AF_INET, SOCK_STREAM, 0);

    if (socketFeed == -1) {
        perror("webserver (socket)");
    }
    // Set the SO_REUSEADDR option
    const int opt = 1;

    if (setsockopt(socketFeed, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0) {
        perror("webserver (setsockopt)");
    }
    printf("socket created successfully\n");
    // Create the address to bind the socket to
    struct sockaddr_in host_addr;
    int host_addrlen = sizeof(host_addr);
    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(PORT);
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    struct sockaddr_in client_addr;
    int client_addrlen = sizeof(client_addr);

    // Bind the socket
    if (bind(socketFeed, (struct sockaddr *)&host_addr, host_addrlen) != 0) {
        perror("webserver (bind)");
    }
    printf("socket successfully bound to address\n");

    // Listen for incoming connections
    if (listen(socketFeed, SOMAXCONN) != 0) {
        perror("webserver (listen)");
    }
    printf("server listening for connections\n");

    for (;;) {
        char buffer[BUFFER_SIZE];
        // Accept incoming connections
        const int newSocketFeed = accept(socketFeed, (struct sockaddr *)&host_addr, (socklen_t *)&host_addrlen);

        if (newSocketFeed < 0) {
            perror("webserver (accept)");
            continue;
        }
        printf("connection accepted\n");

        // Get client address
        const int socketName = getsockname(newSocketFeed, (struct sockaddr *)&client_addr, (socklen_t *)&client_addrlen);

        if (socketName < 0) {
            perror("webserver (getsockname");
            continue;
        }

        // Read from the socket
        const int valread = read(newSocketFeed, buffer, BUFFER_SIZE);
        if (valread < 0) {
            perror("webserver (read)");
            continue;
        }

        // Read headers from the socket
        char headers[BUFFER_SIZE];
        const char *headerEnd = strstr(buffer, "\r\n\r\n");
        if (headerEnd == NULL) {
            perror("webserver (readHeader)");
            continue;
        }
        // Copy including the blank line -> (2 * "\r\n")
        memcpy(headers, buffer, headerEnd - buffer + 4);
        // Read the request
        char method[BUFFER_SIZE], uri[BUFFER_SIZE], version[BUFFER_SIZE];
        sscanf(buffer, "%s %s %s", method, uri, version);
        printf("[%s:%u] %s %s %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), method, version, uri);

        // Handle POST requests
        if (strcmp(method, "POST") == 0) {
            // Get the content length header
            char contentLenghtHeader[BUFFER_SIZE];
            const char *contentLengthLine = strstr(headers, "Content-Length: ");

            if (contentLengthLine == NULL) {
                perror("webserver (strstr)");
                continue;
            }
            sscanf(contentLengthLine, "Content-Length: %s", contentLenghtHeader);

            // Parse the Content-Length header
            const int contentLength = atoi(contentLenghtHeader);

            if (contentLength <= 0 || contentLength > BUFFER_SIZE) {
                perror("webserver (atoi)");
                continue;
            }

            // Read the JSON data from the client
            char jsonData[BUFFER_SIZE] = {0};
            const int remainingData = contentLength - (valread - (headerEnd - buffer + 4));

            if (remainingData > 0) {
                memcpy(jsonData, headerEnd + 4, contentLength - remainingData);
                const int jsonDataLen = read(newSocketFeed, jsonData + contentLength - remainingData, remainingData);

                if (jsonDataLen < 0) {
                    perror("webserver (read)");
                    continue;
                }
            }
            else {
                memcpy(jsonData, headerEnd + 4, contentLength);
            }

            // Parse the JSON data
            hashtable* table = parseJSON(jsonData);
            print_table(table, "JSON");
            free_table(table);
        }

        const int valwrite = write(newSocketFeed, resp, strlen(resp));
        if (valwrite < 0) {
            perror("webserver (write)");
            continue;
        }
        close(newSocketFeed);
    }
}