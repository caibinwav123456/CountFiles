#include "GenFileList.h"
int GenFileListInternal(const char* path,const intf_cntfile* callback);
DLL int GenFileList(const char* path,const intf_cntfile* callback)
{
	return GenFileListInternal(path,callback);
}
