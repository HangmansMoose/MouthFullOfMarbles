//#include <winbase.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <cstdlib>
#include <string>
#include "Sockets.h"

// NOTE: Remember to keep this only to socket specific actions. Everything else like data management and threading
// can be done by the class that needs to do it.

/*
* GetErrorMessage Takes in the DWORD (windows 32-bit unsigned integer) value of the error returned
* and a char container to place the formatted message in. The dw_error value can be obtained by calling
* WSAGetLastError() and casting it to DWORD (the function returns an int because of course it does)
*/

Socket::Socket()
{
	if (!wsaInitialised)
	{
		if (!WSAInit())
		{
			printf("Could not initialise WSA.\n");
			exit(EXIT_FAILURE);
		}
		printf("WSA init complete.\n");
		wsaInitialised = true;
	}

}

Socket::Socket(SOCKET establishedSocket) : m_socket(establishedSocket) {}

Socket::~Socket()
{
	closesocket(m_socket);
	WSACleanup();
}

void Socket::GetErrorMessage(DWORD dw_error)
{
	LPSTR *pnc_msg = NULL;
	DWORD dw_flags;

	dw_flags = FORMAT_MESSAGE_ALLOCATE_BUFFER
			  |FORMAT_MESSAGE_FROM_SYSTEM
			  |FORMAT_MESSAGE_IGNORE_INSERTS;

	FormatMessageA(dw_flags, NULL, dw_error, LANG_SYSTEM_DEFAULT, *pnc_msg, 0, NULL);

	fprintf(stderr, "Failed with code %ld\n", dw_error);
	fprintf(stderr, "%s\n", *pnc_msg);
}

bool Socket::WSAInit()
{
	int result = WSAStartup(MAKEWORD(2,2), &m_wsaData);

	if (result != 0) {
        printf("WSAStartup failed with error: %d\n", result);
        return false;
    }
	
	if (LOBYTE(m_wsaData.wVersion) < 2 ||
       HIBYTE(m_wsaData.wVersion) < 2)
    {
		fprintf(stderr,"Version 2.2 of Winsock is not available.\n");
		WSACleanup();

		return false;
    }
	
	wsaInitialised = true;
	return true;
}

void Socket::CreateSocket()
{
	DWORD dw_error;

	if(!WSAInit())
	{
		printf("WSAInit failed\n");
		exit(EXIT_FAILURE);
	}

	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (m_socket == INVALID_SOCKET)
	{
		printf("Socket create failed.\n");
		dw_error = (DWORD)WSAGetLastError();
		GetErrorMessage(dw_error);
		exit(EXIT_FAILURE);
	}

}


// FindListenSocket(const char* c_port) creates the required address structures in order to query
// the available addressess. It receives the response in the form of the ps_listenSockets struct
// being filled out by the getaddrinfo function. Once it has an address to use it will then attempt
// to create a socket context, then bind to that context, then start listening on that socket.

void Socket::FindListenSocket(std::string port)
{
    struct addrinfo *ps_listenSockets;
    DWORD            dw_error;
    struct addrinfo   s_hints;
    int               result;

	ZeroMemory(&s_hints, sizeof(s_hints));

	s_hints.ai_family = AF_INET;
	s_hints.ai_socktype = SOCK_STREAM; // STREAM for TCP DGRAM for UDP
	s_hints.ai_protocol = IPPROTO_TCP;
	s_hints.ai_flags = AI_PASSIVE;

	result = getaddrinfo(NULL, port.c_str(), &s_hints, &ps_listenSockets);

	if( result != 0 )
	{
		printf("getaddrinfo failed.\n");
		dw_error = WSAGetLastError();
		GetErrorMessage(dw_error);
		exit(EXIT_FAILURE);
	}

	m_socket = socket(ps_listenSockets->ai_family, ps_listenSockets->ai_socktype, 
					  ps_listenSockets-> ai_protocol);

	if (m_socket == INVALID_SOCKET) 
	{
        printf("Failed to create socket\n");
		dw_error = WSAGetLastError();
		GetErrorMessage(dw_error);

        freeaddrinfo(ps_listenSockets);
		exit(EXIT_FAILURE); 
    }

	result = bind(m_socket, ps_listenSockets->ai_addr, (int)ps_listenSockets->ai_addrlen);
	
	if (result == SOCKET_ERROR) 
	{
        printf("bind failed\n");
		dw_error = WSAGetLastError();
		GetErrorMessage(dw_error);
        freeaddrinfo(ps_listenSockets);
        closesocket(m_socket);
        exit(EXIT_FAILURE);
    }

	// No longer need this data so it can be freed
    freeaddrinfo(ps_listenSockets);

    result = listen(m_socket, SOMAXCONN);
    if (result == SOCKET_ERROR) 
	{
        printf("listen failed\n");
		dw_error = WSAGetLastError();
		GetErrorMessage(dw_error);
        closesocket(m_socket);
        exit(EXIT_FAILURE);
    }

}

Socket Socket::AcceptConnection() 
{
	sockaddr_in newClientAddr;
    int clientSize = sizeof(newClientAddr);
    SOCKET newSocket = accept(m_socket, (sockaddr*)&newClientAddr, &clientSize);
    if (newSocket == INVALID_SOCKET) {
        printf("Accept failed\n");
	    DWORD dw_error = WSAGetLastError();
	    GetErrorMessage(dw_error);
    }

	Socket newClientSock;
	newClientSock.m_socket = newSocket;
	newClientSock.m_addr = newClientAddr;
    return newClientSock; 
}

void Socket::ConnectToRemoteHost(const char* serverIP, int port) 
{
	// This needs to be TLS so we need to start by initialiasing SCHANNEL
	SCHANNEL_CRED cred = 
	{
		.dwVersion = SCHANNEL_CRED_VERSION,
		.grbitEnabledProtocols = SP_PROT_TLS1_2,  // allow only TLS v1.2	
		.dwFlags = SCH_USE_STRONG_CRYPTO          // use only strong crypto alogorithms
				 | SCH_CRED_AUTO_CRED_VALIDATION  // automatically validate server certificate
				 | SCH_CRED_NO_DEFAULT_CREDS,     // no client certificate authentication
	};
	
	if (AcquireCredentialsHandle(NULL, (wchar_t*)UNISP_NAME, SECPKG_CRED_OUTBOUND, NULL, &cred, NULL, NULL, &m_credHandle, NULL) != SEC_E_OK)
	{
		printf("Could not acquire credentials handle\n");
		// TODO: if this fails potentially should just give a warning and ask if the user wants to connect unencrypted.
		exit(EXIT_FAILURE);
	}

    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);
    inet_pton(AF_INET, serverIP, &m_addr.sin_addr);

    if (connect(m_socket, (sockaddr*)&m_addr, sizeof(m_addr)) == SOCKET_ERROR) {
        printf("Connection to server failed\n");
		DWORD dw_error = WSAGetLastError();
		GetErrorMessage(dw_error);
        exit(EXIT_FAILURE);
    }
}

bool Socket::Send(SOCKET destinationFd, const char* message) {
    int result = send(destinationFd, message, strlen(message), 0);

	if (result == SOCKET_ERROR)
	{
		DWORD dw_error = WSAGetLastError();
		GetErrorMessage(dw_error);
		printf("Message failed to send to FD %lld\n", destinationFd);
		return false;
	}
	return true;
}

bool Socket::Receive(SOCKET senderFd, char* buffer, int bufferSize) {
    memset(buffer, 0, bufferSize);
	int bytesReceived = recv(m_socket, buffer, bufferSize, 0);

	if (bytesReceived == SOCKET_ERROR || bytesReceived == 0)
	{
	   if (bytesReceived == 0) // Connection closed
	   {
			fprintf(stderr,"socket %lld hung up\n",senderFd);
			return false;
	   }
	   else
	   {
			printf("Error attempting to receive from FD %lld\n", senderFd);
			DWORD dw_error = WSAGetLastError();
			GetErrorMessage(dw_error);
			return false;
	   }
	
	}

	return true;
}

SOCKET Socket::GetSocket() const
{
    return m_socket;
}

char* Socket::GetAddress() const
{
	PSTR ipBuffer[16];  
	inet_ntop(AF_INET,&m_addr, *ipBuffer, sizeof(ipBuffer));
	return *ipBuffer;
}

CredHandle* Socket::GetCredHandle()
{
	return &m_credHandle;
}

