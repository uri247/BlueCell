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

	__asm nop;
	
}


int wmain( int argc, wchar_t* argv[] )
{
	initApplication( );
	discover();

}

