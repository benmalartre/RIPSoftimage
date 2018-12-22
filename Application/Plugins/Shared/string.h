
#ifndef FLAG_STRING
#define FLAG_STRING

#include "xsi_common.h"

namespace SOFTIMAGE {

wstring escape(const wstring& a);
const char* cs2cc(CString a);
CString cc2cs(const char* a);
string cs2s(CString a);
CString s2cs(string a);
string ws2s(wstring a);
wstring s2ws(string a);
CString ws2cs(wstring a);
wstring cs2ws(CString a);

// logmessage
#define CVALUETOSTRING(a) CValue(a).GetAsText()
void LOG(const CString& a);
void LOG(CString& a);
void LOG(wstring& a);
void LOG(const unsigned int a);
void LOG(const int a);
void LOG(const long a);
void LOG(const float a);
void LOG(const double a);

} // namespace SOFTIMAGE

#endif
