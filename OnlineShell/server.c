#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024

// argc: number of arguments
// argv[]: array of given arguments, ex: {'./client', <server name>, <port>}
int main(int argc, char *argv[]) {

    int port = atoi(argv[1]);

    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t addressLength = sizeof(clientAddress);

    // make server socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    // setup address
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    // connect socket to ip and port
    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    // listen on port
    listen(serverSocket, 5); // keep 5 clients in queue

    while (1) {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &addressLength);

        printf("Client connected: %s:%d\n",
            inet_ntoa(clientAddress.sin_addr), //converts ip address from binary to string
            ntohs(clientAddress.sin_port)); 

        // fork clients
        pid_t pid = fork();

        // for child
        if (pid == 0) {
            close(serverSocket);

            char buffer[BUFFER_SIZE];

            while(1) {
                memset(buffer, 0, BUFFER_SIZE);

                int bytes = recv(clientSocket, buffer, BUFFER_SIZE, 0);
                if (bytes <= 0) break;

                buffer[strcspn(buffer, "\n")] = 0;

                if(strcmp(buffer, "quit") == 0)
                    break;
                    
                // replace stdout and stderr with socket
                dup2(clientSocket, STDOUT_FILENO);
                dup2(clientSocket, STDERR_FILENO);

                // parse the command to split into arguments
                char *args[64];
                int i = 0;
                char *token = strtok(buffer, " ");
                while (token != NULL) {
                    args[i++] = token;
                    token = strtok(NULL, " ");
                }
                args[i] = NULL;

                // execute command
                execvp(args[0], args);
                exit(1);
            }
            close(clientSocket);
            exit(0);
        } else {
            close(clientSocket);
        }
    }
    close(serverSocket);
    return 0;
}

// server should:
    // bind to tcp socket on a known port
        // waits for client connections
    // accept connections from clients
    // fork child process for each client that connects
    // display message: Client (IP address: port num) connected to the server successfully
    // execute command
    // send back output (stdout and stderr) to client
    // parent loops back to wait for more connections
