#ifndef _FILE_LIST_LOADER_INTERNAL_H_
#define _FILE_LIST_LOADER_INTERNAL_H_
#include <assert.h>
static inline uint forward_str(const byte*& ptr,uint& len,uint step)
{
	uint cstep=0;
	for(;cstep<step&&len>0;cstep++)
	{
		if((*ptr)&0x80)
		{
			if(len>1)
			{
				ptr+=2;
				len-=2;
			}
			else
			{
				ptr+=len;
				len=0;
			}
		}
		else
		{
			ptr++;
			len--;
		}
	}
	return cstep;
}
static inline bool rev_str(const byte* ptr,const byte* start,const byte*& revout)
{
	assert(ptr>=start);
	const byte* tmpptr;
	for(tmpptr=ptr;tmpptr>start;tmpptr--)
	{
		if(*(tmpptr-1)&0x80)
		{
			tmpptr--;
		}
		else if(*tmpptr=='\n')
		{
			revout=tmpptr+1;
			return true;
		}
	}
	revout=tmpptr+1;
	return false;
}
static inline bool find_byte(const byte*& ptr,uint& len,byte c)
{
	for(;len>0;forward_str(ptr,len,1))
	{
		if(*ptr==c)
			return true;
	}
	return false;
}
#define advance_ptr(ptr,len,step) \
	if(forward_str(ptr,len,step)<step) \
		return ERR_BUFFER_OVERFLOW
#define pass_byte(c) \
	buf=ptr; \
	advance_ptr(ptr,len,1); \
	if(*buf!=(byte)(c)) \
		return ERR_BAD_CONFIG_FORMAT; \
	buf=ptr
#define pass_space pass_byte(' ')
void free_cache_item(void* item);
int load_file_list(ctx_flist_loader* ctx,LRUCache* cache);
void unload_file_list(ctx_flist_loader* ctx,LRUCache* cache);
int load_error_list(err_dir_node* enode,void* hef);
int retrieve_node_info(fnode* node,file_node_info* pinfo,void* hlf,LRUCache* cache);
int retrieve_enode_info(efnode* node,err_node_info* pinfo,void* hef,LRUCache* cache);
int get_err_dir_node_name(err_dir_node* enode,string& name,void* hef);
int expand_dir(dir_node* node,bool expand,void* hlf,void* hef);
#endif
