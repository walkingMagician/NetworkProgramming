//#define _CRT_SECURE_NO_WARNINGS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <iostream>
#include <map>
#include <string>
#include <FormatLastError.h>
using namespace std;

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "FormatLastError.lib")

#define DEFAULT_PORT	"27015"
#define DEFAULT_BUFFER_LENGTH	1500

int clientID = 1;
map<SOCKET, int> clientIds;


void PrintClientInfo(SOCKET clientSocket);
DWORD WINAPI ClientHandler(LPVOID lParam);


void main()
{
	setlocale(LC_ALL, "");
	// 1) инициализация winSock
	WSADATA wsaData;
	INT iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		PrintLastEroor(WSAGetLastError());
		return;
	}
	
	// 2) проверяем не занят ли нам нужный порт 
	addrinfo* result = NULL;
	addrinfo hints;
	ZeroMemory(&hints, sizeof(hints)); 
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		cout << "getaddrinfo() failed with ";
		PrintLastEroor(WSAGetLastError());
		WSACleanup();
		return;
	}

	// 3) создаём сокет который будет слушаь и ждать подключение 
	SOCKET listen_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (listen_socket == INVALID_SOCKET)
	{
		cout << "socket() failed with ";
		PrintLastEroor(WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	// 4) BindSocket - связываем сокет с целевым ip-адресом и портом 
	iResult = bind(listen_socket, result->ai_addr, result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Bind() failed with ";
		PrintLastEroor(WSAGetLastError());
		closesocket(listen_socket);
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	// 5) включаем прослушивание Сокета
	iResult = listen(listen_socket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		cout << "listen() failed with";
		PrintLastEroor(WSAGetLastError());
		closesocket(listen_socket);
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	// 6) принимаем запросы от клиентов 
	/*SOCKET client_socket = accept(listen_socket, NULL, NULL);
	if (client_socket == INVALID_SOCKET)
	{
		cout << "accept() failed with ";
		PrintLastEroor(WSAGetLastError());
		closesocket(listen_socket);
		freeaddrinfo(result);
		WSACleanup();
		return;
	}*/

	//clientIds[client_socket] = clientID++;
	//PrintClientInfo(client_socket);

	// 7) получение и отправка данных
	/*
	CHAR recvbuffer[DEFAULT_BUFFER_LENGTH] = {};
	do
	{
		iResult = recv(client_socket, recvbuffer, DEFAULT_BUFFER_LENGTH, 0);
		if (iResult > 0)
		{
			printf("[Client %d] Receved bytes %i, Message: %s\n", clientIds[client_socket], iResult, recvbuffer);
			// Добавляем ID клиента в ответ
			string response = "[Server] Your ID: " + to_string(clientIds[client_socket]) +
				". Your message: " + recvbuffer;

			if (send(client_socket, recvbuffer, strlen(recvbuffer), 0) == SOCKET_ERROR)
			{
				cout << "send() failed with";
				PrintLastEroor(WSAGetLastError());
				break;
			}
		}
		else if (iResult == 0) cout << "Connection closing" << endl;
		else 
		{
			PrintLastEroor(WSAGetLastError());
		}
	} while (iResult > 0);
	*/
	while (true)
	{
		SOCKET client_socket = accept(listen_socket, NULL, NULL);
		if (client_socket == INVALID_SOCKET)
		{
			cout << "accept() failed with ";
			PrintLastEroor(WSAGetLastError());
			continue;
		}

		clientIds[client_socket] = clientID++;
		PrintClientInfo(client_socket);

		HANDLE hThread = CreateThread(NULL, 0, ClientHandler, (LPVOID)client_socket, 0, NULL);
		if (hThread == NULL)
		{
			cout << "Failed to create thread for client" << endl;
			clientIds.erase(client_socket);
			closesocket(client_socket);
		}
		else 
		{
			CloseHandle(hThread);
		}
	}

	//clientIds.erase(client_socket); // Удаляем клиента из map при отключении

	// ?) освобождение ресурсов 
	//closesocket(client_socket);
	closesocket(listen_socket);
	freeaddrinfo(result);
	WSACleanup();
}

void PrintClientInfo(SOCKET clientSocket)
{
	sockaddr_in clientAddr;
	int addrLen = sizeof(clientAddr);
	if (getpeername(clientSocket, (sockaddr*)&clientAddr, &addrLen) == SOCKET_ERROR)
	{
		PrintLastEroor(WSAGetLastError());
		return;
	}
	
	char ipStr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr, INET_ADDRSTRLEN);

	printf("Client IP: %s\nPort: %i\nAssigned ID: %d\n", ipStr, ntohs(clientAddr.sin_port), clientIds[clientSocket]);
}

DWORD WINAPI ClientHandler(LPVOID lParam)
{
	SOCKET client_socket = (SOCKET)lParam;
	int clientID = clientIds[client_socket];
	CHAR recvbuffer[DEFAULT_BUFFER_LENGTH] = {};
	int iResult;
	do
	{
		iResult = recv(client_socket, recvbuffer, DEFAULT_BUFFER_LENGTH, 0);
		if (iResult > 0)
		{
			printf("Receved bytes %i, Message: %s\n", iResult, recvbuffer);

			// Добавляем ID клиента в ответ
			string response = "[Server] Your ID: " + to_string(clientIds[client_socket]) +
				". Your message: " + recvbuffer;

			if (send(client_socket, response.c_str(), response.length(), 0) == SOCKET_ERROR)
			{
				printf("[Client %i] send() failed with", clientID);
				PrintLastEroor(WSAGetLastError());
				break;
			}
		}
		else if (iResult == 0) cout << "Connection closing" << endl;
		else
		{	
			printf("[Client %i]", clientID);
			PrintLastEroor(WSAGetLastError());
		}
	} while (iResult > 0);

	clientIds.erase(client_socket);
	closesocket(client_socket);
	return 0;
}
