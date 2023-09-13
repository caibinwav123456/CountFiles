#ifndef _FILE_LIST_LOADER_INTERNAL_H_
#define _FILE_LIST_LOADER_INTERNAL_H_
#include <assert.h>
#include <vector>
#define FLAG_NODE_EXPANDED    1
#define FLAG_NODE_DELETED     2
#define FLAG_NODE_LOADERR     4
#define node_state_exp(node) (!!((node)->flags&FLAG_NODE_EXPANDED))
#define node_expand(node) ((node)->flags|=FLAG_NODE_EXPANDED)
#define node_foldup(node) ((node)->flags&=~FLAG_NODE_EXPANDED)
#define node_state_del(node) (!!((node)->flags&FLAG_NODE_DELETED))
#define node_delete(node) ((node)->flags|=FLAG_NODE_DELETED)
#define node_restore(node) ((node)->flags&=~FLAG_NODE_DELETED)
struct fnode
{
	dword flags;
	UInteger64 fl_start;
	UInteger64 fl_end;
	void* handle;
	fnode():flags(0),handle(NULL){}
};
struct dir_contents;
struct err_dir_node;
struct dir_node:public fnode
{
	dir_contents* contents;
	err_dir_node* enode;
	dir_node():contents(NULL),enode(NULL){}
};
//Here base fnode member fl_start & fl_end represents non-standard records.
//In err_dir_node::fnode, they only represent the subdir name.
//In err_contents members err_dirs & err_files, efnode::fnode represent file/dir name.
//Members efnode::efl_start & efnode::efl_end represent offsets of an error description.
struct efnode:public fnode
{
	UInteger64 efl_start;
	UInteger64 efl_end;
	efnode()
	{
		flags|=FLAG_NODE_LOADERR;
	}
};
struct dir_err_contents
{
	vector<efnode> err_dirs;
	vector<efnode> err_files;
};
struct err_dir_node:public fnode
{
	dir_err_contents* err_contents;
	vector<err_dir_node>* subdirs;
	err_dir_node():err_contents(NULL),subdirs(NULL){}
	bool empty()
	{
		return err_contents==NULL&&subdirs==NULL;
	}
};
struct dir_contents
{
	vector<dir_node> dirs;
	vector<fnode> files;
	dir_err_contents* err_contents;
	dir_contents():err_contents(NULL){}
};
struct ctx_flist_loader
{
	string listfile;
	string errfile;
	void* hlf;
	void* hef;
	dir_node* base_node;
	err_dir_node* base_enode;
	ctx_flist_loader()
	{
		base_node=NULL;
		base_enode=NULL;
		hlf=NULL;
		hef=NULL;
	}
};
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
	for(tmpptr=ptr;tmpptr>=start;tmpptr--)
	{
		if(tmpptr>start&&*(tmpptr-1)&0x80)
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
static inline bool match_tag(const char* str,const byte*& ptr,uint& len)
{
	const byte* buf=ptr;
	uint slen=strlen(str);
	if(len<slen)
		return false;
	ptr+=slen,len-=slen;
	if(memcmp(buf,str,slen)!=0)
		return false;
	return true;
}
#define pass_str(str) \
	if(!match_tag(str,ptr,len)) \
		return ERR_BAD_CONFIG_FORMAT; \
	buf=ptr
int FindLine(UInteger64& off,const UInteger64& end,void* hlf);
int RevFindLine(UInteger64& off,void* hlf);
void free_cache_item(void* item);
int load_file_list(ctx_flist_loader* ctx,LRUCache* cache);
void unload_file_list(ctx_flist_loader* ctx,LRUCache* cache);
int load_error_list(err_dir_node* enode,const UInteger64& off,const UInteger64& end,void* hef);
int retrieve_node_info(fnode* node,file_node_info* pinfo,void* hlf,LRUCache* cache);
int retrieve_enode_info(efnode* node,err_node_info* pinfo,void* hef,LRUCache* cache);
int get_err_dir_node_name(fnode* enode,string& name,void* hef,dword* type=NULL);
int expand_dir(dir_node* node,bool expand,bool release,void* hlf,void* hef,LRUCache* cache);
#endif
