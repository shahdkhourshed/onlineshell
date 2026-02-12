#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFFER_SIZE 1024

// argc: number of arguments
// argv[]: array of given arguments, ex: {'./client', <server name>, <port>}
int main(int argc, char *argv[]) {
    // catch incorrect input from users
    if (argc != 3) {
        printf("format should be %s <server_ip> <port> \n", argv[0]);
        return -1;
    }

    char *host = argv[1];
    int port = atoi(argv[2]);

    int currSocket;
    struct sockaddr_in serverAddress; // make struct of type sockaddr_in to store info for server address
    struct hostent *hostName; // make struct of type hostent to store info for host

    // creating socket
    currSocket = socket(AF_INET, SOCK_STREAM, 0);
    // getting host
    hostName = gethostbyname(host);

    // set up address struct
    serverAddress.sin_family = AF_INET; // IPv4 connection
    serverAddress.sin_port = htons(port); // port to connect to
    memcpy(&serverAddress.sin_addr.s_addr,
        hostName->h_addr,
        hostName->h_length); // ip address

    // connect
    if (connect(currSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Connection failed");
        close(currSocket);
        return -1;
    }
    
    printf("Connected to the server %s: %d successfully\n", host, port);

    char buffer[BUFFER_SIZE];

    while (1) {
        printf("osh>"); // prompts user to enter input
        fflush(stdout); // writes whatever is in stdout buffer and empties stdout so it makes prompt appear instantly

        memset(buffer, 0, BUFFER_SIZE); // clears previous input
        fgets(buffer, BUFFER_SIZE, stdin); //read user input

        buffer[strcspn(buffer, "\n")] = 0; //removes new line from buffer before sending command

        //check for quit command
        if (strncmp(buffer, "quit", 4) == 0)
            break;
        
        // send command to server
        printf("DEBUG: Sending command: '%s'\n", buffer);
        send(currSocket, buffer, strlen(buffer), 0);

        memset(buffer, 0, BUFFER_SIZE);
        //HANGING HERE
        recv(currSocket, buffer, BUFFER_SIZE, 0); // read data from server

        // display output
        printf("%s", buffer);
    } 

    // close socket after quit sequence
    close(currSocket);

    return 0;
}

// client should do all these things:
    // user specifies host and port in command line
    // connect to server using tcp socket
    // message: Connected to the server (ip address: port num) successfully
    // client sends command to server
    // client displays output received from server to stdout
    // waits for next command from user
    // does not close/exit until user enters quit command
