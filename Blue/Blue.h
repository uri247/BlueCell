#include <exception>
#include <iostream>

class GleWin32
{
    // This is a traits class to be used by 'CodeException' exception class. The error code
	// is retreived from GetLastError
public:
	typedef DWORD CodeType;
	static DWORD gle() { return GetLastError(); }
};

class GleWsa
{
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


typedef CodeException<GleWin32> Win32Exception;
typedef CodeException<GleWsa> WsaException;


inline
std::ostream& operator<<( std::ostream& strm, GUID& guid )
{
	// Use this operator to dump GUID's to an io stream.
	wchar_t buff[60];
	StringFromGUID2( guid, buff, _countof(buff) );
	strm << CW2A(buff);
	return strm;
}

inline
std::ostream& operator<<( std::ostream& strm, SOCKADDR_BTH& addr )
{
	// Use this method to dump sock address (currently, only bluetooth) to an io stream
	wchar_t buff[60];
	DWORD maxlen = _countof(buff) - 1;
	WSAAddressToString( (SOCKADDR*)&addr, sizeof(addr), NULL, buff, &maxlen );
	strm << CW2A(buff);
	return strm;
}


struct Device
{
	// A structure to hold data aboout a bluetooth device
	std::string name;
	GUID guid;
	SOCKADDR_BTH address;
};

struct Service
{
	// A structure to hold data about a specific service (and own address) of a device
	std::string name;
	std::string description;
	SOCKADDR_BTH address;

};


