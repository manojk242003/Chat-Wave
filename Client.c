#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080

void *receive_messages(void *arg) {
    int client_socket = *((int *)arg);
    while (1) {
        char message[1024];
        int bytes_received = recv(client_socket, message, sizeof(message), 0);
        if (bytes_received <= 0) {
            break;
        }
        printf("%s\n", message);
    }
    pthread_exit(NULL);
}

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char username[1024];

    // Create a socket and connect to the server
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Error in socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error in connect");
        exit(1);
    }
    printf("Connected to the server.\n");

    // Get the username from the user
    printf("Enter your username: ");
    scanf("%s", username);

    // Send the username to the server
    send(client_socket, username, sizeof(username), 0);

    // Create a thread to receive messages from the server
    pthread_t receive_thread;
    pthread_create(&receive_thread, NULL, receive_messages, &client_socket);
    
    // Send messages to the server
    while (1) {
        char message[1024];

    
        scanf(" %99[^\n]", message);

        send(client_socket, message, sizeof(message), 0);

        if (strcmp(message, ":exit") == 0) {
            break;
        }
    }



    close(client_socket);
    return 0;
}
