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
#define MAX_CLIENTS 10

typedef struct {
    int client_socket;
    char username[1024];
} Client;

Client clients[MAX_CLIENTS];    //Array of structures is defined for the
int num_clients = 0;

void *handle_client(void *arg) {
    int client_socket = *((int *)arg);
    char username[1024];

    // Receive the username from the client
    recv(client_socket, username, sizeof(username), 0);

    // Add the client to the list
    if (num_clients < MAX_CLIENTS) {
        clients[num_clients].client_socket = client_socket;
        strcpy(clients[num_clients].username, username);
        num_clients++;
    } else {
        printf("Too many clients. Connection closed for %s\n", username);
        close(client_socket);
        return NULL;
    }

    // Send a welcome message to the client
    char welcome_message[1024];
    sprintf(welcome_message, "Welcome to the chat, %s!", username);
    send(client_socket, welcome_message, sizeof(welcome_message), 0);

    while (1) {
        char message[1024];
        int bytes_received = recv(client_socket, message, sizeof(message), 0);
        if (bytes_received <= 0) {
            // Client disconnected
            printf("Client %s disconnected.\n", username);
            close(client_socket);
            break;
        }

        // Broadcast the message to all other clients
        for (int i = 0; i < num_clients; i++) {
            if (clients[i].client_socket != client_socket) {
                char broadcast_message[1024];
                sprintf(broadcast_message, "%s: %s", username, message);
                send(clients[i].client_socket, broadcast_message, sizeof(broadcast_message), 0);
            }
        }
    }
    //after ending the infinite while loop ---> disconnecting from server.
    // Remove the client from the list
    for (int i = 0; i < num_clients; i++) {
        if (clients[i].client_socket == client_socket) {
            for (int j = i; j < num_clients - 1; j++) {
                clients[j] = clients[j + 1];
            }
            num_clients--;
            break;
        }
    }

    return NULL;
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error in socket");
        exit(1);
    }
    printf("Server socket created.\n");

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error in binding");
        exit(1);
    }
    printf("Binding to port %d\n", PORT);

    if (listen(server_socket, 10) == 0) {
        printf("Listening...\n");
    } else {
        perror("Error in listening");
        exit(1);
    }

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
        if (client_socket < 0) {
            perror("Error in accepting");
            exit(1);
        }
        printf("Connection accepted from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        pthread_t x;
        pthread_create(&x, NULL, handle_client, &client_socket); //client_socket is the argument that we gonna pass into handle_client fn...
    }

    close(server_socket);
    return 0;
}
