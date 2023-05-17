#include "GenFileList.h"
int GenFileListInternal(char* path,intf_cntfile* callback,char dsym);
DLL int GenFileList(char* path,intf_cntfile* callback,char dsym)
{
	return GenFileListInternal(path,callback,dsym);
}
