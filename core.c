#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "core.h"


pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;
struct Client *clients = NULL;


void addClient(struct Client** clients, int new_socket){

    struct Client *new_client = (struct Client*)malloc(sizeof(struct Client));

    new_client->client_socket = new_socket;
    new_client->next = (*clients);
    (*clients) = new_client;
}

void deleteClient(struct Client ** clients, int client_socket){

    struct Client *temp = *clients, *prev;

    if (temp != NULL && temp->client_socket == client_socket) { 
        *clients = temp->next;
        close(temp->client_socket);
        free(temp);
        return; 
    }
    while (temp != NULL && temp->client_socket != client_socket) { 
        prev = temp; 
        temp = temp->next; 
    }
    if (temp == NULL) 
        return;

    prev->next = temp->next; 
    close(temp->client_socket);
    free(temp);
  
}

void printList(struct Client* node) 
{ 
    while (node != NULL) { 
        printf(" %d ", node->client_socket); 
        node = node->next; 
    } 
} 

void brodcastClients(struct Client** clients, const char *buffer) 
{ 
    struct Client *temp = *clients;
    while (temp != NULL) { 
        if (write(temp->client_socket, buffer, strlen(buffer)) == -1){
            perror("Broadcast failed");
        }
        temp = temp->next; 
    } 
} 

void *handle_client(void *arg) {
    char buffer[BUFFER_SIZE];
    int bytes_read;
    struct Client *client = (struct Client*) arg;
    int socket = client->client_socket;
    while ((bytes_read = read(socket, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[bytes_read] = '\0';  // Null-terminate the message
        printf("Client %d: %s\n", socket, buffer);  // Print the message received from the client
        // printList(client);
        // Echo the message back to the clients

        // If the client sends "exit", break the loop and close the connection
        if (strncmp(buffer, "exit", 4) == 0) {
            printf("Client %d disconnected.\n", socket);
            break;
        }
        brodcastClients(&clients, buffer);

    }
    pthread_mutex_lock(&mutex_lock);
    deleteClient(&clients, socket);
    pthread_mutex_unlock(&mutex_lock);

    close(socket);  // Close the client socket when done
}

void runServer(){
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Create the server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket to the address
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Start listening for client connections
    if (listen(server_socket, 10) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server started on port %d. Waiting for clients to connect...\n", PORT);
    // could you do a while true - if accept - then create a thread with that info?
    // Accept a single client connection (single-threaded)
    while(1){
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Client accept failed");
            close(server_socket);
            exit(EXIT_FAILURE);
        }
        // add to linked list
        printf("Client connected!\n");
        pthread_mutex_lock(&mutex_lock);
        addClient(&clients, client_socket);
        pthread_mutex_unlock(&mutex_lock);
        // printList(clients);
        pthread_t client_thread;

        // send client to thread to handle it on the thread
        if (pthread_create(&client_thread, NULL, handle_client, (void *)clients) != 0) {
            perror("Thread creation failed");
            pthread_mutex_lock(&mutex_lock);
            deleteClient(&clients, client_socket);
            pthread_mutex_unlock(&mutex_lock);
            close(client_socket);
        } else {
            pthread_detach(client_thread);
        }
    }

    // Close the server socket
    close(server_socket);
    pthread_mutex_destroy(&mutex_lock);
}