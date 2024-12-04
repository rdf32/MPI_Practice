#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "core.h"

#define LOG_FILE "/mnt/c/Users/it/Desktop/PhD/Fall2024/CSC718/homework_1/Log.txt"

struct threadArgs {
    struct Client *client;
    FILE *fp;
};

void *handle_client_daemon(void *arg) {
    char buffer[BUFFER_SIZE];
    int bytes_read;
    struct threadArgs *args = (struct threadArgs *)arg;
    struct Client *client = args->client;
    FILE *fp = args->fp;
    int socket = client->client_socket;
    while ((bytes_read = read(socket, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[bytes_read] = '\0';  // Null-terminate the message

        // printList(client);
        // Echo the message back to the clients

        // If the client sends "exit", break the loop and close the connection
        if (strncmp(buffer, "exit", 4) == 0) {
            fprintf(fp, "Client %d disconnected.\n", socket);
            fflush(fp);
            break;
        }
        fprintf(fp, "Client %d: %s\n", socket, buffer);  // Print the message received from the client
        fflush(fp);
        brodcastClients(&clients, buffer);

    }
    pthread_mutex_lock(&mutex_lock);
    deleteClient(&clients, socket);
    pthread_mutex_unlock(&mutex_lock);

    close(socket);  // Close the client socket when done
}

int main(int argc, char* argv[])
{
	FILE *fp= NULL;
	pid_t process_id = 0;
	pid_t sid = 0;
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

	// Create child process
	process_id = fork();

	// Indication of fork() failure
	if (process_id < 0)
	{
		printf("fork failed!\n");
		// Return failure in exit status
		exit(1);
	}

	// PARENT PROCESS. Need to kill it.
	if (process_id > 0)
	{
		printf("process_id of child process %d \n", process_id);
		// return success in exit status
		exit(0);
	}

	//unmask the file mode
	umask(0);

	//set new session
	sid = setsid();

	if(sid < 0)
	{
		// Return failure
		exit(1);
	}

	// Change the current working directory to root.
	chdir("/");


	// Close stdin. stdout and stderr
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	// Open a log file in write mode.
	fp = fopen (LOG_FILE, "w+");
    if (fp == NULL) {
        perror("Failed to open log file");
        exit(1);
    }


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
        sleep(1);
		// fprintf(fp, "Logging info...\n");
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            fprintf(fp, "Client accept failed");
            close(server_socket);
            exit(EXIT_FAILURE);
        }
        // add to linked list
        fprintf(fp,"Client connected!\n");
        pthread_mutex_lock(&mutex_lock);
        addClient(&clients, client_socket);
        pthread_mutex_unlock(&mutex_lock);
        // printList(clients);
        pthread_t client_thread;
        struct threadArgs *args = malloc(sizeof(struct threadArgs));
        args->client = clients;
        args->fp = fp;

        // send client to thread to handle it on the thread
        if (pthread_create(&client_thread, NULL, handle_client_daemon, (void *)args) != 0) {
            fprintf(fp,"Thread creation failed");
            pthread_mutex_lock(&mutex_lock);
            deleteClient(&clients, client_socket);
            pthread_mutex_unlock(&mutex_lock);
            close(client_socket);
        } else {
            pthread_detach(client_thread);
        }
	    fflush(fp);
    }

    // Close the server socket
    close(server_socket);
    pthread_mutex_destroy(&mutex_lock);
	fclose(fp);

	return (0);
}
