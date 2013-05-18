

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



struct VCard {
	int ver_major;
	int ver_minor;
	std::string full_name;
	std::vector<std::string> names;
	std::map<std::string, std::string> telephones;
	std::vector<std::string> emails;
	int uid;
};


void VCardParse( std::string str, VCard& v );
std::ostream& operator<<( std::ostream& strm, VCard& v );
