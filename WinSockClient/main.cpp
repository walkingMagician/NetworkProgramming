#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <iostream>
#include <string>
#include <FormatLastError.h>


using namespace std;

#pragma comment(lib, "FormatLastError.lib")
#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_PORT "27015"
#define DEFAULT_BUFFER_LENGTH 1500





void main()
{
	setlocale(LC_ALL, "Russian");
	cout << "WinSock Client" << endl;
	// 1) инициализация winsock
	WSADATA wsaData;
	INT iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult)
	{
		cout << "WSAStartuo() failed witc code " << iResult << endl;
		return;
	}

	// 2) создаём ClientSocket
	addrinfo* result = NULL;
	addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET; // INET - TCP/IPv4
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// 3) определяем IP-адресс Сервера:
	iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
	if (iResult)
	{
		cout << "getaddressinfo() failed witch code " << iResult << endl;
		return;
	}

	SOCKET connect_socket = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);
	if (connect_socket == INVALID_SOCKET)
	{
		PrintLastEroor(WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	// 4) подключаемся к серверу
	iResult = connect(connect_socket, result->ai_addr, result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		PrintLastEroor(WSAGetLastError());
		closesocket(connect_socket);
		freeaddrinfo(result);
		WSACleanup();
	}


	// 5) отправляем и получение данных с сервера 
	
	//// Создаем буфер для ввода пользовательских данных
	//CHAR sendBuffer[DEFAULT_BUFFER_LENGTH] = { 0 };
	//
	//// Получаем ввод от пользователя
	//printf("Введите сообщение для отправки: ");
	//fgets(sendBuffer, DEFAULT_BUFFER_LENGTH, stdin);
	//
	//// Убираем символ новой строки, который fgets добавляет в буфер
	//size_t len = strlen(sendBuffer);
	//if (len > 0 && sendBuffer[len - 1] == '\n') {
	//	sendBuffer[len - 1] = '\0';
	//}

	CONST CHAR sendBuffer[DEFAULT_BUFFER_LENGTH] = { "hello server"};
	CHAR recvbuffer[DEFAULT_BUFFER_LENGTH] = {};
	iResult = send(connect_socket, sendBuffer, sizeof(sendBuffer), 0);
	if (iResult == SOCKET_ERROR)
	{
		PrintLastEroor(WSAGetLastError());
		closesocket(connect_socket);
		freeaddrinfo(result);
		WSACleanup();
		return;
	}
	do
	{
		iResult = recv(connect_socket, recvbuffer, DEFAULT_BUFFER_LENGTH, 0);
		if (iResult > 0) cout << "Receved bytes " << iResult << ", Message: " << recvbuffer << endl;
		else if (iResult == 0) cout << "Connection closing" << endl;
		else PrintLastEroor(WSAGetLastError());
	} while (iResult > 0);

	// 6) закрываем соединение
	iResult = shutdown(connect_socket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		cout << "Error shutdown";
		PrintLastEroor(WSAGetLastError());
	}
	// 7) освобождаем ресурсы winSock
	closesocket(connect_socket);
	freeaddrinfo(result);
	WSACleanup();
}

