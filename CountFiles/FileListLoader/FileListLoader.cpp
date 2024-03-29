#define DLL_IMPORT
#include "struct.h"
#include "LRUCache.h"
#include "FileListLoaderInternal.h"
#include "utility.h"
#define safe_fclose(hfile) \
	if(VALID(hfile)) \
	{ \
		sys_fclose(hfile); \
		hfile=NULL; \
	}
void free_cache_item(void* item)
{
	delete (node_info_base*)item;
}
static void free_flist_file_data(ctx_flist_loader* ctx)
{
	safe_fclose(ctx->hlf);
	safe_fclose(ctx->hef);
	ctx->listfile.clear();
	ctx->errfile.clear();
}
static void free_dir_node(dir_node* dir)
{
	if(dir==NULL)
		return;
	if(dir->contents==NULL)
		return;
	for(int i=0;i<(int)dir->contents->dirs.size();i++)
		free_dir_node(&dir->contents->dirs[i]);
	delete dir->contents;
	dir->contents=NULL;
}
static void free_enode(err_dir_node* edir)
{
	if(edir==NULL)
		return;
	if(edir->err_contents!=NULL)
	{
		delete edir->err_contents;
		edir->err_contents=NULL;
	}
	if(edir->subdirs==NULL)
		return;
	for(int i=0;i<(int)edir->subdirs->size();i++)
		free_enode(&(edir->subdirs->at(i)));
	delete edir->subdirs;
	edir->subdirs=NULL;
}
int RevFindLine(UInteger64& off,void* hlf)
{
	if(off<=UInteger64(1))
		return ERR_CORRUPTED_FILE;
	UInteger64 tmpoff=off-UInteger64(1);
	UInteger64 start;
	byte buf[64];
	int ret=0;
	while(tmpoff>UInteger64(0))
	{
		start=(tmpoff>UInteger64(64)?tmpoff-UInteger64(64):0);
		return_ret(ret,0,sys_fseek(hlf,start.low,&start.high,SEEK_BEGIN));
		uint len=(tmpoff-start).low;
		return_ret(ret,0,sys_fread(hlf,buf,len));
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
	pass_str(tag);
	UInteger64 tmpsize;
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
static int parse_rec(const byte* buf,uint len,file_node_info* pinfo,UInteger64& recsize)
{
	int ret=0;
	const char* tag_recsize=TAG_DRSIZE;
	const char* tag_size=TAG_SIZE;
	const byte* ptr=buf;
	UInteger64 tmpsize(0);
	advance_ptr(ptr,len,2);
	byte tmpbuf[100];
	if(memcmp(buf,TAG_TYPE_DIR,2)==0)
	{
		buf=ptr;
		return_ret(ret,0,match_tagged_size(tag_recsize,tmpsize,ptr,len,tmpbuf,100));
		pinfo->type=FILE_TYPE_DIR;
		pass_space;
	}
	else if(memcmp(buf,TAG_TYPE_FILE,2)==0)
		pinfo->type=FILE_TYPE_NORMAL;
	else
		return ERR_INVALID_TAG;

	recsize=tmpsize;

	pass_byte('\"');
	if(!find_byte(ptr,len,'\"'))
		return ERR_BAD_CONFIG_FORMAT;
	{
		uint tmplen=ptr-buf;
		char* namebuf=new char[tmplen+1];
		memcpy(namebuf,buf,tmplen);
		namebuf[tmplen]=0;
		pinfo->name=namebuf;
		delete[] namebuf;
	}
	pass_byte('\"');
	pass_space;

	return_ret(ret,0,match_tagged_size(tag_size,tmpsize,ptr,len,tmpbuf,100));
	pinfo->size=tmpsize;
	pass_space;

	return_ret(ret,0,pinfo->mod_time.FromString(ptr,len,FORMAT_DATE|FORMAT_TIME|FORMAT_WEEKDAY));
	pass_byte('\n');

	if(len!=0)
		return ERR_BAD_CONFIG_FORMAT;

	return 0;
}
static int ReadRecContent(UInteger64 start,uint len,UInteger64& prevoff,void* hlf,file_node_info* pinfo=NULL)
{
	int ret=0;
	byte* buf=new byte[len];
	file_node_info dummyinfo;
	file_node_info& info=(pinfo==NULL?dummyinfo:*pinfo);
	UInteger64 recsize;
	fail_goto(ret,0,sys_fseek(hlf,start.low,&start.high,SEEK_BEGIN),end);
	fail_goto(ret,0,sys_fread(hlf,buf,len),end);
	fail_goto(ret,0,parse_rec(buf,len,&info,recsize),end);
	prevoff=start-recsize;
end:
	delete[] buf;
	return ret;
}
static int RevReadNode(void* hlf,UInteger64& off,file_node_info* pinfo=NULL)
{
	int ret=0;
	UInteger64 tmpoff=off,prevoff;
	return_ret(ret,0,RevFindLine(tmpoff,hlf));
	return_ret(ret,0,ReadRecContent(tmpoff,(off-tmpoff).low,prevoff,hlf,pinfo));
	off=prevoff;
	return 0;
}
struct path_value_t
{
	string val;
	string* pval;
	path_value_t():pval(NULL){}
};
bool operator<(const path_value_t& a,const path_value_t& b)
{
	const string &stra=a.pval!=NULL?*a.pval:a.val,
		&strb=b.pval!=NULL?*b.pval:b.val;
	return compare_pathname(stra,strb)<0;
}
struct node_iterator:public iterator_base<dir_node>
{
	void* hlf;
	LRUCache* cache;
	node_iterator(vector<dir_node>& flist,void* _hlf,LRUCache* _cache):iterator_base<dir_node>(flist),hlf(_hlf),cache(_cache){}
	path_value_t operator*()
	{
		path_value_t v;
		file_node_info info;
		int ret=retrieve_node_info(&*it,&info,hlf,cache);
		if(ret!=0)
			throw ret;
		v.val=info.name;
		return v;
	}
};
struct err_node_iterator:public iterator_base<err_dir_node>
{
	void* hef;
	err_node_iterator(vector<err_dir_node>& flist,void* _hef):iterator_base<err_dir_node>(flist),hef(_hef){}
	path_value_t operator*()
	{
		path_value_t v;
		int ret=get_err_dir_node_name(&*it,v.val,hef);
		if(ret!=0)
			throw ret;
		return v;
	}
};
int merge_callback(node_iterator it1,err_node_iterator it2,E_MERGE_SIDE side,void* param)
{
	switch(side)
	{
	case eMSLeft:
		break;
	case eMSRight:
		assert(false);
		break;
	case eMSBoth:
		it1.it->enode=&*(it2.it);
		break;
	default:
		assert(false);
	}
	return 0;
}
static int merge_error_list(dir_node* node,void* hlf,void* hef,LRUCache* cache)
{
	if(node->enode->subdirs==NULL)
		return 0;
	node_iterator itdir(node->contents->dirs,hlf,cache);
	err_node_iterator iterr(*node->enode->subdirs,hef);
	try
	{
		merge_ordered_list(itdir,iterr,&merge_callback,(void*)NULL);
	}
	catch(int err)
	{
		return err;
	}
	return 0;
}
int retrieve_node_info(fnode* node,file_node_info* pinfo,void* hlf,LRUCache* cache)
{
	if(node==NULL)
		return ERR_INVALID_PARAM;
	int ret=0;
	node_info_base* item=(node_info_base*)cache->get(&node->handle);
	file_node_info* pfinfo;
	if(item!=NULL)
	{
		pfinfo=dynamic_cast<file_node_info*>(item);
		assert(pfinfo!=NULL);
	}
	else
	{
		item=pfinfo=new file_node_info;
		UInteger64 tmpoff=node->fl_end;
		fail_op(ret,0,RevReadNode(hlf,tmpoff,pfinfo),
		{
			delete pfinfo;
			return ret;
		})
		cache->put(item,&node->handle);
	}
	*pinfo=*pfinfo;
	return 0;
}
int expand_dir(dir_node* node,bool expand,bool release,void* hlf,void* hef,LRUCache* cache)
{
	int ret=0;
	if(node==NULL)
		return ERR_INVALID_PARAM;
	if(!expand)
	{
		node_foldup(node);
		if(release)
			free_dir_node(node);
		return 0;
	}
	if(node_state_exp(node))
		return 0;
	node_expand(node);
	if(node->contents!=NULL)
		return 0;
	dir_contents* contents=node->contents=new dir_contents;
	dir_node dirnode;
	fnode filenode;
	file_node_info info;
	UInteger64 off=node->fl_end;
	fail_goto(ret,0,RevFindLine(off,hlf),fail);
	while(off>node->fl_start)
	{
		UInteger64 endoff=off;
		fail_goto(ret,0,RevReadNode(hlf,off,&info),fail);
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
		ret=ERR_CORRUPTED_FILE;
		goto fail;
	}
	reverse(node->contents->dirs.begin(),node->contents->dirs.end());
	reverse(node->contents->files.begin(),node->contents->files.end());

	if(node->enode!=NULL)
	{
		node->contents->err_contents=node->enode->err_contents;
		fail_goto(ret,0,merge_error_list(node,hlf,hef,cache),fail);
	}

	return 0;

fail:
	delete node->contents;
	node->contents=NULL;
	node_foldup(node);
	return ret;
}
static int build_base_tree(dir_node*& base,void* hlf,const UInteger64& endrec)
{
	int ret=0;
	base=new dir_node;
	base->fl_start=base->fl_end=endrec;
	file_node_info info;
	return_ret(ret,0,RevReadNode(hlf,base->fl_start,&info));
	if(info.type!=FILE_TYPE_DIR)
		return ERR_CORRUPTED_FILE;
	return 0;
}
static int build_err_tree(err_dir_node*& ebase,void* hef,const UInteger64& off,const UInteger64& end,dir_node* base)
{
	int ret=0;
	if(base==NULL)
		return ERR_INVALID_CALL;
	ebase=new err_dir_node;
	return_ret(ret,0,load_error_list(ebase,off,end,hef));
	if(ebase->empty())
		return 0;
	if(ebase->subdirs==NULL||ebase->subdirs->size()!=1)
		return ERR_CORRUPTED_FILE;
	base->enode=&ebase->subdirs->at(0);
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
	fail_op(ret,0,sys_get_file_size(ctx->hlf,&off.low,&off.high),
	{
		free_flist_file_data(ctx);
		return ret;
	})
	fail_goto(ret,0,build_base_tree(ctx->base_node,ctx->hlf,off),fail);
	if(VALID(ctx->hef))
	{
		fail_goto(ret,0,sys_get_file_size(ctx->hef,&off.low,&off.high),fail);
		fail_goto(ret,0,build_err_tree(ctx->base_enode,ctx->hef,UInteger64(0),off,ctx->base_node),fail);
	}
	return 0;
fail:
	unload_file_list(ctx,cache);
	return ret;
}
void unload_file_list(ctx_flist_loader* ctx,LRUCache* cache)
{
	free_flist_file_data(ctx);
	if(ctx->base_node!=NULL)
	{
		free_dir_node(ctx->base_node);
		delete ctx->base_node;
		ctx->base_node=NULL;
	}
	if(ctx->base_enode!=NULL)
	{
		free_enode(ctx->base_enode);
		delete ctx->base_enode;
		ctx->base_enode=NULL;
	}
	cache->clear();
}