#define DLL_IMPORT
#include "struct.h"
#include "LRUCache.h"
#include <assert.h>
#define safe_fclose(hfile) \
	if(VALID(hfile)) \
	{ \
		sys_fclose(hfile); \
		hfile=NULL; \
	}
template<class T>
struct ListItem:public LRUCacheItem
{
	T t;
};
void free_cache_item(LRUCacheItem* item)
{
	delete item;
}
void free_flist_file_data(ctx_flist_loader* ctx)
{
	safe_fclose(ctx->hlf);
	safe_fclose(ctx->hef);
	ctx->listfile.clear();
	ctx->errfile.clear();
}
void free_dir_node(dir_node* dir)
{
	if(dir==NULL)
		return;
	if(dir->contents==NULL)
		return;
	for(int i=0;i<(int)dir->contents->dirs.size();i++)
	{
		free_dir_node(&dir->contents->dirs[i]);
	}
	delete dir->contents;
	dir->contents=NULL;
}
static inline uint forward_str(const byte*& ptr,uint& len,uint step)
{
	uint cstep=0;
	for(;len>0&&cstep<step;cstep++)
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
		if(tmpptr-1>=start)
		{
			if(*(tmpptr-1)&0x80)
			{
				tmpptr--;
				continue;
			}
			else if(*tmpptr=='\n')
			{
				revout=tmpptr+1;
				return true;
			}
		}
		else
			break;
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
int RevFindLine(UInteger64& off,void* hlf)
{
	if(off<=UInteger64(1))
		return ERR_GENERIC;
	UInteger64 tmpoff=off-UInteger64(1);
	UInteger64 start;
	byte buf[64];
	int ret=0;
	while(tmpoff>UInteger64(0))
	{
		start=(tmpoff>UInteger64(64)?tmpoff-UInteger64(64):0);
		if(0!=(ret=sys_fseek(hlf,start.low,&start.high,SEEK_BEGIN)))
			return ret;
		uint len=(tmpoff-start).low;
		if(0!=(ret=sys_fread(hlf,buf,len)))
			return ret;
		const byte* ptr=NULL;
		if(rev_str(buf+len-1,buf,ptr))
		{
			tmpoff=start+UInteger64(ptr-buf);
			break;
		}
		tmpoff=start+UInteger64(start==UInteger64(0)?0:ptr-buf);
	}
	off=tmpoff;
	return 0;
}
static inline int match_tagged_size(const char* tag,UInteger64& size,const byte*& ptr,uint& len,byte* tmpbuf,uint tmpbuflen)
{
	const byte* buf=ptr;
	uint taglen=strlen(tag);
	UInteger64 tmpsize;
	advance_ptr(ptr,len,taglen);
	if(memcmp(buf,tag,taglen)!=0)
		return ERR_BAD_CONFIG_FORMAT;

	buf=ptr;
	if(!find_byte(ptr,len,' '))
		return ERR_BAD_CONFIG_FORMAT;
	uint tmplen=ptr-buf;
	if(tmplen>=tmpbuflen)
		return ERR_BUFFER_OVERFLOW;
	memcpy(tmpbuf,buf,tmplen);
	tmpbuf[tmplen]=0;
	if(!I64FromDec(string((char*)tmpbuf),tmpsize))
		return ERR_BAD_CONFIG_FORMAT;
	size=tmpsize;
	return 0;
}
int parse_rec(const byte* buf,uint len,file_node_info* pinfo,UInteger64& recsize)
{
	int ret=0;
	const char* tag_recsize=TAG_DRSIZE;
	const char* tag_size=TAG_SIZE;
	const byte* ptr=buf;
	UInteger64 tmpsize(0);
	uint tmplen;
	advance_ptr(ptr,len,2);
	byte tmpbuf[100];
	if(memcmp(buf,TAG_TYPE_DIR,2)==0)
	{
		buf=ptr;
		if(0!=(ret=match_tagged_size(tag_recsize,tmpsize,ptr,len,tmpbuf,100)))
			return ret;
		pinfo->type=FILE_TYPE_DIR;
		pass_space;
	}
	else if(memcmp(buf,TAG_TYPE_FILE,2)==0)
		pinfo->type=FILE_TYPE_NORMAL;
	else
		return ERR_GENERIC;

	recsize=tmpsize;

	pass_byte('\"');
	if(!find_byte(ptr,len,'\"'))
		return ERR_BAD_CONFIG_FORMAT;
	{
		tmplen=ptr-buf;
		char* namebuf=new char[tmplen+1];
		memcpy(namebuf,buf,tmplen);
		namebuf[tmplen]=0;
		pinfo->name=namebuf;
		delete[] namebuf;
	}
	pass_byte('\"');
	pass_space;

	if(0!=(ret=match_tagged_size(tag_size,tmpsize,ptr,len,tmpbuf,100)))
		return ret;
	pinfo->size=tmpsize;
	pass_space;

	if(0!=(ret=pinfo->mod_time.FromString(ptr,len)))
		return ret;
	pass_byte('\n');

	if(len!=0)
		return ERR_BAD_CONFIG_FORMAT;

	return 0;
}
int ReadRecContent(UInteger64 start,uint len,UInteger64& prevoff,void* hlf,file_node_info* pinfo=NULL)
{
	int ret=0;
	byte* buf=new byte[len];
	file_node_info dummyinfo;
	file_node_info& info=(pinfo==NULL?dummyinfo:*pinfo);
	UInteger64 recsize;
	if(0!=(ret=sys_fseek(hlf,start.low,&start.high,SEEK_BEGIN)))
		goto end;
	if(0!=(ret=sys_fread(hlf,buf,len)))
		goto end;
	if(0!=(ret=parse_rec(buf,len,&info,recsize)))
		goto end;
	prevoff=start-recsize;
end:
	delete[] buf;
	return ret;
}
int RevReadNode(void* hlf,UInteger64& off,file_node_info* pinfo=NULL)
{
	int ret=0;
	UInteger64 tmpoff=off,prevoff;
	if(0!=(ret=RevFindLine(tmpoff,hlf)))
		return ret;
	if(0!=(ret=ReadRecContent(tmpoff,(off-tmpoff).low,prevoff,hlf,pinfo)))
		return ret;
	off=prevoff;
	return 0;
}
int retrieve_node_info(fnode* node,file_node_info* pinfo,void* hlf,LRUCache* cache)
{
	if(node==NULL)
		return ERR_GENERIC;
	int ret=0;
	ListItem<file_node_info>* item=(ListItem<file_node_info>*)cache->get(&node->handle);
	if(item==NULL)
	{
		item=new ListItem<file_node_info>;
		cache->put(item,&node->handle);
		UInteger64 tmpoff=node->fl_end;
		if(0!=(ret=RevReadNode(hlf,tmpoff,&item->t)))
			return ret;
	}
	*pinfo=item->t;
	return 0;
}
int expand_dir(dir_node* node,bool expand,void* hlf)
{
	int ret=0;
	if(node==NULL)
		return ERR_GENERIC;
	if(expand&&node_get_expand_state(node)
		||((!expand)&&(!node_get_expand_state(node))))
		return 0;
	if(expand)
	{
		node_expand(node);
		if(node->contents!=NULL)
			return 0;
		dir_contents* contents=node->contents=new dir_contents;
		dir_node dirnode;
		fnode filenode;
		file_node_info info;
		UInteger64 off=node->fl_end;
		if(0!=(ret=RevFindLine(off,hlf)))
			return ret;
		while(off>node->fl_start)
		{
			UInteger64 endoff=off;
			RevReadNode(hlf,off,&info);
			switch(info.type)
			{
			case FILE_TYPE_DIR:
				dirnode.fl_start=off;
				dirnode.fl_end=endoff;
				contents->dirs.push_back(dirnode);
				break;
			case FILE_TYPE_NORMAL:
				filenode.fl_start=off;
				filenode.fl_end=endoff;
				contents->files.push_back(filenode);
				break;
			}
		}
		if(off<node->fl_start)
		{
			delete node->contents;
			node->contents=NULL;
			node_foldup(node);
			return ERR_GENERIC;
		}
		reverse(node->contents->dirs.begin(),node->contents->dirs.end());
		reverse(node->contents->files.begin(),node->contents->files.end());
	}
	else
	{
		node_foldup(node);
	}
	return 0;
}
int build_base_tree(dir_node*& base,void* hlf,const UInteger64& endrec)
{
	int ret=0;
	base=new dir_node;
	base->fl_start=base->fl_end=endrec;
	file_node_info info;
	if(0!=(ret=RevReadNode(hlf,base->fl_start,&info)))
		return ret;
	if(info.type!=FILE_TYPE_DIR)
		return ERR_GENERIC;
	return 0;
}
void unload_file_list(ctx_flist_loader* ctx,LRUCache* cache);
int load_file_list(ctx_flist_loader* ctx,LRUCache* cache)
{
	ctx->hlf=sys_fopen((char*)ctx->listfile.c_str(),FILE_OPEN_EXISTING|FILE_READ);
	if(!VALID(ctx->hlf))
	{
		free_flist_file_data(ctx);
		return ERR_OPEN_FILE_FAILED;
	}
	if(!ctx->errfile.empty())
	{
		ctx->hef=sys_fopen((char*)ctx->errfile.c_str(),FILE_OPEN_EXISTING|FILE_READ);
		if(!VALID(ctx->hef))
		{
			free_flist_file_data(ctx);
			return ERR_OPEN_FILE_FAILED;
		}
	}
	int ret=0;
	UInteger64 off;
	if(0!=(ret=sys_get_file_size(ctx->hlf,&off.low,&off.high)))
	{
		free_flist_file_data(ctx);
		return ret;
	}
	if(0!=(ret=build_base_tree(ctx->base_node,ctx->hlf,off)))
	{
		unload_file_list(ctx,cache);
		return ret;
	}
	return 0;
}
void unload_file_list(ctx_flist_loader* ctx,LRUCache* cache)
{
	free_flist_file_data(ctx);
	free_dir_node(ctx->base_node);
	delete ctx->base_node;
	ctx->base_node=NULL;
	cache->clear();
}