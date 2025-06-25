#include "commonVariables.h"

int clientID = 1;
std::map<SOCKET, int> clientIds;
std::mutex mtx_client;