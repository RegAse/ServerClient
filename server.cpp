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

string GenerateFortune();

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
string serverID;

void HandleClientCommand(string command, string body, client_data *client)
{
    int n;
    char buffer[256];
    if(command == "A")
    {
        cout << "Send Message to All Clients" << endl;
        cout << "From: " << client->thread_id << endl;
        for(int i = 0; i < 10; i++)
        {
            if(clients[i] != 0)
            {
                if(client->thread_id == data[i].thread_id)
                {
                    continue;
                }
                cout << "Send Message to:" << data[i].username << endl;
                strncpy(buffer, body.c_str(), sizeof(buffer));
                n = write(data[i].client_socket, body.c_str(), strlen(buffer));
                if (n < 0)
                    ErrorMessage("ERROR writing to socket");
            }
        }
    }
    else if(command == "M")
    {

    }
    else if(command == "W")
    {
        cout << "Sending list of users:" << endl;
        string userList = "==== Users On Server ====\n";
        for(int i = 0; i < 10; i++)
        {
            if(clients[i] != 0)
            {
                userList += data[i].username;
            }
        }
        userList += "=========================\n";
        strncpy(buffer, userList.c_str(), sizeof(buffer));
        n = write(client->client_socket, userList.c_str(), strlen(buffer));
        cout << userList << endl;
        if (n < 0)
            ErrorMessage("ERROR writing to socket");
    }
    else if(command == "I" || command == "K")
    {
        if(command == "K")
        {
            serverID = GenerateFortune();
        }
        cout << "Sending ID of server" << endl;
        strncpy(buffer, serverID.c_str(), sizeof(buffer));
        n = write(client->client_socket, buffer, strlen(buffer));
        if (n < 0)
            ErrorMessage("ERROR writing to socket");
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
            clients[client->thread_id] = NULL;
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
        HandleClientCommand(command, body, client);

        bzero(buffer, 256);
    }

    close(client->client_socket);
}

void ListenForConnections(int port)
{
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

string GenerateFortune()
{
    string command, data;
    FILE * stream;
    char buffer[256];
    command.append("fortune");

    stream = popen(command.c_str(), "r");
    if (stream) {
        while (!feof(stream))
        if (fgets(buffer, 256, stream) != NULL)
        {
            data.append(buffer);
        }
        pclose(stream);

    }
    return data;
}

int main()
{
    serverID = GenerateFortune();
    //serverID = popen("fortune", "r");
    cout << "Server starting..." << endl;
    cout << "ID: " << serverID;
    ListenForConnections(5232);
    return -1;
}
