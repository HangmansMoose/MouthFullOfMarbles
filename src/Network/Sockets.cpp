#include "Sockets.h"
#include "winbase.h"
#include "winsock2.h"

// NOTE: Remember to keep this only to socket specific actions. Everything else like data management and threading
// can be done by the class that needs to do it.

/*
* GetErrorMessage Takes in the DWORD (windows 32-bit unsigned integer) value of the error returned
* and a char container to place the formatted message in. The dw_error value can be obtained by calling
* WSAGetLastError() and casting it to DWORD (the function returns an int because of course it does)
*/

Socket::~Socket()
{
	closesocket(m_socket);
	WSACleanup();
}

void Socket::GetErrorMessage(DWORD dw_error, char** pnc_msg)
{
	DWORD dw_flags;

	dw_flags = FORMAT_MESSAGE_ALLOCATE_BUFFER
			  |FORMAT_MESSAGE_FROM_SYSTEM
			  |FORMAT_MESSAGE_IGNORE_INSERTS;

	FormatMessage(dw_flags, NULL, dw_error, LANG_SYSTEM_DEFAULT, (LPTSTR)pnc_msg, 0, NULL);

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
	return true;
}

/*
 * StartListener takes in an ip address and a port as a string (not integer!), retrieves the local address info
 * (required to fill the expected addrinfo struct) then creates the socket with the ip:port combo and binds.
 * If false is returned at all from this function then the listener was unsuccessful and the program should be
 * exitted gracefully. Check the error messages.
 */
bool Socket::StartListener( std::string sPort)
{
	struct addrinfo* addrInfoResult = NULL;
	if(!WSAInit() 
		|| (!ServerGetAddressInfo(sPort, addrInfoResult)) 
	    || (!CreateSocket(addrInfoResult)) 
		|| (!BindSocket(addrInfoResult))
		|| (!ListenOnSocket())
		)
	{
		return false;
	}

	freeaddrinfo(addrInfoResult);
	// At this point m_socket is bound and listening on the requested interface
	return true;

}

bool Socket::CreateSocket(addrinfo* addrInfo)
{
	m_socket = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);

	if (m_socket == INVALID_SOCKET)
	{
		char** p_messageStore = NULL;
		DWORD dw_error = (DWORD)WSAGetLastError();
		GetErrorMessage(dw_error, p_messageStore);
		return false;
	}
	return true;
}

bool Socket::BindSocket(addrinfo* addrInfo)
{	
	int result = bind(m_socket, addrInfo->ai_addr, (int)addrInfo->ai_addrlen);

	if (result == SOCKET_ERROR)
	{
		char** p_messageStore = NULL;
		DWORD dw_error = (DWORD)WSAGetLastError();
		GetErrorMessage(dw_error, p_messageStore);
		return false;
	}
	return true;
}

bool Socket::ListenOnSocket()
{
	int result = listen(m_socket, SOMAXCONN);

	if (result == SOCKET_ERROR)
	{
		char** p_messageStore = NULL;
		DWORD dw_error = (DWORD)WSAGetLastError();
		GetErrorMessage(dw_error, p_messageStore);
		return false;
	}
	return true;
}

bool Socket::ConnectToHost(std::string p_targetIp, std::string p_targetPort)
{
	struct addrinfo* addrInfoResult = NULL;
	if ( (!WSAInit()) || (!CreateSocket(addrInfoResult)) )
	{
		printf("Could not connect to server.\n");
		freeaddrinfo(addrInfoResult);
		return false;
	}

	freeaddrinfo(addrInfoResult);
	// At this point m_socket is bound and listening on the requested interface
	return true;
}

bool Socket::AcceptIncomingConnection()
{

	int result = accept(m_socket, NULL, NULL);

	if (result == SOCKET_ERROR)
	{
		char** p_messageStore = NULL;
		DWORD dw_error = (DWORD)WSAGetLastError();
		GetErrorMessage(dw_error, p_messageStore);
		return false;
	}
	return true;
}

bool Socket::Send(char sendBuffer[], int sendBufferLength)
{
	int result = send(m_socket, sendBuffer, sendBufferLength, 0);

	if (result == SOCKET_ERROR)
	{
		char** p_messageStore = NULL;
		DWORD dw_error = (DWORD)WSAGetLastError();
		GetErrorMessage(dw_error, p_messageStore);
		return false;
	}
	return true;
}

void Socket::Receive(char receiveBuffer[], int bufferLength)
{
	int result = 0;
	do {

        result = recv(m_socket, receiveBuffer, bufferLength, 0);
        if (result > 0) {
            printf("Bytes received: %d\n", result);

        // Echo the buffer back to the sender
        }
        else if (result == 0)
            printf("Connection closing...\n");
        else  {
			char** p_messageStore = NULL;
			DWORD dw_error = (DWORD)WSAGetLastError();
			GetErrorMessage(dw_error, p_messageStore);
        }

    } while (result > 0);
}

void Socket::Flush()
{

}

bool Socket::Disconnect()
{
	int result = shutdown(m_socket, SD_SEND);
    if (result == SOCKET_ERROR) {
		char** p_messageStore = NULL;
		DWORD dw_error = (DWORD)WSAGetLastError();
		GetErrorMessage(dw_error, p_messageStore);
        return false;
    }
	return true;
}

// For servers 
bool Socket::ServerGetAddressInfo( std::string sPort, struct addrinfo* addrInfoResult)
{
	int result;
	char* nc_error;
	struct addrinfo hints;
    
	ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

	result = getaddrinfo(NULL, sPort.c_str(), &hints, &addrInfoResult);

	if (result != 0)
	{
		DWORD dw_error = (DWORD)WSAGetLastError();
		GetErrorMessage(dw_error, &nc_error);
		return false;
	}

	return true;


}

// For Clients
bool Socket::ClientGetAddressInfo( std::string ipAddr, std::string sPort, struct addrinfo* addrInfoResult)
{
	int result;
	char* nc_error;
	struct addrinfo hints;
    
	ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

	result = getaddrinfo(ipAddr.c_str(), sPort.c_str(), &hints, &addrInfoResult);

	if (result != 0)
	{
		DWORD dw_error = (DWORD)WSAGetLastError();
		GetErrorMessage(dw_error, &nc_error);
		return false;
	}

	return true;


}
