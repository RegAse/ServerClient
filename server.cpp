#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <pthread.h>
#include <cstdlib>

using namespace std;

/* Show error message */
void ErrorMessage(const char *msg)
{
    perror(msg);
}

/* Fatal error will stop program. */
void FatalErrorMessage(const char *msg)
{
    perror(msg);
    exit(1);
}

struct client_data {
    int thread_id;
    int client_socket;
    string username;
};

/* Global Variables */
pthread_t clients[10]; // 10 Clients
struct client_data data[10];

void HandleClientCommand(string command, client_data *client)
{
    if(command == "C")
    {
        // Client can call this again to change his username.
        cout << "I GOT CONNECT COMMAND" << endl;
    }
    else if(command == "A")
    {
        cout << "Send Message to All Clients" << endl;
        cout << "From: " << client->username << endl;
    }
}

void *ReadFromClient(void *thread)
{
    cout << "=> Thread started for ReadFromClient" << endl;
    struct client_data *client;
    client = (struct client_data *) thread;

    int n;
    char buffer[256];
    bool hasConnection = true;
    while(hasConnection)
    {
        cout << "Reading from client socket." << endl;

        n = read(client->client_socket, buffer, 255);
        if(n < 1)
        {
            ErrorMessage("Lost connection to client.");
            hasConnection = false;
            break;
        }
        /* The response should always contain a command. */
        string response (buffer);
        string command = response.substr(0, 1);
        string body = response.substr(1, response.length());

        cout << "Command = " << command << ", Body = " << body << endl;

        cout << body << endl;

        if(command == "C")
        {
            client->username = body;
            cout << "SETTING USERNAME" << endl;
        }
        HandleClientCommand(command, client);

        bzero(buffer, 256);
    }

    close(client->client_socket);
}

void ListenForConnections(int port)
{
    //pthread_t clients[10]; // 10 Clients
    //struct client_data data[10];

    int serverSock, clientSock;

    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    int clientCount = 0;

    serverSock = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSock < 0)
        ErrorMessage("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    bool hasFatalError = false;
    /* Binding Server Socket */
    if (bind(serverSock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        ErrorMessage("ERROR on binding");
        hasFatalError = true;
    }

    while(!hasFatalError)
    {
        listen(serverSock, 5);

        clilen = sizeof(cli_addr);

        cout << "Listening for connections" << endl;
        clientSock = accept(serverSock, (struct sockaddr *) &cli_addr, &clilen);
        if (clientSock < 0)
        {
            ErrorMessage("ERROR on accept");
        }

        bzero(buffer, 256);

        // Create Thread to read from client.
        data[clientCount].thread_id = clientCount;
        data[clientCount].client_socket = clientSock;
        data[clientCount].username = "NOT SET";

        int rc = pthread_create(&clients[clientCount], NULL, ReadFromClient, (void *)&data[clientCount]);

        cout << "Thread created to handle current client, i am going to start listening again." << endl;
        clientCount += 1;
    }
    close(clientSock);
    close(serverSock);
}

int main()
{
    ListenForConnections(5232);
    return -1;
    pthread_t clients[10]; // 10 Clients
    struct client_data data[10];

    int port = 5231;
    int serverSock, clientSock;

    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    serverSock = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSock < 0)
        ErrorMessage("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    /* Binding Server Socket */
    if (bind(serverSock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        ErrorMessage("ERROR on binding");

    listen(serverSock, 5);

    clilen = sizeof(cli_addr);

    cout << "Listening for connections" << endl;
    clientSock = accept(serverSock, (struct sockaddr *) &cli_addr, &clilen);
    if (clientSock < 0)
    {
        ErrorMessage("ERROR on accept");
    }

    bzero(buffer,256);

    // Create Thread to read from client.
    data[0].thread_id = 1;
    data[0].client_socket = clientSock;

    int rc = pthread_create(&clients[0], NULL, ReadFromClient, (void *)&data[0]);

    cout << "Thread created to handle current client, i am going to start listening again." << endl;

    //ReadFromClient(clientSock);
    /*while(true)
    {
        cout << "Reading from client socket." << endl;
        n = read(clientSock, buffer, 255);
        printf("Message from USERNAME: %s\n", buffer);
    }*/

    if (n < 0) ErrorMessage("ERROR reading from socket");
    printf("Here is the message: %s\n", buffer);

    n = write(clientSock,"I got your message", 18);

    if (n < 0) ErrorMessage("ERROR writing to socket");

    /* Close sockets */
    close(clientSock);
    close(serverSock);
    return 0;
}
