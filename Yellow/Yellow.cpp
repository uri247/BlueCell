#include "stdafx.h"
#include "utils.h"
#include "Yellow.h"

const int maxlen_devices = 20;
const int discover_timeout = 30;


std::mutex g_cvmx;
std::condition_variable g_cv;
std::vector<Device> g_devices;


void initApplication( )
{
	BTBOOL connected;
	BTINT32 result;
	
	connected = Btsdk_IsServerConnected();
	if( connected != BTSDK_TRUE ) {
		result = Btsdk_Init();
		if( result != BTSDK_OK ) {
			throw std::exception( "failed Bdsdk_Init()" );
		}
	}

	std::cout << "connect to BlueSoleil successful" << std::endl;

	result = Btsdk_StartBluetooth( );
	if( result != BTSDK_OK ) {
		throw std::exception( "Error on StartBluetooth" );
	}

	std::cout << "bluetooth started" << std::endl;

}


void inquiryResultInd( BTDEVHDL handle )
{
	Device device;
	device.handle = handle;
	g_devices.push_back( device );
}

void inquiryCompleteInd( void )
{
	std::unique_lock<std::mutex> lock(g_cvmx);
	g_cv.notify_one();
}


void discover( )
{
	BTINT32 status;

	// register callbacks
	BtSdkCallbackStru callbacks[] = {
		{ BTSDK_INQUIRY_RESULT_IND, inquiryResultInd },
		{ BTSDK_INQUIRY_COMPLETE_IND, inquiryCompleteInd },
	};

	for( auto& cb : callbacks ) {
		status = Btsdk_RegisterCallback4ThirdParty( &cb );
		if( status != BTSDK_OK ) {
			throw std::exception( "failed to register" );
		}
	}

	std::unique_lock<std::mutex> lock(g_cvmx);
	status = Btsdk_StartDeviceDiscovery( 0, 20, 30 );
	g_cv.wait(lock);

	for( Device& device : g_devices ) {
		char name[40];
		BTUINT16 maxlen = sizeof(name);

		status = Btsdk_GetRemoteDeviceName( device.handle, (BTUINT8*)name, &maxlen );
		if( status != BTSDK_OK ) {
			throw std::exception( "failed to get name" );
		}
		device.name = std::string( name );

		status = Btsdk_GetRemoteDeviceAddress( device.handle, device.address );
		if( status != BTSDK_OK ) {
			throw std::exception( "failed getting remote address" );
		}

		status = Btsdk_GetRemoteDeviceClass( device.handle, &device.devclass );
		if( status != BTSDK_OK ) {
			std::exception( "failed getting class" );
		}
	}

	__asm nop;
	
}

int getUserDeviceSelection( )
{
	int index = 0;
	for( Device& device : g_devices ) {
		std::cout << index++ << ". " <<
			std::left << std::setw(30) << device.name << 
			device.address << 
			"0x" << std::hex << std::right<< std::setw(12) << std::setfill(' ') << device.devclass <<
			std::endl;
	}

	int choice;
	std::cout << std::endl << "Your choice: ";
	std::cin >> choice;

	return choice;
}


void getServices( Device& device )
{
	BTINT32 status;
	BTSVCHDL svchdls[30];
	BTUINT32 maxlen = _countof(svchdls);

	status = Btsdk_BrowseRemoteServices( device.handle, svchdls, &maxlen );
	if( status != BTSDK_OK ) {
		throw std::exception( "error browse remote services" );
	}


	device.services.clear();
	device.services.resize(maxlen);

	for( BTUINT32 i=0; i<maxlen; ++i ) {
		Service& svc = device.services[i];
		svc.handle = svchdls[i];

		BtSdkRemoteServiceAttrStru attrs;
		memset( &attrs, 0, sizeof(attrs) );
		attrs.mask = BTSDK_RSAM_SERVICENAME | BTSDK_RSAM_EXTATTRIBUTES;;
		status = Btsdk_GetRemoteServiceAttributes( svc.handle, &attrs );
		svc.name = (char*)attrs.svc_name;
		svc.svcclass = attrs.svc_class;
	}
}


int getUserServiceSelection( Device& device )
{
	int index = 0;
	for( Service& svc : device.services ) {
		std::cout << index++ << ". " <<
			std::left << std::setw(30) << std::setfill(' ') << svc.name << 
			"0x" << std::hex << std::right<< std::setw(8) << std::setfill('0') << svc.svcclass <<
			std::endl;
	}

	int choice;
	std::cout << std::endl << "Your choice: ";
	std::cin >> choice;

	return choice;
}

void connect( Device& dev, Service& svc )
{
	BTINT32 status;
	BTCONNHDL conhdl;

	//status = Btsdk_Connect( svc.handle, 0, &conhdl );

}


int wmain( int argc, wchar_t* argv[] )
{
	int devIndex, svcIndex;

	initApplication( );
	discover();
	devIndex = getUserDeviceSelection( );
	getServices( g_devices[devIndex] );
	svcIndex = getUserServiceSelection( g_devices[devIndex] );
	connect( g_devices[devIndex], g_devices[devIndex].services[svcIndex] );
}

