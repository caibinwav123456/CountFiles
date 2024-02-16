#ifndef _FILE_LIST_LOADER_H_
#define _FILE_LIST_LOADER_H_
#include "struct.h"
uint fllapi get_subdir_cnt(HDNODE dir);
uint fllapi get_subfile_cnt(HDNODE dir);
HDNODE fllapi get_subdir(HDNODE dir,int idx);
HFNODE fllapi get_subfile(HDNODE dir,int idx);
uint fllapi get_errdir_cnt(HDNODE dir);
uint fllapi get_errfile_cnt(HDNODE dir);
HENODE fllapi get_errdir(HDNODE dir,int idx);
HENODE fllapi get_errfile(HDNODE dir,int idx);
bool fllapi get_dir_expand_state(HDNODE dir);
class fllapi FileListLoader
{
public:
	FileListLoader();
	~FileListLoader();

	int Load(const char* listfile,const char* errfile=NULL);
	void Unload();

	int ExpandNode(HDNODE node,bool expand,bool release=false);
	int GetNodeInfo(HFNODE node,file_node_info* pinfo);
	int GetNodeInfo(HDNODE node,file_node_info* pinfo);
	int GetNodeErrInfo(HENODE node,err_node_info* peinfo);

	HDNODE GetRootNode();

private:
	void* priv;
};
#endif
