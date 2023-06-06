#include "GenFileList.h"
int GenFileListInternal(const char* path,const intf_cntfile* callback,char dsym);
DLL int GenFileList(const char* path,const intf_cntfile* callback,char dsym)
{
	return GenFileListInternal(path,callback,dsym);
}
