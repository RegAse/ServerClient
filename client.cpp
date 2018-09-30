#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <string>
#include <pthread.h>

using namespace std;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

struct server_data {
    int server_socket;
};

/* Global variable */
pthread_t serverThread;
struct server_data data;

void *ReadFromServer(void *thread)
{
    cout << "=> Thread started for ReadFromServer" << endl;
    struct server_data *server;
    server = (struct server_data *) thread;

    int serverSocket = (int)(intptr_t)thread;
    int n;

    char buffer[256];
    while(true)
    {
        bzero(buffer, 256);

        n = read(server->server_socket, buffer, 255);
        cout << buffer << endl;

        if (n < 0)
             error("ERROR reading from socket");
    }
}

int main()
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;           // Socket address structure
    struct hostent *server;
    string username = "";

    char buffer[256];

    portno = 5232;     // Read Port No from command line

    sockfd = socket(AF_INET, SOCK_STREAM, 0); // Open Socket

    if (sockfd < 0)
        error("ERROR opening socket");

    server = gethostbyname("localhost");        // Get host from IP

    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET; // This is always set to AF_INET

    // Host address is stored in network byte order
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);

    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    /* Now we ask for username */
    printf("Please enter a username (more than 3 characters):");

    bzero(buffer, 256); // Empty buffer.
    fgets(buffer, 254, stdin); // Read from console.

    // The FIRST char in the buffer is reserved for commands
    string command ("C");
    username = buffer;
    string tosend = command + username;
    strncpy(buffer, tosend.c_str(), sizeof(buffer));

    n = write(sockfd, buffer, strlen(buffer));

    username = username.substr(0, username.length()-1);

    /* TODO: make client listen to server socket in another thread */

    // Read from server
    data.server_socket = sockfd;
    int rc = pthread_create(&serverThread, NULL, ReadFromServer, (void *)&sockfd);

    // Read and write to socket
    while (true)
    {
        printf("Please enter the message: ");

        bzero(buffer, 256); // Empty buffer.
        fgets(buffer, 254, stdin); // Read from console.

        string command ("A"); // SEND TO ALL USERS
        string message (buffer);
        string fool = command + "[" + username + "]: " + message;
        strncpy(buffer, fool.c_str(), sizeof(buffer));

        n = write(sockfd, buffer, strlen(buffer));

        if (n < 0)
            error("ERROR writing to socket");

    }
    close(sockfd);
    return 0;
}
