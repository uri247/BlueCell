#include "stdafx.h"
#include "..\Blue\Blue.h"

#include <initguid.h>
DEFINE_GUID( UUID_URI_SVC, 0x2bc52a95, 0x7563, 0x4fda, 0x80, 0xbd, 0x64, 0x78, 0x21, 0xd5, 0x42, 0x5a );


void initializeWinsock( )
{
	// Initialize winsock or throw
	WSADATA data;
	WORD version = MAKEWORD( 2 , 2 );
	LONG result = WSAStartup( version, &data );

    if( result != ERROR_SUCCESS ) {
		throw WsaException( "WSAStartup" );
	}
}



int wmain( int argc, wchar_t argv[] )
{
	//
	// Open a socket, publish a service, listen, wait and accept. This exit
	//

	initializeWinsock();
	SOCKET server;
	SOCKADDR_BTH addr;
	int result;

	server = socket( AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM );
	if( server == SOCKET_ERROR ) {
		throw WsaException("socket");
	}

	addr.addressFamily = AF_BTH;
	addr.btAddr = 0;
	addr.port = BT_PORT_ANY;

	result = bind( server, (sockaddr*)&addr, sizeof(addr) );
	if( result != 0 ) {
		throw WsaException( "bind" );
	}

	listen( server, 1 );

	// get the address given to us by winsock
	int namelen = sizeof(addr);
	result = getsockname(server, (sockaddr*)&addr, &namelen );

	// advertize
	CSADDR_INFO info;
	info.iProtocol = BTHPROTO_RFCOMM;
	info.iSocketType = SOCK_STREAM;
	info.LocalAddr.lpSockaddr = (SOCKADDR*)&addr;
	info.LocalAddr.iSockaddrLength = sizeof(addr);
	info.RemoteAddr.lpSockaddr = (SOCKADDR*)&addr;
	info.RemoteAddr.iSockaddrLength = sizeof(addr);

	WSAQUERYSET qs = {0};
	qs.dwSize = sizeof(qs);
	qs.dwNameSpace = NS_BTH;
	qs.lpszServiceInstanceName = L"Uri's own Bluetooth service";
	qs.lpszComment = L"A test service";
	qs.dwNumberOfCsAddrs = 1;
	qs.lpServiceClassId = const_cast<UUID*>(&UUID_URI_SVC);
	qs.lpcsaBuffer = &info;

	result = WSASetService( &qs, RNRSERVICE_REGISTER, 0 );
	if( result != 0 ) {
		throw WsaException("WSASetService");
	}

	SOCKET client;
	//SOCKADDR_BTH addr = { 0 };
	memset( &addr, 0, sizeof(addr) );
	int maxlen = sizeof( addr );
	char buff[1024] = { 0 };
	int bufflen = sizeof(buff);

	client = accept( server, (SOCKADDR*)&client, &bufflen );
	if( client == SOCKET_ERROR ) {
		throw WsaException( "accept" );
	}

	std::cout << "accepted a connection from client ...";

	return 0;
}

