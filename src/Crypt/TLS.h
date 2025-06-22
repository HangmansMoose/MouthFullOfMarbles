#pragma once
#include "Network/CommsHeader.h"
#include "Network/Sockets.h"
#include <assert.h>
#include <stdio.h>
#include <cstdlib>


namespace TLS {
	int tls_connect(SOCKET* sock, const char* hostname, uint16_t port);
    bool init_schannel(Socket* sock);
};

