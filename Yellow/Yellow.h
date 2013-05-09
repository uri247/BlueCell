

struct Service
{
	BTSVCHDL handle;
	std::string name;
	BTUINT16 svcclass;

};

struct Device
{
	BTDEVHDL handle;
	std::string name;
	BTUINT8 address[6];
	BTUINT32 devclass;
	std::vector<Service> services;
};


