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
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define BUFFLENGTH 16384

class Socket{
public:
	// This creates the one predefined instance of this class. This is the singleton design pattern
	Socket();
	~Socket();
	bool StartListener( std::string sPort);
	bool ConnectToHost(std::string p_targetIp, std::string n_targetPort);
	bool AcceptIncomingConnection();
	bool Send(char sendBuffer[], int sendBufferLength);
	void Receive(char receiveBuffer[], int bufferLength);
	void Flush();
	bool Disconnect();
private:
	bool WSAInit();
	void GetErrorMessage(DWORD dw_error, char** pnc_msg);
	bool ServerGetAddressInfo(std::string sPort, struct addrinfo* addrInfoResult);
	bool ClientGetAddressInfo(std::string ipAddr, std::string sPort, struct addrinfo* addrInfoResult);
	bool CreateSocket(addrinfo* addrInfo);
	bool BindSocket(addrinfo* addrInfo);
	bool ListenOnSocket();
	int m_funcResult = 0;
	static bool wsaInitialised;
	WSADATA m_wsaData;
	SOCKET m_socket = INVALID_SOCKET;
	struct addrinfo* m_pResult = NULL;
	struct addrinfo* m_pPtr = NULL;
	struct addrinfo m_hints;
	char m_bRecvBuffer[BUFFLENGTH];
	



};
