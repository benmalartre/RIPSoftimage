
#include "string.h"

namespace SOFTIMAGE
{
wstring escape(const wstring& a)
{
	// abort if string is empty
	size_t str_len = a.length();
	if (str_len == 0) return a;

	wstring b;

	// replace all
	for (size_t i = 0; i < str_len; i++)
	{
		if (a[i] == L'\\')
			b += L"\\\\";
		else
			b += a[i];
	}
	return b;
}

const char* cs2cc(CString a)
{
	const char* s = a.GetAsciiString();
	return s;
}

CString cc2cs(const char* a)
{
	CString s = L"";
	s.PutAsciiString(a);
	return s;
}
string cs2s(CString a)
{
	string s = "";
	for (unsigned int i = 0; i < a.Length(); i++)
	{
		s += (char)a[i];
	}
	return s;
}

CString s2cs(string a)
{
	CString s = L"";
	for (unsigned int i = 0; i < a.length(); i++)
	{
		s += (wchar_t)a[i];
	}
	return s;
}

string ws2s(wstring a)
{
	string s = "";
	for (unsigned int i = 0; i < a.length(); i++)
	{
		s += (char)a[i];
	}
	return s;
}

wstring s2ws(string a)
{
	wstring s = L"";
	for (unsigned int i = 0; i < a.length(); i++)
	{
		s += (wchar_t)a[i];
	}
	return s;
}

CString ws2cs(wstring a)
{
	const wchar_t *pWchar = a.data();
	return CString(pWchar);
}

wstring cs2ws(CString a)
{
	const wchar_t *pWchar = a.GetWideString();
	return wstring(pWchar);
}

// logmessage
void LOG(const CString& a)		{ Application().LogMessage( a ); }
void LOG(CString& a)			{ Application().LogMessage( a ); }
void LOG(wstring& a)			{ Application().LogMessage( ws2cs( a ) ); }
void LOG(const unsigned int a)	{ Application().LogMessage( CVALUETOSTRING( (long)a ) ); }
void LOG(const int a)			{ Application().LogMessage( CVALUETOSTRING( (long)a ) ); }
void LOG(const long a)			{ Application().LogMessage( CVALUETOSTRING( a ) ); }
void LOG(const float a)			{ Application().LogMessage( CVALUETOSTRING( a ) ); }
void LOG(const double a)		{ Application().LogMessage( CVALUETOSTRING( a ) ); }
