#ifndef IMAN_H
#define IMAN_H

#include <netdb.h>

#define MAX_REQUEST_LENGTH 1000

// Remove HTML tags from content
void remove_tags(char* content);

// Setup a socket connection and return the socket descriptor
// Returns -1 in case of error, else returns 0
int establish_socket_connection(struct addrinfo* server_info);

// Resolve the hostname and get server information
// Returns the resolved server information
struct addrinfo* get_server_info(char* hostname, char* port);

// Send an HTTP GET request to retrieve the man page
// Returns -1 in case of error, else returns 0
int send_http_request(int sockfd, char* command);

// Receive the response from the server and process it
int receive_response(int sockfd);

// Retrieves and displays a man page for the given command
void iMan(char* command);

#endif // IMAN_H
