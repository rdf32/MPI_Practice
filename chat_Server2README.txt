1. Instructions on how to compile your program.
2. Steps to run your program.
3. The average running time of your program.
4. The expected output results when your program is run.
5. Any additional information to test the multithreaded behaviour

Program: chat_server2.c

1. compile the program -> gcc -pthread chat_server2.c core.c -o chat_server2
2. run the program -> ./chat_server2
This runs the server which waits for clients to connect (talk about how to connect multiple clients and what you should expect
it should broadcast what each client says back to the rest of the other clients)
3. I implemented the different threads to be handled in a linked list. The time complexity for iterating through a linked list
is O(n), therfore, any broadcasting operations or operations surrounding needing to address all currently connected clients
has an expected time complexity of O(n).
4. When you execute the program, the server should start listening for clients to connect and print that to the screen. Once 
clients are connected the server will print to the terminal that a client has connected and then print to the terminal any 
messages that client sends. The server will then broadcast those messages to all other clients.
5. You can connect multiple clients by creating multiple different terminals and calling "telnet 127.0.0.1 8080". Again each terminal
instance should be able to connect as a client and the server handles client connections and communication.



