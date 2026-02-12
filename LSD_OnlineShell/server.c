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

int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return -1;
    }

    int port = atoi(argv[1]);

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocket < 0) { 
        perror("Socket failed"); return -1; 
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    if(bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Bind failed"); return -1;
    }

    if(listen(serverSocket, 5) < 0) { 
        perror("Listen failed"); return -1;
    }

    printf("Server listening on port %d\n", port);

    while(1) {
        struct sockaddr_in clientAddress;
        socklen_t addressLength = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &addressLength);
        if(clientSocket < 0) { 
            perror("Accept failed"); 
        } else {
            continue; 
        }

        printf("Client %s:%d connected to the server successfully\n",
               inet_ntoa(clientAddress.sin_addr),
               ntohs(clientAddress.sin_port));

        pid_t pid = fork();
        if(pid < 0) { 
            perror("Fork failed"); close(clientSocket); continue; 
        }

        if(pid == 0) { // child handle the client
            close(serverSocket); // child doesn’t need the listening socket
            char buffer[BUFFER_SIZE];

            while(1) {
                memset(buffer, 0, BUFFER_SIZE);
                int bytes = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
                if(bytes <= 0) break; // client disconnected

                buffer[bytes] = '\0';
                buffer[strcspn(buffer, "\n")] = 0; // remove newline

                if(strcmp(buffer, "quit") == 0) break;

                // fork to run the command
                pid_t cmdPid = fork();
                if(cmdPid < 0) { 
                    perror("Command fork failed"); continue; 
                }

                if(cmdPid == 0) { // grandchild executes command
                    // redirect stdout and stderr to socket
                    dup2(clientSocket, STDOUT_FILENO);
                    dup2(clientSocket, STDERR_FILENO);

                    // parse command into args
                    char *args[64];
                    int i = 0;
                    char *token = strtok(buffer, " ");
                    while(token != NULL) {
                        args[i++] = token;
                        token = strtok(NULL, " ");
                    }
                    args[i] = NULL;

                    if(args[0] == NULL) exit(1); // no command

                    execvp(args[0], args);
                    perror("execvp failed"); // if exec fails
                    exit(1);
                } else {
                    int status;
                    waitpid(cmdPid, &status, 0); // wait for command to finish
                }
            }

            close(clientSocket);
            exit(0);
        } else { // PARENT: back to listening for new clients
            close(clientSocket);
        }
    }

    close(serverSocket);
    return 0;
}
