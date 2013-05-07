#include "stdafx.h"
#include "Blue.h"


void initializeWinsock( )
{
	// Initialize winsock
	WSADATA data;
	WORD version = MAKEWORD( 2 , 2 );
	LONG result = WSAStartup( version, &data );

    if( result != ERROR_SUCCESS ) {
		throw Win32Exception( "WSAStartup" );
	}
}


std::vector<Device> getDeviceList()
{
	// Get all nearby Bluetooth devices
	WSAQUERYSET qs;
	HANDLE hlookup;
	LONG result;
	std::vector<Device> devices;
	DWORD flags = LUP_CONTAINERS | LUP_RETURN_ALL | LUP_FLUSHCACHE;

	memset( &qs, 0, sizeof(WSAQUERYSET) );
	qs.dwSize = sizeof(WSAQUERYSET);
	qs.dwNameSpace = NS_BTH;
	result = WSALookupServiceBegin( 
		&qs,
		flags,
		&hlookup );

	while( true )
	{
		char buff[4096];
		WSAQUERYSET* pqs = (WSAQUERYSET*)buff;
		DWORD size = sizeof(buff);
		memset( pqs, 0, size );
		Device device;

		result = WSALookupServiceNext( 
			hlookup, 
			flags,
			&size, 
			pqs );

		if( result != 0 ) {
			if( GetLastError() == WSA_E_NO_MORE ) {
			    break;
		    }
			else {
				throw Win32Exception( "WSALookupServiceNext" );
			}
		}

		device.name = CW2A(pqs->lpszServiceInstanceName);
		device.guid = pqs->lpServiceClassId ? *pqs->lpServiceClassId : GUID_NULL;
		device.address = *(SOCKADDR_BTH*)pqs->lpcsaBuffer->RemoteAddr.lpSockaddr;

		devices.push_back( device );

		__asm nop;
	}

	WSALookupServiceEnd( hlookup );

	return devices;
}


int userSelectDevice( std::vector<Device>& devices )
{
	// Presents all the devices to the user, and let the user select the device he
	// wants to connect to.
	int index = 0;
	int selection;
	for( Device& device: devices ) {
		std::cout << 
			index++ << ". " <<
			"name: " << std::left << std::setw(18) << device.name <<
			"address: " << device.address << "  " <<
			"class id: " << device.guid <<  "  " <<
			std::endl;
	}

	std::cout << std::endl << "Your selection: ";
	std::cin >> selection;

	return selection;
}


std::vector<Service> getServiceList( Device const& device )
{
	// For a given device, fetch all the services offered by this device
	WSAQUERYSET qs;
	wchar_t context[60];
	DWORD maxlen = _countof(context) - 1;
	LONG result;
	std::vector<Service> services;

    result = WSAAddressToString( (SOCKADDR*)&device.address, sizeof(SOCKADDR_BTH), NULL, context, &maxlen );

	memset( &qs, 0, sizeof(qs) );
	qs.dwSize = sizeof(qs);
	qs.dwNameSpace = NS_BTH;
	qs.dwNumberOfCsAddrs = 0;
	qs.lpszContext = context;
	qs.lpServiceClassId = const_cast<LPGUID>( &PublicBrowseGroupServiceClass_UUID );
	DWORD flags = LUP_FLUSHCACHE | LUP_RETURN_ALL;
	HANDLE hlookup;

	result = WSALookupServiceBegin( &qs, flags, &hlookup );
	if( result != 0 ) {
		throw Win32Exception( "WSALookupServiceBegin" );
	}

	while( true )
	{
		char buff[4096];
		WSAQUERYSET* pqs = (WSAQUERYSET*)buff;
		DWORD size = sizeof(buff);
		memset( buff, 0, size );
		Service svc;

		result = WSALookupServiceNext( hlookup, flags, &size, pqs );
		if( result != 0 && GetLastError() == WSA_E_NO_MORE ) {
			break;
		}
		else if( result != 0 ) {
			throw Win32Exception( "WSALookupServiceNext for service" );
		}

		svc.name = CW2A( pqs->lpszServiceInstanceName );
		svc.description = CW2A( pqs->lpszComment );
		if( pqs->lpcsaBuffer ) {
    		svc.address = *(SOCKADDR_BTH*)pqs->lpcsaBuffer->RemoteAddr.lpSockaddr;
		}
		services.push_back( svc );
	}

	return services;

}


int userSelectService( std::vector<Service>& services )
{
	// Present all services and let the user select the service he wants to connect
	int index = 0;
	int selection;
	for( Service& service: services ) {
		std::cout << 
			index++ << ". " << service.name.c_str() << std::endl <<			
			"- description: " << service.description.c_str() << std::endl <<
			"- address: " << service.address << std::endl <<
			std::endl;
		__asm nop;
	}

	std::cout << std::endl << "Your selection: ";
	std::cin >> selection;

	return selection;
}

SOCKET Connect( Service& svc )
{
	int result;

	SOCKET sock = socket( AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM );
	result = connect( sock, (SOCKADDR*)&svc.address, sizeof(svc.address) );
	if( result != 0 ) {
		throw WsaException("connect");
	}

	return sock;
}

int wmain( int argc, wchar_t* argv[])
{
	std::vector<Device> devices;
	std::vector<Service> services;
	int devIndex, svcIndex;
	SOCKET sock;

	try {
		initializeWinsock();
		devices = getDeviceList();
		devIndex = userSelectDevice( devices );
		services = getServiceList( devices[devIndex] );
		svcIndex = userSelectService( services );
		sock = Connect( services[svcIndex] );
		std::cout << "was able to connect";
	}
	catch( std::exception& ex ) {
		std::cout << ex.what();
	}

	return 0;
}


