1. Instructions on how to compile your program.
2. Steps to run your program.
3. The expected output results when your program is run.



IMPORTANT: before you compile the program, you will find a preprocessor directive LOG_FILE at the top of the c file.
You need to update this path to to path of where you would like your log file to be. The log file is created and where
information from the server is logged while it is executing.

1. compile the program -> gcc -pthread chat_server3.c core.c -o chat_server3
2. run the program -> ./chat_server3
    When you run the program, the PID will be printed to the console. Since this creates a daemon processes, the only way
    to terminate the program is to manually kill the processes
    end the process -> kill (PID) where PID is the process running the chat_server2
    additionally you can check the status of the process by call ps -p PID or print all running processes with ps aux
3.When you execute the program it will create a daemon server process and print the PID to the terminal. like
the other chat_server2 program, the server will handle multiple client connections and broadcast client communcation to all clients.
This particular program does not print server information / communcation to the screen. Rather, all server information
is logged in the Log.txt file which you set the path for before compiling the program.
4. You can connect multiple clients by creating multiple different terminals and calling "telnet 127.0.0.1 8080". Again each terminal
instance should be able to connect as a client and the server handles client connections and communication.
