#define DLL_IMPORT
#include "struct.h"
#include "LRUCache.h"
#include "FileListLoaderInternal.h"
int load_error_list(err_dir_node* enode,void* hef)
{
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
		if(0!=(ret=get_err_dir_node_name(node,peinfo->name,hef)))
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
int get_err_dir_node_name(fnode* enode,string& name,void* hef)
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
	return ret;
}