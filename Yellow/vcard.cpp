#include "stdafx.h"
#include "Yellow.h"

#define pass __asm nop

const std::string test_vcard(
	"BEGIN:VCARD\n"
    "VERSION:2.1\n"
    "FN;CHARSET=UTF-8:Avner Aharoni\n"
    "N;CHARSET=UTF-8:Aharoni;Avner;;;\n"
    "TEL;TYPE=WORK:(425) 705-4443\n"
    "TEL;TYPE=HOME:(206) 523-8597\n"
    "EMAIL;CHARSET=UTF-8;TYPE=INTERNET:avnera@microsoft.com\n"
    "EMAIL;CHARSET=UTF-8;TYPE=INTERNET:avner.aharoni.7@facebook.com\n"
    "ORG;CHARSET=UTF-8:Microsoft\n"
    "UID:2494\n"
    "END:VCARD\n" );


std::ostream& operator<<( std::ostream& strm, VCard& v )
{
	std::string email = ( v.emails.size() > 0 ) ? v.emails[0] : "";
	strm << std::left << std::setfill(' ') <<
		std::setw(30) << v.full_name <<
		std::setw(20) << v.telephones["CELL"] <<
		std::setw(20) << v.telephones["WORK"] <<
		std::setw(20) << v.telephones["HOME"] <<
		std::setw(30) << email;

	return strm;
}


void VCardParse( std::string str, VCard& v )
{
	std::regex rx_tel( "TEL;TYPE=([^:]+):([^\r\n]+)\r?" );
	std::regex rx_fn( "FN;CHARSET=UTF-8:([^\r\n]+)\r?" );
	std::regex rx_email( "EMAIL;CHARSET=UTF-8;TYPE=INTERNET:([^\r\n]+)\r?" );

	std::istringstream ss( str );
	std::string line;

	while( ss ) {
	    std::getline( ss, line );
		std::smatch m;

		if( std::regex_match( line, m, rx_tel ) ) {
			std::string tp = std::string( m[1].first, m[1].second );
			std::string ph = std::string( m[2].first, m[2].second );
			v.telephones[tp] = ph;
			pass;
		}
		else if( std::regex_match( line, m, rx_fn ) ) {
			std::string full_name( m[1].first, m[1].second );
			v.full_name = std::string( full_name );
			pass;
		}
		else if( std::regex_match( line, m, rx_email ) ) {
			std::string email = std::string( m[1].first, m[1].second );
			v.emails.push_back( email );
		}
		pass;
	}

}


int test( int argc, wchar_t* argv[] )
{
	VCard v;
	int ret;

	try {
		VCardParse( test_vcard, v );
		std::cout << v << std::endl;
		ret = 0;
	}
	catch( std::exception& ex ) {
		std::cout << ex.what();
		ret = -1;
	}

	return ret;

}

