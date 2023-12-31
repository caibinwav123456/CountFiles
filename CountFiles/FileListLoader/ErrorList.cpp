#define DLL_IMPORT
#include "struct.h"
#include "LRUCache.h"
#include "FileListLoaderInternal.h"
static inline err_dir_node* add_err_dir_node(err_dir_node* enode,vector<string>& stack,const string& name,uint idx)
{
	assert(idx<=(uint)stack.size());
	if(idx<(uint)stack.size()&&stack[idx]==name)
	{
		assert(enode->subdirs!=NULL);
		return &enode->subdirs->back();
	}
	if(idx==(uint)stack.size())
	{
		assert(enode->subdirs==NULL);
		enode->subdirs=new vector<err_dir_node>;
		enode->subdirs->push_back(err_dir_node());
		stack.push_back(name);
	}
	else
	{
		assert(enode->subdirs!=NULL);
		enode->subdirs->push_back(err_dir_node());
		stack[idx]=name;
		stack.erase(stack.begin()+idx+1,stack.end());
	}
	return &enode->subdirs->back();
}
static inline efnode* add_leaf_err_node(err_dir_node* enode,bool bdir)
{
	if(enode->err_contents==NULL)
		enode->err_contents=new dir_err_contents;
	if(bdir)
	{
		enode->err_contents->err_dirs.push_back(efnode());
		return &enode->err_contents->err_dirs.back();
	}
	else
	{
		enode->err_contents->err_files.push_back(efnode());
		return &enode->err_contents->err_files.back();
	}
}
static inline void init_err_node(fnode* node,UInteger64 fl_start,UInteger64 fl_end)
{
	node->fl_start=fl_start;
	node->fl_end=fl_end;
}
static inline void init_leaf_err_node(efnode* node,UInteger64 fl_start,UInteger64 fl_end,UInteger64 efl_start,UInteger64 efl_end)
{
	init_err_node(node,fl_start,fl_end);
	node->efl_start=efl_start;
	node->efl_end=efl_end;
}
static int parse_error_rec(err_dir_node* enode,vector<string>& stack,const UInteger64& off,const byte* _buf,uint len)
{
	string name;
	uint idx=0;
	const byte *buf=_buf,*ptr=_buf;
	pass_byte('\"');
	uint tmplen=len;
	if(!find_byte(ptr,tmplen,'\"'))
		return ERR_CORRUPTED_FILE;
	const byte *namebuf=buf,*nameptr=buf;
	buf=ptr;
	uint namelen=len-tmplen;
	len=tmplen;
	bool errdir=false;
	UInteger64 startoff,endoff;
	for(;;)
	{
		bool bfind=find_byte(nameptr,namelen,dir_symbol);
		if(nameptr==namebuf)
			return ERR_CORRUPTED_FILE;
		if((!bfind)||namelen==1)
		{
			startoff=off+UInteger64(namebuf-_buf),endoff=off+UInteger64(nameptr-_buf);
			if(namelen==1)
			{
				errdir=true;
				endoff+=UInteger64(1);
				pass_byte_spec(namebuf,nameptr,namelen,dir_symbol);
			}
			break;
		}
		*(byte*)nameptr=0;
		name=(char*)namebuf;
		if(name.empty())
			return ERR_CORRUPTED_FILE;
		enode=add_err_dir_node(enode,stack,name,idx);
		init_err_node(enode,off+UInteger64(namebuf-_buf),off+UInteger64(nameptr-_buf));
		pass_byte_spec(namebuf,nameptr,namelen,'\0');
		idx++;
	}
	efnode* err_file_node=add_leaf_err_node(enode,errdir);
	pass_byte('\"');
	pass_byte(' ');
	pass_str(TAG_ERR_DESC);
	if(len<1)
		return ERR_CORRUPTED_FILE;
	ptr+=len-1;
	len=1;
	init_leaf_err_node(err_file_node,startoff,endoff,
		off+UInteger64(buf-_buf),off+UInteger64(ptr-_buf));
	pass_byte('\n');
	if(len!=0)
		return ERR_CORRUPTED_FILE;
	return 0;
}
int FindLine(UInteger64& off,const UInteger64& end,void* hlf)
{
	const UInteger64 buf_len(64);
	UInteger64 tmpoff=off,tmpend;
	byte buf[64];
	int ret=0;
	while(tmpoff<end)
	{
		tmpend=tmpoff+buf_len;
		if(tmpend>end)
			tmpend=end;
		uint len=(tmpend-tmpoff).low;
		return_ret(ret,0,sys_fseek(hlf,tmpoff.low,&tmpoff.high,SEEK_BEGIN));
		return_ret(ret,0,sys_fread(hlf,buf,len));
		const byte* ptr=buf;
		if(find_byte(ptr,len,'\n'))
		{
			off=tmpoff+UInteger64(ptr+1-buf);
			return 0;
		}
		tmpoff=tmpend;
	}
	off=end;
	return 0;
}
int load_error_list(err_dir_node* enode,const UInteger64& off,const UInteger64& end,void* hef)
{
	UInteger64 tmpoff=off,offstart=off,offend;
	vector<string> name_stack;
	int ret=0;
	while(tmpoff<end)
	{
		return_ret(ret,0,FindLine(tmpoff,end,hef));
		offend=tmpoff;
		uint reclen=(offend-offstart).low;
		byte* recbuf=new byte[reclen];
		fail_goto(ret,0,sys_fseek(hef,offstart.low,&offstart.high,SEEK_BEGIN),end_read);
		fail_goto(ret,0,sys_fread(hef,recbuf,reclen),end_read);
		ret=parse_error_rec(enode,name_stack,offstart,recbuf,reclen);
end_read:
		delete[] recbuf;
		if(ret!=0)
			return ret;
		offstart=offend;
	}
	return 0;
}
int retrieve_enode_info(efnode* node,err_node_info* pinfo,void* hef,LRUCache* cache)
{
	if(node==NULL)
		return ERR_INVALID_PARAM;
	int ret=0;
	node_info_base* item=(node_info_base*)cache->get(&node->handle);
	err_node_info* peinfo;
	if(item!=NULL)
	{
		peinfo=dynamic_cast<err_node_info*>(item);
		assert(peinfo!=NULL);
	}
	else
	{
		item=peinfo=new err_node_info;
		fail_goto(ret,0,get_err_dir_node_name(node,peinfo->name,hef,&peinfo->type),end);
		{
			uint len=(node->efl_end-node->efl_start).low;
			char* buf=new char[len+1];
			fail_goto(ret,0,sys_fseek(hef,node->efl_start.low,&node->efl_start.high,SEEK_BEGIN),end2);
			fail_goto(ret,0,sys_fread(hef,buf,len),end2);
			buf[len]=0;
			peinfo->err_desc=buf;
end2:
			delete[] buf;
		}
end:
		if(ret!=0)
		{
			delete peinfo;
			return ret;
		}
		cache->put(item,&node->handle);
	}
	*pinfo=*peinfo;
	return 0;
}
int get_err_dir_node_name(fnode* enode,string& name,void* hef,dword* type)
{
	int ret=0;
	uint len=(enode->fl_end-enode->fl_start).low;
	char* buf=new char[len+1];
	fail_goto(ret,0,sys_fseek(hef,enode->fl_start.low,&enode->fl_start.high,SEEK_BEGIN),end);
	fail_goto(ret,0,sys_fread(hef,buf,len),end);
	buf[len]=0;
	name=buf;
end:
	delete[] buf;
	bool bdir=false;
	if(name.size()>0&&name.back()==dir_symbol)
	{
		name=name.substr(0,name.size()-1);
		bdir=true;
	}
	if(type!=NULL)
		*type=(bdir?FILE_TYPE_DIR:FILE_TYPE_NORMAL);
	return ret;
}