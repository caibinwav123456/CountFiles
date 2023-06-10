#include "FileListLoader.h"
#include "LRUCache.h"
#include <string>
#include "FileListLoaderInternal.h"
#define FLIST_CACHE_SIZE 500
#define GET_DATA(data) FileListData* data=(FileListData*)priv
uint fllapi get_subdir_cnt(HDNODE dir)
{
	dir_node* _dir=(dir_node*)dir;
	if(_dir==NULL||_dir->contents==NULL)
		return 0;
	return _dir->contents->dirs.size();
}
uint fllapi get_subfile_cnt(HDNODE dir)
{
	dir_node* _dir=(dir_node*)dir;
	if(_dir==NULL||_dir->contents==NULL)
		return 0;
	return _dir->contents->files.size();
}
HDNODE fllapi get_subdir(HDNODE dir,int idx)
{
	dir_node* _dir=(dir_node*)dir;
	if(_dir==NULL||_dir->contents==NULL)
		return NULL;
	if(idx<0||idx>=(int)_dir->contents->dirs.size())
		return NULL;
	return (HDNODE)(&_dir->contents->dirs[idx]);
}
HFNODE fllapi get_subfile(HDNODE dir,int idx)
{
	dir_node* _dir=(dir_node*)dir;
	if(_dir==NULL||_dir->contents==NULL)
		return NULL;
	if(idx<0||idx>=(int)_dir->contents->files.size())
		return NULL;
	return (HFNODE)(&_dir->contents->files[idx]);
}
uint fllapi get_errdir_cnt(HDNODE dir)
{
	dir_node* _dir=(dir_node*)dir;
	if(_dir==NULL||_dir->contents==NULL||_dir->contents->err_contents==NULL)
		return 0;
	return _dir->contents->err_contents->err_dirs.size();
}
uint fllapi get_errfile_cnt(HDNODE dir)
{
	dir_node* _dir=(dir_node*)dir;
	if(_dir==NULL||_dir->contents==NULL||_dir->contents->err_contents==NULL)
		return 0;
	return _dir->contents->err_contents->err_files.size();
}
HENODE fllapi get_errdir(HDNODE dir,int idx)
{
	dir_node* _dir=(dir_node*)dir;
	if(_dir==NULL||_dir->contents==NULL||_dir->contents->err_contents==NULL)
		return NULL;
	if(idx<0||idx>=(int)_dir->contents->err_contents->err_dirs.size())
		return NULL;
	return (HENODE)(&_dir->contents->err_contents->err_dirs[idx]);
}
HENODE fllapi get_errfile(HDNODE dir,int idx)
{
	dir_node* _dir=(dir_node*)dir;
	if(_dir==NULL||_dir->contents==NULL||_dir->contents->err_contents==NULL)
		return NULL;
	if(idx<0||idx>=(int)_dir->contents->err_contents->err_files.size())
		return NULL;
	return (HENODE)(&_dir->contents->err_contents->err_files[idx]);
}
bool fllapi get_dir_expand_state(HDNODE dir)
{
	dir_node* _dir=(dir_node*)dir;
	return node_state_exp(_dir);
}
struct FileListData
{
	ctx_flist_loader ctx;
	LRUCache cache;
	FileListData(uint cachesize):cache(cachesize,free_cache_item){}
};
FileListLoader::FileListLoader()
{
	priv=new FileListData(FLIST_CACHE_SIZE);
}
FileListLoader::~FileListLoader()
{
	GET_DATA(data);
	Unload();
	delete data;
}
int FileListLoader::Load(const char* listfile,const char* errfile)
{
	GET_DATA(data);
	if(!(data->ctx.listfile.empty()&&data->ctx.errfile.empty()))
		return ERR_INVALID_CALL;
	data->ctx.listfile=listfile;
	data->ctx.errfile=(errfile==NULL?"":errfile);
	if(data->ctx.listfile.empty())
	{
		data->ctx.errfile.clear();
		return ERR_INVALID_CALL;
	}
	return load_file_list(&data->ctx,&data->cache);
}
void FileListLoader::Unload()
{
	GET_DATA(data);
	if(!(data->ctx.listfile.empty()&&data->ctx.errfile.empty()))
		unload_file_list(&data->ctx,&data->cache);
}
int FileListLoader::ExpandNode(HDNODE node,bool expand)
{
	GET_DATA(data);
	dir_node* _node=(dir_node*)node;
	return expand_dir(_node,expand,data->ctx.hlf,data->ctx.hef,&data->cache);
}
int FileListLoader::GetNodeInfo(HFNODE node,file_node_info* pinfo)
{
	GET_DATA(data);
	fnode* _node=(fnode*)node;
	return retrieve_node_info(_node,pinfo,data->ctx.hlf,&data->cache);
}
int FileListLoader::GetNodeInfo(HDNODE node,file_node_info* pinfo)
{
	GET_DATA(data);
	dir_node* _node=(dir_node*)node;
	return retrieve_node_info(_node,pinfo,data->ctx.hlf,&data->cache);
}
int FileListLoader::GetNodeErrInfo(HENODE node,err_node_info* peinfo)
{
	GET_DATA(data);
	efnode* _node=(efnode*)node;
	return retrieve_enode_info(_node,peinfo,data->ctx.hef,&data->cache);
}
HDNODE FileListLoader::GetRootNode()
{
	GET_DATA(data);
	return (HDNODE)(data->ctx.base_node);
}
