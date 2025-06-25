#pragma once

#include "commonVariables.h"

void PrintClientInfo(SOCKET clientSocket);
DWORD WINAPI ClientHandler(LPVOID lParam);
DWORD WINAPI RecvThread(LPVOID lParam);
DWORD WINAPI SendThread(LPVOID lParam);