#define WIN32_LEAN_AND_MEAN

// Add a comment explaining why each library is needed

#include <windows.h> // This library is used to access Win32 API functions that makes it easy to use windows specific functionality.
#include <winsock2.h>// This library is used to initialize and use winsock that allows processes to communicate with eachother.
                     //      It inludes important functions like bind, connect, listen, accept, and send as well as many more functions.
#include <ws2tcpip.h>// This library is used for TCP/IP specific information used by winsock2.  It includes important functions like getaddrinfo and freeaddrinfo that are used
                     //      for the sever and many other functions used for windows sockets.
#include <stdlib.h>  // This library is needed to use sizeof() function and size_t variable type as well as others.
                     //      It is also needed for memory allocation functions like malloc, calloc, and free amongst many other functions.
#include <stdio.h>   // This library is needed for all standard input and output functions.

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

//Add a comment explaining what each #define line is used for
#define DEFAULT_BUFLEN 512 // Defines the buffer length which is used later as the size of the buffer for the received message from the client.
#define DEFAULT_PORT "8001"// Defines the default port as port 8001 which will be used to create the socket 


//Add a comment explaining what this struct is used for
// It is used to hold information about a client that connects to the server.  It holds the client socket itself as well as the id
//      which based off the number of clients have connected, starting at 0. (1st client gets id 0, 2nd client gets id 1 ... )
struct threadinfo {
    SOCKET ClientSocket;
    int id;
};



//Add a comment explaining what this function does
DWORD WINAPI handleClient(LPVOID IpParameter) {
    
    //Add a comment indicating what each variable is used for
    struct threadinfo* temp = (struct threadinfo*)IpParameter; // Crates a temporary threadinfo struct that hold the client socket and its id
    int iSendResult; // Holds the the nuber of bytes sent when the server responds to the client, -1 (SOCKET_ERROR) if there is an error while sending.
    int iResult;  // Holds the number of bytes recieved form the client, -1 (SOCKET_ERROR) if there is an error while recieving the data.
    char recvbuf[DEFAULT_BUFLEN]; // A buffer where the message sent by the client is stored.
    int recvbuflen = DEFAULT_BUFLEN; // Holds the default length of a buffer used as the size parameter for functions used on recvbuf.
    SOCKET ClientSocket = temp->ClientSocket; // ClientSocket used to communicate back to the client.
    int id = temp->id; // id used to uniquely identify theclients that connect to the server.
    int random; // Initialization of the random integer that is generated to represent how many times to echo the message back to the client.
    char response[DEFAULT_BUFLEN] = "!quit!"; // Stores the string "!quit!" but is not used anywhere else in the code.

    printf("Connecting Client %d.\n", id);
    char welcomeMessage[DEFAULT_BUFLEN];
    sprintf_s(welcomeMessage, sizeof(welcomeMessage), "Client %d, Thank you for connecting", id);

    //Add a comment describing what the next 8 lines of code do
    //  Sends a welcome message to the client, if there is a socket error it prints it to the console and closes connection with the client
    iSendResult = send(ClientSocket, welcomeMessage, strlen(welcomeMessage), 0);
    if (iSendResult == SOCKET_ERROR)
    {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    //Add a comment describing what this loop does
    do {
        memset(recvbuf, 0, recvbuflen); //needed to fix buffer content
        //What does the following line do?
        //  Takes a connected client socket and stores the recieved message in recvbuf and returns the number of bytes recieved.
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        
        //When does this if statement trigger?
        //  If a message is receieved (bytes recieved > 0) it triggers a reply back to the client.
        if (iResult > 0) {

            //Describe the following 5 lines of code.
            //  Prints a "Receieved: " message that echo's to the commandline the message that whas recieved.
            //  Then it checks if the client sent the message to quit, if so, it breaks out of the do while loop and clsoes the connection with the client
            printf("Recieved: %s\n", recvbuf);
            if (strcmp(recvbuf, "quit") == 0) {
                printf("Disconnecting Client %d", id);
                break;
            }

            //Describe the following 12 lines of code.
            // Creates a random integer from 2-10 and echos the message sent by the client back to the client the amount of times of the random integer.
            // If there is an error it will print to the console that it failed, close the connection, and run WSACleanup()
            random = 2 + (rand() % 9); 
            char randomstr[3];
            sprintf_s(randomstr, 3, "%d", random);
            printf("random %s\n", randomstr);
            Sleep(100);
            iResult = send(ClientSocket, randomstr, strlen(randomstr), 0);
            if (iSendResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
            
            
            //Describe the following 14 lines of code
            //  If the client send the message "test" it will echo back test a random amount of times but also print to the server console "Yes" the same amount of times.
            for (int i = 0; i < random; i++) {
                if (strcmp(recvbuf, "test\n") == 0) {
                    printf("Yes");
                }
                Sleep(100);
                iSendResult = send(ClientSocket, recvbuf, strlen(recvbuf), 0);

                if (iSendResult == SOCKET_ERROR) {
                    printf("send failed with error: %d\n", WSAGetLastError());
                    closesocket(ClientSocket);
                    WSACleanup();
                    return 1;
                }
            }
            
            
        }
        //Under what circumstances does this branch trigger?
        //  When the Client closes the connection
        else if (iResult == 0) {
            printf("Connection closed\n");
        }
        //Describe what this branch does. Under what circumstances does this branch trigger? 
        //  If the recv() fuction returns a value less that 0 due to an error, then an arror message is printed and the connection with the client is closed
        else {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

    } while (iResult > 0);

    //Describe the following 8 lines of code
    // It tries to shutdown the socket, unlike close(), shutdown() can still receive pending data, then closes the client socket -> https://stackoverflow.com/questions/4160347/close-vs-shutdown-socket
    //  If the shutdown fails, it prints and error and closes the connection with the client.
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }
    closesocket(ClientSocket);


    return 0;
}

int main(void)
{

    //Describe what each variable does
    int id = 0; // Hold the id for a client starting at 0 and increases by one everytime a client connects
    WSADATA wsaData; // Used to initialize winsock
    int iResult; // Stores result of different winsock functions and uses its return value to error check
    SOCKET ListenSocket = INVALID_SOCKET; // Initializes the ListenSocket to listen for connections
    SOCKET ClientSocket = INVALID_SOCKET; // Initializes the ClientSocket to communicate with the client once connected
    struct addrinfo* result = NULL; // Holds address information about the socket 
    struct addrinfo hints; // Holds address information about the socket
    int iSendResult; // Holds the result of sending data back to the client, doesnt seem to be used in main.
    char recvbuf[DEFAULT_BUFLEN]; // Initializes the buffer with the default buffer length.
    int recvbuflen = DEFAULT_BUFLEN; // sets the received buffer length to the default length of a buffer

    
    
    //Describe what the following 5 lines of code do.
    //  Initializes winsock with WSAStartup and then does and error check. If the initialization is not successful and prints an error message.
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    //Describe what the following 5 lines of code do
    memset(&hints, 0, sizeof hints); // Sets all of the memory in hints to 0.
    hints.ai_family = AF_INET; // Sets IPv4 adress family
    hints.ai_socktype = SOCK_STREAM; // Sets stream socket
    hints.ai_protocol = IPPROTO_TCP; // Sets IP protocol to TCP
    hints.ai_flags = AI_PASSIVE; // Flag to indicate caller will use returned socket address in bind later

    //Describe what the following 6 ines of code do
    //  Resolves the local addres and port to be used by the server.
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    //Describe what the following 7 lines of code do
    //  Creates a listener socket that will be used to listen for connections on the port.
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    //Describe what the following 8 lines of code do
    //  Binds the created listner socket to an ip address and port.
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    //Why is this line needed?
    //  It frees the the memory allocated for the addrinfo struct result
    freeaddrinfo(result);

    //Describe what the following 7 lines of code do\
    //  The ListenSocket now begins listening for a connection unless there is an error, then a error messgae is printed and the socket is closed.
    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    printf("server is listening\n");

    
    //What does this loop do?
    //  Infinite loops so that it can accept multiple clients and use threads to support multiple clients.
    while (1)
    {    
        //Describe what the following 7 lines of code do
        //  Accepts the client connection upon request, if there is an error a message is printed and the socket is closed.
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }

        //Describe what the following 12 lines of code do
        //  Creates a thread for the client that connected and the handleClient function is used to communicate with the client.
        HANDLE handle;
        struct threadinfo temp;
        temp.ClientSocket = ClientSocket;
        temp.id = id;
        handle = CreateThread(NULL, 0, handleClient, &temp, 0, NULL);
        if (handle == NULL) {
            printf("Thread was unable to be made\n");
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }
        id += 1;
    }
}
