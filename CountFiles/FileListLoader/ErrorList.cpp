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
	uint tmplen=len;
	pass_byte('\"');
	if(!find_byte(ptr,tmplen,'\"'))
		return ERR_CORRUPTED_FILE;
	ptr=buf;
	uint namelen=len-tmplen;
	bool errdir=false;
	UInteger64 startoff,endoff;
	for(;;)
	{
		bool bfind=find_byte(ptr,namelen,dir_symbol);
		if(ptr==buf)
			return ERR_CORRUPTED_FILE;
		if((!bfind)||namelen==1)
		{
			startoff=off+UInteger64(buf-_buf),endoff=off+UInteger64(ptr-_buf);
			if(namelen==1)
			{
				errdir=true;
				endoff+=UInteger64(1);
				pass_byte(dir_symbol);
			}
			break;
		}
		*(byte*)ptr=0;
		name=(char*)buf;
		if(name.empty())
			return ERR_CORRUPTED_FILE;
		enode=add_err_dir_node(enode,stack,name,idx);
		init_err_node(enode,off+UInteger64(buf-_buf),off+UInteger64(ptr-_buf));
		pass_byte('\0');
		idx++;
	}
	efnode* err_file_node=add_leaf_err_node(enode,errdir);
	pass_byte('\"');
	pass_byte(' ');
	int ret=0;
	if(0!=(ret=pass_str(TAG_ERR_DESC,ptr,len)))
		return ret;
	buf=ptr;
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
int load_error_list(err_dir_node* enode,const UInteger64& off,const UInteger64& end,void* hef)
{
	UInteger64 tmpoff=off,tmpend,offstart=off,offend;
	const UInteger64 buf_len(64);
	vector<string> name_stack;
	byte buf[64];
	int ret=0;
	while(tmpoff<end)
	{
		tmpend=tmpoff+buf_len;
		if(tmpend>end)
			tmpend=end;
		uint len=(tmpend-tmpoff).low;
		if(0!=(ret=sys_fseek(hef,tmpoff.low,&tmpoff.high,SEEK_BEGIN)))
			return ret;
		if(0!=(ret=sys_fread(hef,buf,len)))
			return ret;
		const byte* ptr=buf;
		if(find_byte(ptr,len,'\n'))
		{
			offend=tmpoff+UInteger64(ptr+1-buf);
			uint reclen=(offend-offstart).low;
			byte* recbuf=new byte[reclen];
			if(0!=(ret=sys_fseek(hef,offstart.low,&offstart.high,SEEK_BEGIN)))
				goto end_read;
			if(0!=(ret=sys_fread(hef,recbuf,reclen)))
				goto end_read;
			ret=parse_error_rec(enode,name_stack,offstart,recbuf,reclen);
end_read:
			delete[] recbuf;
			if(ret!=0)
				return ret;
			offstart=offend;
		}
		tmpoff=tmpend;
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
		if(0!=(ret=get_err_dir_node_name(node,peinfo->name,hef,&peinfo->type)))
			goto end;
		{
			uint len=(node->efl_end-node->efl_start).low;
			char* buf=new char[len+1];
			if(0!=(ret=sys_fseek(hef,node->efl_start.low,&node->efl_start.high,SEEK_BEGIN)))
				goto end2;
			if(0!=(ret=sys_fread(hef,buf,len)))
				goto end2;
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
	if(0!=(ret=sys_fseek(hef,enode->fl_start.low,&enode->fl_start.high,SEEK_BEGIN)))
		goto end;
	if(0!=(ret=sys_fread(hef,buf,len)))
		goto end;
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