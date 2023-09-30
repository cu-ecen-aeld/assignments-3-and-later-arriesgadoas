#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>

#define PORT "9000"
#define BACKLOG 5
#define MAX_BUFFER_SIZE 32768
#define FILENAME "/var/tmp/aesdsocketdata"

int server_sock, file_fd;

// Function to gracefully exit the server
void handle_exit(int signal) {
    // Close the file and server socket
    close(file_fd);
    close(server_sock);
    remove(FILENAME);
    syslog(LOG_INFO, "Caught signal, exiting");
    exit(0);
}

// Function to get the IP address, IPv4 or IPv6
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main() {
    struct addrinfo hints, *server_info, *p;
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char client_ip[INET6_ADDRSTRLEN];
    char buffer[MAX_BUFFER_SIZE];
    char read_buff[MAX_BUFFER_SIZE];
    //char file_buffer[MAX_BUFFER_SIZE];
    int yes = 1;

    // Initialize the hints struct and set up signal handling
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    signal(SIGINT, handle_exit);  // Register signal handler for SIGINT (Ctrl+C)
    signal(SIGTERM, handle_exit); // Register signal handler for SIGTERM

    // Get address information for the server (use NULL for the hostname)
    if (getaddrinfo(NULL, PORT, &hints, &server_info) != 0) {
        fprintf(stderr, "getaddrinfo error\n");
        return -1;
    }

    // Loop through all the results and bind to the first available
    for (p = server_info; p != NULL; p = p->ai_next) {
        // Create socket
        if ((server_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("Socket creation failed");
            continue;
        }
        
        if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &yes,
            sizeof(int)) == -1) {
            perror("setsockopt");
            return -1;
        }
        
        // Bind socket to address and port
        if (bind(server_sock, p->ai_addr, p->ai_addrlen) == -1) {
            close(server_sock);
            perror("Binding failed");
            continue;
        }

        break;  // Success, break out of the loop
    }

    freeaddrinfo(server_info);  // No longer needed

    // Check if we successfully bound
    if (p == NULL) {
        fprintf(stderr, "Failed to bind\n");
        return -1;
    }

    // Listen for incoming connections
    if (listen(server_sock, BACKLOG) == -1) {
        perror("Listening failed");
        return -1;
    }

    printf("Server is listening on port %s...\n", PORT);

    // // Open the file for reading the data to be sent to clients
    // file_fd = open(FILENAME, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    // if (file_fd == -1) {
    //     perror("Error opening the file");
    //     return -1;
    // } 

    // Main server loop
    while (1) {
        int client_sock;
        // Accept incoming connection

        if ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len)) == -1) {
            perror("Accepting connection failed");
            continue;
        }

        // Open the file for reading the data to be sent to clients
        file_fd = open(FILENAME, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
        if (file_fd == -1) {
            perror("Error opening the file");
            return -1;
        }  

        // Get the client's IP address
        inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *)&client_addr), client_ip, sizeof(client_ip));

        // Connection established
        printf("Connection accepted from %s\n", client_ip);
        syslog(LOG_INFO, "Accepted connection from %s\n", client_ip);

        // Receive data from the client
        ssize_t bytes_received = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received == -1) {
            perror("Error while receiving data");
        } else if (bytes_received == 0) {
            printf("Client disconnected\n");
        } else {
            buffer[bytes_received] = '\0';  // Null-terminate the received data
            printf("Received bumber of bytes: %ld \n", bytes_received);
            printf("Received data from client: %s", buffer);
	    
            // You can process the received data here if needed
	        write(file_fd, buffer, bytes_received);
            
            close(file_fd);

            // Open the file for reading
            file_fd = open(FILENAME, O_RDONLY);
            if (file_fd == -1) {
                perror("Error opening the file");
                return -1;
            } 
            // Send a response back to the client
            ssize_t bytes_read = read(file_fd, read_buff, sizeof(read_buff));
            close(file_fd);
            read_buff[bytes_read] = '\0';
            printf("%s", read_buff);
            
            send(client_sock, read_buff, strlen(read_buff), 0);
        }

        // Close the client socket
        close(client_sock);
        printf("Closed connection from %s\n", client_ip);
        syslog(LOG_INFO, "Closed connection from %s\n", client_ip);
    }

    // This part will not be reached in the loop
    return 0;
}

