#include "clientFunctions.h"
#include <iostream>
#include <string>
#include "FormatLastError.h"

using namespace std;

void PrintClientInfo(SOCKET clientSocket)
{
	sockaddr_in clientAddr;
	int addrLen = sizeof(clientAddr);
	if (getpeername(clientSocket, (sockaddr*)&clientAddr, &addrLen) == SOCKET_ERROR) // �������� �������� ����������
	{
		PrintLastEroor(WSAGetLastError());
		return;
	}

	char ipStr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr, INET_ADDRSTRLEN); // �� ��������� ������� � ��������� 

	printf("Client IP: %s\nPort: %i\nAssigned ID: %d\n", ipStr, ntohs(clientAddr.sin_port), clientIds[clientSocket]);
}

DWORD WINAPI ClientHandler(LPVOID lParam)
{
	ThreadParameters* params = (ThreadParameters*)lParam;
	SOCKET client_socket = params->socket;
	int hClientID = params->id;

	// ��������
	ThreadParameters recvParamet = *params;
	ThreadParameters sendParamet = *params;

	HANDLE hRevcThread = CreateThread(NULL, 0, RecvThread, &recvParamet, 0, NULL);
	HANDLE hSendThread = CreateThread(NULL, 0, SendThread, &sendParamet, 0, NULL);


	if (hRevcThread == NULL || hSendThread == NULL)
	{
		cerr << "Failed to created thread for client " << hClientID << endl;

		lock_guard<mutex> lock(mtx_client);
		clientIds.erase(client_socket);
		closesocket(client_socket);
		PrintLastEroor(WSAGetLastError());
		delete params;
		return 1;
	}

	// ��� ���������� �������
	WaitForSingleObject(hRevcThread, INFINITE);
	params->run_flag = false;
	WaitForSingleObject(hSendThread, INFINITE);

	// ��������� �����������
	CloseHandle(hRevcThread);
	CloseHandle(hSendThread);

	// ������� ������� 
	{
		lock_guard<mutex> lock(mtx_client);
		clientIds.erase(client_socket);
	}

	closesocket(client_socket);
	delete params;

	return 0;
}

DWORD WINAPI RecvThread(LPVOID lParam)
{
	ThreadParameters* params = (ThreadParameters*)lParam;
	SOCKET recv_client_socket = params->socket;
	int recv_clientID = params->id;

	CHAR recv_buffer[DEFAULT_BUFFER_LENGTH] = {};
	int iResult;

	while (params->run_flag)
	{
		iResult = recv(recv_client_socket, recv_buffer, DEFAULT_BUFFER_LENGTH, 0);
		if (iResult > 0)
		{
			//recv_buffer[iResult] = '\0'; // �������� �� ����
			printf("[Client %i] received %d bytes, message: %s\n", recv_clientID, iResult, recv_buffer);
		}
		else if (iResult == 0)
		{
			printf("[Client %i] disconected\n", recv_clientID);
			params->run_flag = false;
			return 1;
		}
		else
		{
			printf("[Client %i] Error recv: ", recv_clientID);
			PrintLastEroor(WSAGetLastError());
			params->run_flag = false;
			return 1;
		}
	}
	return 0;
}

DWORD WINAPI SendThread(LPVOID lParam)
{
	ThreadParameters* params = (ThreadParameters*)lParam;
	SOCKET send_client_socket = params->socket;
	int send_clientID = params->id;

	string response = "[Server] Your ID: " + to_string(send_clientID);
	int iResult = send(send_client_socket, response.c_str(), response.length(), 0);

	if (iResult == SOCKET_ERROR)
	{
		printf("Client %i Error send", send_clientID);
		PrintLastEroor(WSAGetLastError());
		params->run_flag = false;
		return 1;
	}

	return 0;
}