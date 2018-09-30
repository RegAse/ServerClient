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
        cout << buffer;

        if (n < 0)
             error("ERROR reading from socket");
    }
}

void ShowCommandList()
{
    cout << "Welcome to the chat!" << endl;
    cout << "Type \\ and then the command letter, example: ´\\M <Bobo>: Hello What is up´" << endl;
    cout << "Commands available are:" << endl;
    cout << "A: Sends message to all users (default no need to type \A)" << endl;
    cout << "M: Send message to single user" << endl;
    cout << "W: List of users on the server" << endl;
    cout << "L: Leave chat and exit program" << endl;
    cout << "I: Shows the id of the server" << endl;
    cout << "K: Changes the id of the server" << endl;
    cout << "============================================================================" << endl << endl;
}

void HandleClientCommand(string username,string command, string body, string parameter1, int serverSocket);

int main()
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;           // Socket address structure
    struct hostent *server;
    string username = "";

    char buffer[512];

    portno = 5235;     // Read Port No from command line

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

    bzero(buffer, 512); // Empty buffer.
    fgets(buffer, 510, stdin); // Read from console.

    // The FIRST char in the buffer is reserved for commands
    string command ("C");
    username = buffer;
    string tosend = command + username;
    strncpy(buffer, tosend.c_str(), sizeof(buffer));

    n = write(sockfd, buffer, strlen(buffer));

    username = username.substr(0, username.length()-1);

    // Read from server in another thread.
    data.server_socket = sockfd;
    int rc = pthread_create(&serverThread, NULL, ReadFromServer, (void *)&sockfd);

    ShowCommandList();
    // Write to the server.
    while (true)
    {
        bzero(buffer, 512); // Empty buffer.
        fgets(buffer, 510, stdin); // Read from console.

        string message (buffer);
        string command, parameter, body;
        body = message;
        if(message.substr(0, 1) == "\\")
        {
            command = message[1];
            body = body.substr(2, body.length());
        }
        else
        {
            command = "A";
        }

        HandleClientCommand(username, command, body, parameter, sockfd);
    }
    close(sockfd);
    return 0;
}

void HandleClientCommand(string username, string command, string body, string parameter1, int serverSocket)
{
    int n;
    char buffer[512];
    if(command == "A" || command == "W" || command == "I" || command == "K" || command == "M")
    {
        string response = command + "[" + username + "]: " + body;
        strncpy(buffer, response.c_str(), sizeof(buffer));

        n = write(serverSocket, buffer, strlen(buffer));

        if (n < 0)
            error("ERROR writing to socket");
    }
    else if(command == "L")
    {
        cout << "Leaving...." << endl;
        close(serverSocket);
        exit(1);
    }
}
