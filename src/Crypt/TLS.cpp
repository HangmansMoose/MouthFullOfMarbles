#include "TLS.h"

int TLS::tls_connect(SOCKET* sock, const char* hostname, uint16_t port )
{
	return 0;
}


bool TLS::init_schannel(Socket* sock)
{
	SCHANNEL_CRED* cred = nullptr;
	cred->dwVersion = SCHANNEL_CRED_VERSION,
	cred->grbitEnabledProtocols = SP_PROT_TLS1_2,
	cred->dwFlags = SCH_USE_STRONG_CRYPTO
				  | SCH_CRED_AUTO_CRED_VALIDATION
				  | SCH_CRED_NO_DEFAULT_CREDS;

	PCredHandle credHandle = sock->GetCredHandle();
    
    if(AcquireCredentialsHandleW(NULL, (LPWSTR)UNISP_NAME_A, SECPKG_CRED_OUTBOUND, NULL
                                  , cred, NULL, NULL, credHandle, NULL ) != SEC_E_OK )
    {
        return false;
    }

    return true;
}