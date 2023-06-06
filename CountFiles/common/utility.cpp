#include "utility.h"
#include <string.h>
#if !defined(BUILD_ON_WINDOWS)
#define CmpNoCase strncasecmp
#else
#define CmpNoCase strnicmp
#endif
DLL int compare_pathname(const string& a,const string& b)
{
	int ret=CmpNoCase(a.c_str(),b.c_str(),min(a.size(),b.size()));
	if(ret!=0)
		return ret;
	if(a.size()!=b.size())
		return a.size()<b.size()?-1:1;
	if(a!=b)
		return a<b?-1:1;
	return 0;
}