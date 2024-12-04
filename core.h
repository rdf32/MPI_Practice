// core.h
#ifndef CORE_H
#define CORE_H

#include <stdio.h>

// Macros
#define PORT 8080
#define BUFFER_SIZE 1024

// Struct Definitions
struct Client{
    int client_socket;
    struct Client *next;
    
};


// Global variable defintions
extern pthread_mutex_t mutex_lock;
extern struct Client *clients;


// Function Declarations
void addClient(struct Client** clients, int new_socket);
void deleteClient(struct Client ** clients, int client_socket);
void printList(struct Client* node);
void brodcastClients(struct Client** clients, const char *buffer);
void *handle_client(void *arg);
void runServer();

#endif // CORE_H
