#pragma once

#include "commonVariables.h"

void PrintClientInfo(SOCKET clientSocket); // вывод информации о подключеённым клиенте
DWORD WINAPI ClientHandler(LPVOID lParam); // обработка соединение с клиентом
DWORD WINAPI RecvThread(LPVOID lParam); // приём данных от клиента
DWORD WINAPI SendThread(LPVOID lParam); // отправка даных клиенту