#include <exception>


class GleWin32
{
	// GleWin32 - GetLastError for Win32 errors
    // This is a traits class to be used by 'CodeException' exception class. The error code
    // is retreived from GetLastError
public:
    typedef DWORD CodeType;
    static DWORD gle() { return GetLastError(); }
};

class GleWsa
{
	// GleWSA - GetLastError for WinSock
    // A traits class for CodeException. The error code is int and is retrieved from WSAGetLastError
public:
    typedef int CodeType;
    static DWORD gle() { return WSAGetLastError(); }
};


template< typename Tr >
class CodeException : public std::exception
{
    // This template holds an error code, retrived from the Trait class 'gle' method during
    // construction. It derives from standard c++ std::exception class, and override the 'what'
    // virtual method
private:
    typename Tr::CodeType m_error;
    char const* m_where;
    char m_msg[120];

public:
    CodeException( char const* where ) :m_error(Tr::gle()), m_where(where) {
        sprintf_s( m_msg, "Win32 error: %d in %s", m_error, where );
    }
    CodeException( typename Tr::CodeType err, char const* where ) :m_error(err), m_where(where) {
        sprintf_s( m_msg, "Win32 error: %d in %s", m_error, where );
    }

    typename Tr::CodeType error() const { return m_error; }
    char const* where() const { return m_where; }
    virtual const char* what() const { return m_msg; }
};


inline
std::ostream& operator<<( std::ostream& strm, BTUINT8 (&address)[6] )
{
	strm << '(' << std::hex << std::setw(2);
	for( int i=0; i<6; ++i ) {
		strm << std::hex << std::setw(2) << std::setfill('0') << std::right << (int)address[i];
		if( i < 5 )
			strm << ":";
		else
			strm << ")";
	}

	return strm;
}


