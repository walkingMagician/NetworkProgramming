#include "clientFunctions.h"
#include <iostream>
#include <string>
#include "FormatLastError.h"

using namespace std;

void PrintClientInfo(SOCKET clientSocket) // вывод информации о подключеённым клиенте
{
	// получаем информацию о клиенте
	sockaddr_in clientAddr;
	int addrLen = sizeof(clientAddr);
	if (getpeername(clientSocket, (sockaddr*)&clientAddr, &addrLen) == SOCKET_ERROR) // получаем адрусную информацию
	{
		PrintLastEroor(WSAGetLastError());
		return;
	}

	// преобразуем из бинарного формата в строковый
	char ipStr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr, INET_ADDRSTRLEN);  

	// вывод инфы 
	printf("Client IP: %s\nPort: %i\nAssigned ID: %d\n", ipStr, ntohs(clientAddr.sin_port), clientIds[clientSocket]);
}

DWORD WINAPI ClientHandler(LPVOID lParam) // обработка соединение с клиентом
{
	// получаем параметры от переданной структуры 
	ThreadParameters* params = (ThreadParameters*)lParam; // приводим params к нашему типу
	SOCKET client_socket = params->socket;
	int hClientID = params->id;

	// копируем параметры для потоков
	ThreadParameters recvParamet = *params;
	ThreadParameters sendParamet = *params;

	// создаем потоки отправки и приёма
	HANDLE hRevcThread = CreateThread(NULL, 0, RecvThread, &recvParamet, 0, NULL);
	HANDLE hSendThread = CreateThread(NULL, 0, SendThread, &sendParamet, 0, NULL);

	// проверка на то как успешно создыны потоки 
	if (hRevcThread == NULL || hSendThread == NULL)
	{
		cerr << "Failed to created thread for client " << hClientID << endl;

		// удаление клиента 
		lock_guard<mutex> lock(mtx_client);
		clientIds.erase(client_socket);
		closesocket(client_socket);
		PrintLastEroor(WSAGetLastError());
		delete params;
		return 1;
	}

	// ждём завершение потоков 
	WaitForSingleObject(hRevcThread, INFINITE); // INFINITE - бечконечное ожидание
	params->run_flag = false; // сигнал что поток hRevcThread закончил работу 
	WaitForSingleObject(hSendThread, INFINITE);

	// закрываем дескрипторы потоков
	CloseHandle(hRevcThread);
	CloseHandle(hSendThread);

	// удаялем клиента 
	{
		lock_guard<mutex> lock(mtx_client);
		clientIds.erase(client_socket);
	}

	// освобождение памяти  и закрытие сокета
	closesocket(client_socket);
	delete params;

	return 0;
}

DWORD WINAPI RecvThread(LPVOID lParam) // приём данных от клиента
{
	// получаем параметры от переданной структуры 
	ThreadParameters* params = (ThreadParameters*)lParam; // приводим params к нужному типу
	SOCKET recv_client_socket = params->socket;
	int recv_clientID = params->id;

	CHAR recv_buffer[DEFAULT_BUFFER_LENGTH] = {};
	int iResult;

	while (params->run_flag)
	{
		// приём данных от клиента
		iResult = recv(recv_client_socket, recv_buffer, DEFAULT_BUFFER_LENGTH, 0);
		if (iResult > 0) // успех
		{
			//recv_buffer[iResult] = '\0'; // проверка на ноль на всякий случай 
			printf("[Client %i] received %d bytes, message: %s\n", recv_clientID, iResult, recv_buffer);
		}
		else if (iResult == 0) // отключение
		{
			printf("[Client %i] disconected\n", recv_clientID);
			params->run_flag = false;
			return 1;
		}
		else // ошибка приёма 
		{
			printf("[Client %i] Error recv: ", recv_clientID);
			PrintLastEroor(WSAGetLastError());
			params->run_flag = false; // устанавливаем флаг = false если клиент отключён или ошибка 
			return 1;
		}
	}
	return 0;
}

DWORD WINAPI SendThread(LPVOID lParam) // отправка даных клиенту
{
	// получаем параметры от переданной структуры 
	ThreadParameters* params = (ThreadParameters*)lParam; // приводим params к нужному типу
	SOCKET send_client_socket = params->socket;
	int send_clientID = params->id;
	
	// сообщение от сервера клиенту
	string response = "[Server] Your ID: " + to_string(send_clientID);
	int iResult = send(send_client_socket, response.c_str(), response.length(), 0); // отправка сообщение клиенту

	if (iResult == SOCKET_ERROR) // обработчик ошибки 
	{
		printf("Client %i Error send", send_clientID);
		PrintLastEroor(WSAGetLastError());
		params->run_flag = false;
		return 1;
	}

	return 0;
}

//LPVOID - указатель который может ссылаться на разные типы данных, тот же самый void*