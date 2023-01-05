#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "8001"
#

int main(void) {
	// Initialize Variables
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL, * ptr = NULL, hints;
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;
	char* sendbuf;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d", iResult);
		return 1;
	}

	// Zero the memory of hints
	ZeroMemory(&hints, sizeof(hints));
	// Fill address info structure
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve server address and port -> hardcoded my ip address, not sure if this will work
	iResult = getaddrinfo("192.168.1.12", DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d", iResult);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a socket for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed: %d", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connect to server. CLose connection if it fails and reset ConnectSocket back to its default value 
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break; // Break after success or none of the addrinfo structs connect
	}

	freeaddrinfo(result);

	// Check if ClientSocket was able to connect
	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	// Receieve Welcome message
	iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	recvbuf[iResult] = '\0';
	if (iResult > 0)
		printf("%s\n", recvbuf);
	else if (iResult == 0)
		printf("Connection closed\n");
	else
		printf("recv failed with error: %d\n", WSAGetLastError());
	ZeroMemory(&recvbuf, sizeof(recvbuf));

	while (1) {
		// SEND
		sendbuf = (char*)malloc(DEFAULT_BUFLEN);
		printf("Enter message to send ('quit to quit'): ");
		fgets(sendbuf, DEFAULT_BUFLEN, stdin);
		if (strcmp(sendbuf, "quit\n")) {
			iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
			if (iResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
			}
		}else
			break;
		free(sendbuf);

		// RECIEVE
		//   Recieve the number that is sent
		int num;
		memset(recvbuf, 0, recvbuflen);
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("%s", recvbuf);
		}
		else {
			printf("Error with recv: %d", iResult);
			closesocket(ConnectSocket);
			WSACleanup();
			return 1;
		}
		
		//  Using the number that was sent, loop and receive the messages and print them
		sscanf_s(recvbuf, "%d", &num);
		memset(recvbuf, 0, recvbuflen);
		for (int i = 0; i < num; i++) {
			iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
			if (iResult > 0) {
				printf("%s", recvbuf);
			}
			else {
				printf("Error with recv: %d", iResult);
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
			}
			
		}
		

	}
	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	return 0;

}