#include "FileListLoader.h"
#include "LRUCache.h"
#include <string>
#define FLIST_CACHE_SIZE 500
#define GET_DATA(data) FileListData* data=(FileListData*)priv
void free_cache_item(void* item);
int load_file_list(ctx_flist_loader* ctx,LRUCache* cache);
void unload_file_list(ctx_flist_loader* ctx,LRUCache* cache);
int retrieve_node_info(fnode* node,file_node_info* pinfo,void* hlf,LRUCache* cache);
int expand_dir(dir_node* node,bool expand,void* hlf);
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
		return ERR_GENERIC;
	data->ctx.listfile=listfile;
	data->ctx.errfile=(errfile==NULL?"":errfile);
	if(data->ctx.listfile.empty())
	{
		data->ctx.errfile.clear();
		return ERR_GENERIC;
	}
	return load_file_list(&data->ctx,&data->cache);
}
void FileListLoader::Unload()
{
	GET_DATA(data);
	if(!(data->ctx.listfile.empty()&&data->ctx.errfile.empty()))
		unload_file_list(&data->ctx,&data->cache);
}
int FileListLoader::ExpandNode(dir_node* node,bool expand)
{
	GET_DATA(data);
	return expand_dir(node,expand,data->ctx.hlf);
}
int FileListLoader::GetNodeInfo(fnode* node,file_node_info* pinfo)
{
	GET_DATA(data);
	return retrieve_node_info(node,pinfo,data->ctx.hlf,&data->cache);
}
dir_node* FileListLoader::GetRootNode()
{
	GET_DATA(data);
	return data->ctx.base_node;
}
