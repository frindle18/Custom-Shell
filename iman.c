#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "iman.h"
#include "prompt.h" // int handle_error(char* error_message)

// Remove HTML tags from content
void remove_tags(char* content) {
    char *src = content; // Pointer to the source content
    char *dst = content; // Pointer to the destination content (modified in place)

    // Loop through the content
    while (*src) {
        if (*src == '<') { // If an opening HTML tag is found
            while (*src && *src != '>') { // Skip until the closing tag is found
                src++;
            }
            if (*src) {
                src++; // Move past the closing '>'
            }
        }
        else {
            *dst++ = *src++; // Copy the content if not within a tag
        }
    }
    *dst = '\0'; // Null-terminate the cleaned string
}

// Setup a socket connection and return the socket descriptor
// Returns -1 in case of error, else returns 0
int establish_socket_connection(struct addrinfo* server_info) {
    int socket_fd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol); // Create a socket

    if (socket_fd == -1) { // Socket creation failed
        return handle_error("socket() error");
    }

    // Attempt to connect to the server
    if (connect(socket_fd, server_info->ai_addr, server_info->ai_addrlen) == -1) {
        close(socket_fd); // Close socket if connection fails
        return handle_error("connect() error");
    }

    return socket_fd; // Return the connected socket descriptor
}

// Resolve the hostname and get server information
// Returns the resolved server information
struct addrinfo* get_server_info(char* hostname, char* port) {
    struct addrinfo connection_hints, *server_info;

    memset(&connection_hints, 0, sizeof(connection_hints)); // Clear the hints structure
    connection_hints.ai_family = AF_UNSPEC; // Allow IPv4 or IPv6 addresses
    connection_hints.ai_socktype = SOCK_STREAM; // Use TCP stream sockets

    // Get address information for the server
    if (getaddrinfo(hostname, port, &connection_hints, &server_info) != 0) {
        return NULL; // Return NULL if failed to get server information
    }

    return server_info;
}

// Send an HTTP GET request to retrieve the man page
// Returns -1 in case of error, else returns 0
int send_http_request(int sockfd, char* command) {
    char request[MAX_REQUEST_LENGTH];

    // Create the HTTP GET request for the command's man page
    snprintf(request, sizeof(request),
             "GET /man1/%s HTTP/1.0\r\n"
             "Host: man.he.net\r\n"
             "Connection: close\r\n\r\n",
             command);

    // Send the request over the socket
    if (send(sockfd, request, strlen(request), 0) == -1) {
        close(sockfd); // Close the socket if sending fails
        return handle_error("send() error");
    }

    return 0;
}

// Receive the response from the server and process it
int receive_response(int sockfd) {
    char buffer[MAX_REQUEST_LENGTH];
    int bytes_received;

    // Continuously receive data from the server
    while ((bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0'; // Null-terminate the received data

        remove_tags(buffer); // Remove HTML tags from the content

        // Skip the first 6 lines (HTTP headers)
        int newlines = 0;
        for (int i = 0; i < bytes_received; i++) {
            if (buffer[i] == '\n') { // Count newlines
                newlines++;
            }
            if (newlines == 6) { // Once headers are skipped, shift the buffer to process content
                memmove(buffer, buffer + i + 1, bytes_received - i); // Move past headers
                break;
            }
        }

        printf("%s", buffer);  // Print the processed content (man page)
    }

    // Check for errors in receiving data
    if (bytes_received == -1) {
        return handle_error("recv() error");
    }
}

// Retrieves and displays a man page for the given command
void iMan(char* command) {
    // Get server information for the remote man page server
    struct addrinfo* res = get_server_info("man.he.net", "80");
    int sockfd = establish_socket_connection(res); // Establish a connection to the server
    freeaddrinfo(res); // Free the server info structure after use

    // Send the HTTP request for the command's man page
    send_http_request(sockfd, command);

    // Receive and display the response (the man page)
    receive_response(sockfd);

    close(sockfd);  // Close the socket after communication is complete
}
