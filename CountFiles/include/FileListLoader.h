#ifndef _FILE_LIST_LOADER_H_
#define _FILE_LIST_LOADER_H_
#include "struct.h"
class DLL FileListLoader
{
public:
	FileListLoader();
	~FileListLoader();

	int Load(const char* listfile,const char* errfile=NULL);
	void Unload();

	int ExpandNode(dir_node* node,bool expand);
	int GetNodeInfo(fnode* node,file_node_info* pinfo);
	int GetNodeErrInfo(efnode* node,err_node_info* peinfo);

	dir_node* GetRootNode();

private:
	void* priv;
};
#endif
