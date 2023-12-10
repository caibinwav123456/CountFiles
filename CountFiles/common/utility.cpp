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
	return 0;
}
DLL string format_segmented_u64(const UInteger64& u64)
{
	string sz=FormatI64(u64),dispsz;
	for(int i=0;i<(int)sz.size();i+=3)
	{
		string sec=((int)sz.size()>i+3?sz.substr(sz.size()-i-3,3):sz.substr(0,sz.size()-i));
		dispsz=sec+(i==0?"":",")+dispsz;
	}
	return dispsz;
}
