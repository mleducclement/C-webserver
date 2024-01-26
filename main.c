
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "src/my_hashtable.h"
#include "src/constants.h"

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_KVP 1024

void startWebserver();
hashtable* parseJSON();
int getValueType(const char* value);

int main() {
    startWebserver();

    return 0;
}

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
            print_table(table);
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

hashtable* parseJSON(const char *jsonString) {
    printf("%s\n", jsonString);
    void *kvpArray[MAX_KVP];
    int index = 1;
    int start = 1;
    int count = 0;

    // Checks if JSON string starts with {
    if (jsonString[0] != '{' || jsonString[strlen(jsonString) - 1] != '}') {
        printf("Malformed JSON input\n");
        return NULL;
    }

    // Iterate over the entire JSON string
    while (jsonString[index] != '}' && count < MAX_KVP) {
        if (jsonString[index] == '{') {
            const int innerJsonStartIndex = index;
            while (jsonString[index] != '}') {
                index++;
            }
            const int innerJsonEndIndex = index;
            // Copy the string from innerJsonStartIndex to innerJsonEndIndex into kvp
            char *innerJson = strndup(jsonString + innerJsonStartIndex, innerJsonEndIndex - innerJsonStartIndex + 1);
            kvpArray[count] = innerJson;
        }
        const char character = jsonString[index];
        // If character is comma, then copy the string from start to index into kvp
        if (character == ',') {
            // Copy the string from start to index into kvp
            char *kvp = strndup(jsonString + start, index - start);
            kvpArray[count] = kvp;
            start = index + 1;
            count++;
        }
        index++;
    }

    // Copy the string from start to index into kvp
    char *kvp = strndup(jsonString + start, index - start);
    kvpArray[count] = kvp;

    hashtable *table = create_table();

    // Iterate over the kvpArray
    for (int i = 0; i <= count; i++) {
        // Finds the string index of the first occurrence of ':'
        // colon is not alloc new memory it points to the first occurence of the ':' in the string
        const char *colon = strchr(kvpArray[i], ':');
        if (colon != NULL) {
            // Copies the string from kvpArray[i] to colon - kvpArray[i] into key
            char *key = strndup(kvpArray[i], colon - (char*)kvpArray[i]);
            void *value;
            const ValueType type = getValueType(colon + 1);   // Determine type directly from JSON

            if (type == STRING) {
                // Extract and clean string value
                value = strndup(colon + 2, strlen(kvpArray[i]) - strlen(key) - 3);
            }
            else if (type == HASHTABLE) {
                // Parse nested JSON to create hashtable
                value = parseJSON(colon + 1);
            } else {
                // Handle other types (INT, FLOAT, BOOL)
                value = strndup(colon + 1, strlen(kvpArray[i]) - strlen(key) - 1);
            }

            if (key != NULL) {
                hashtable_insert(table, key, value, type);
            }
            free(key);    // Free the key string after it's used
        }
        free(kvpArray[i]);
    }
    return table;
}