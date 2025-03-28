#pragma once

#include "Network/Sockets.h"

#define MAXMSGBUFSIZE 16384

class Client {
private:	
	Socket m_socket;
public:
	Client(const char* serverIp, int port);
	~Client() = default;
	void EnterChat();
}; 
