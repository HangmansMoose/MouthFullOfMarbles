#include "Sockets.h"
#include "winbase.h"
#include "winsock2.h"
#include <cstdlib>

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
	LPTSTR *pnc_msg = NULL;
	DWORD dw_flags;

	dw_flags = FORMAT_MESSAGE_ALLOCATE_BUFFER
			  |FORMAT_MESSAGE_FROM_SYSTEM
			  |FORMAT_MESSAGE_IGNORE_INSERTS;

	FormatMessage(dw_flags, NULL, dw_error, LANG_SYSTEM_DEFAULT, *pnc_msg, 0, NULL);

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
    sockaddr_in clientAddr;
    int clientSize = sizeof(clientAddr);
    SOCKET clientSocket = accept(m_socket, (sockaddr*)&clientAddr, &clientSize);
    if (clientSocket == INVALID_SOCKET) {
        printf("Accept failed\n");
	    DWORD dw_error = WSAGetLastError();
	    GetErrorMessage(dw_error);
    }

	Socket newSocket(clientSocket);
    return newSocket; 
}

void Socket::ConnectToRemoteHost(const char* serverIP, int port) {
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

void Socket::SendMessage(const char* message) {
    send(m_socket, message, strlen(message), 0);
}

int Socket::Receive(char* buffer, int bufferSize) {
    memset(buffer, 0, bufferSize);
    return recv(m_socket, buffer, bufferSize, 0);
}

SOCKET Socket::GetSocket()  
{
    return m_socket;
}


///*
// * StartListener takes in an ip address and a port as a string (not integer!), retrieves the local address info
// * (required to fill the expected addrinfo struct) then creates the socket with the ip:port combo and binds.
// * If false is returned at all from this function then the listener was unsuccessful and the program should be
// * exitted gracefully. Check the error messages.
// */
//bool Socket::ListenerSetup( std::string sPort, char buffer[], int bufferLength)
//{
//	if (!wsaInitialised)
//	{
//		if (!WSAInit())
//		{
//			printf("WSA init failed, exiting.\n");
//		}
//	}
//	struct addrinfo* addrInfoResult = NULL;
//	if(!WSAInit() 
//		|| (!GetListenAddrInfo(sPort, addrInfoResult)) 
//	    || (!CreateSocket(addrInfoResult)) 
//		|| (!BindSocket(addrInfoResult))
//		)
//	{ return false; }
//	
//	ListenOnSocket();
//	
//	printf("Now Listening....\n ");
//
//	while(true) 
//	{
//		AcceptIncomingConnection();
//		ReceiveData(buffer, bufferLength);
//
//	}
//
//	freeaddrinfo(addrInfoResult);
//	// At this point m_listenSock is bound and listening on the requested interface
//	return true;
//
//}
//
//bool Socket::CreateSocket(addrinfo* addrInfo)
//{
//	if (!wsaInitialised)
//	{
//		if (!WSAInit())
//		{
//			printf("WSA init failed, exiting.\n");
//		}
//	}
//
//	int result = setsockopt(m_listenSock, SOL_SOCKET, SO_REUSEADDR, (char *)true, sizeof(true));
//
//	if (result == SOCKET_ERROR)
//	{
//		DWORD dw_error = (DWORD)WSAGetLastError();
//		GetErrorMessage(dw_error);
//		return false;
//	}
//
//	m_listenSock = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
//
//	if (m_listenSock == INVALID_SOCKET)
//	{
//		DWORD dw_error = (DWORD)WSAGetLastError();
//		GetErrorMessage(dw_error);
//		return false;
//	}
//	return true;
//}
//
//bool Socket::BindSocket(addrinfo* addrInfoResult)
//{
//	addrinfo* addrIterator = NULL;
//	int result = 0;
//
//	for (addrIterator = addrInfoResult; addrIterator != NULL; addrIterator = addrIterator->ai_next)
//	{
//		m_listenSock = socket(addrIterator->ai_family, addrIterator->ai_socktype, addrIterator->ai_protocol);
//
//		if (m_listenSock == INVALID_SOCKET)
//		{
//			continue;
//		}
//
//		result = setsockopt(m_listenSock, SOL_SOCKET, SO_REUSEADDR, (char *)true, sizeof(true));
//		if (result == SOCKET_ERROR)
//		{
//			DWORD dw_error = (DWORD)WSAGetLastError();
//			GetErrorMessage(dw_error);
//			return false;
//		}
//
//		result = bind(m_listenSock, addrInfoResult->ai_addr, (int)addrInfoResult->ai_addrlen);
//		
//		if (result == SOCKET_ERROR)
//		{
//			continue;
//		}
//		
//		// If the control flow gets to here it we should have successfully bound an address
//		break;
//	}
//
//	return true;
//}
//
//bool Socket::ListenOnSocket()
//{
//	int result = listen(m_listenSock, SOMAXCONN);
//
//	if (result == SOCKET_ERROR)
//	{
//		DWORD dw_error = (DWORD)WSAGetLastError();
//		GetErrorMessage(dw_error);
//		return false;
//	}
//	return true;
//}
//
//bool Socket::ConnectToHost(std::string p_targetIp, std::string p_targetPort)
//{
//	struct addrinfo* addrInfoResult = NULL;
//	if ( (!WSAInit()) || (!CreateSocket(addrInfoResult)) )
//	{
//		printf("Could not connect to server.\n");
//		freeaddrinfo(addrInfoResult);
//		return false;
//	}
//
//	freeaddrinfo(addrInfoResult);
//	// At this point m_listenSock is bound and listening on the requested interface
//	return true;
//}
//
//bool Socket::AcceptIncomingConnection()
//{
//
//	int result = accept(m_listenSock, NULL, NULL);
//
//	if (result == SOCKET_ERROR)
//	{
//		DWORD dw_error = (DWORD)WSAGetLastError();
//		GetErrorMessage(dw_error);
//		return false;
//	}
//	return true;
//}
//
//bool Socket::Send(char sendBuffer[], int sendBufferLength)
//{
//	int result = send(m_listenSock, sendBuffer, sendBufferLength, 0);
//
//	if (result == SOCKET_ERROR)
//	{
//		DWORD dw_error = (DWORD)WSAGetLastError();
//		GetErrorMessage(dw_error);
//		return false;
//	}
//	return true;
//}
//
//void Socket::ReceiveData(char receiveBuffer[], int bufferLength)
//{
//	int result = 0;
//	do {
//
//        result = recv(m_listenSock, receiveBuffer, bufferLength, 0);
//        if (result > 0) {
//            printf("Bytes received: %d\n", result);
//
//        // Echo the buffer to the console
//		printf("MESSAGE: %s\n", receiveBuffer);
//        }
//        else if (result == 0)
//            printf("Connection closing...\n");
//        else  {
//			DWORD dw_error = (DWORD)WSAGetLastError();
//			GetErrorMessage(dw_error);
//        }
//
//    } while (result > 0);
//}
//
//bool Socket::Disconnect()
//{
//	int result = shutdown(m_listenSock, SD_SEND);
//    if (result == SOCKET_ERROR) {
//		DWORD dw_error = (DWORD)WSAGetLastError();
//		GetErrorMessage(dw_error);
//        return false;
//    }
//	return true;
//}
//
//// For servers 
//bool Socket::GetListenAddrInfo( std::string sPort, struct addrinfo* addrInfoResult)
//{
//	int result;
//	struct addrinfo hints;
//	struct addrinfo* iter;
//    
//	ZeroMemory(&hints, sizeof(hints));
//    hints.ai_family = AF_INET;
//    hints.ai_socktype = SOCK_STREAM;
//    hints.ai_flags = AI_PASSIVE;
//
//	result = getaddrinfo(NULL, sPort.c_str(), &hints, &addrInfoResult);
//
//	if (result != 0)
//	{
//		DWORD dw_error = (DWORD)WSAGetLastError();
//		GetErrorMessage(dw_error);
//		return false;
//	}
//	return true;
//
//}
//
//// For Clients
//bool Socket::GetTargetAddrInfo( std::string ipAddr, std::string sPort, struct addrinfo* addrInfoResult)
//{
//	int result;
//	struct addrinfo hints;
//    
//	ZeroMemory(&hints, sizeof(hints));
//    hints.ai_family = AF_INET;
//    hints.ai_socktype = SOCK_STREAM;
//    hints.ai_protocol = IPPROTO_TCP;
//
//	result = getaddrinfo(ipAddr.c_str(), sPort.c_str(), &hints, &addrInfoResult);
//
//	if (result != 0)
//	{
//		DWORD dw_error = (DWORD)WSAGetLastError();
//		GetErrorMessage(dw_error);
//		return false;
//	}
//
//	return true;
//
//
//}
