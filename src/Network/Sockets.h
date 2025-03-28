#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string>
// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")

#define BUFFLENGTH 16384

class Socket{
public:
	Socket();
	Socket(SOCKET establishedSocket);
	~Socket();
	void GetErrorMessage(DWORD dw_error);
	bool WSAInit();
	void FindListenSocket(std::string port);
	void CreateSocket();
	Socket AcceptConnection();
	void ConnectToRemoteHost(const char* targetIpAddr, int port);
	void SendMessage(const char* message);
	int Receive(char* msgBuffer, int msgBufferSize);
	SOCKET GetSocket();

private:
	SOCKET m_socket = INVALID_SOCKET;
	sockaddr_in m_addr = { 0 };
	WSADATA m_wsaData = { 0 };
	bool wsaInitialised = false;
};
