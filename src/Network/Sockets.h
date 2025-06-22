#pragma once

#include "Network/CommsHeader.h"
#include <string>
#include <unordered_map>

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
	bool Send(SOCKET destinationFd, const char* message);
	bool Receive(SOCKET senderFd, char* msgBuffer, int msgBufferSize);
	SOCKET GetSocket() const;
	char* GetAddress() const;
	CredHandle* GetCredHandle();
  struct HttpRequest {
    std::string requestMethod;
    std::string uri;
    std::unordered_map<std::string, std::string> headers;
  };


private:
	SOCKET m_socket = INVALID_SOCKET;
	sockaddr_in m_addr = { 0 };
	WSADATA m_wsaData = { 0 };
	WSAPOLLFD* m_wsaPollFd = { 0 };
	bool wsaInitialised = false;
	CredHandle m_credHandle;
	CtxtHandle m_contextHandle;
	SecPkgContext_StreamSizes m_streamSizes;
	int received;    // byte count in incoming buffer (ciphertext)
	int used;        // byte count used from incoming buffer to decrypt current packet
	int available;   // byte count available for decrypted bytes
	char* decrypted; // points to incoming buffer where data is decrypted inplace
	char incoming[TLS_MAX_PACKET_SIZE];
};
