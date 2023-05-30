#ifndef _STRUCT_H_
#define _STRUCT_H_
#include "common.h"
#include "datetime.h"
#include "Integer64.h"
#include <vector>
#include <string>
typedef int (*file_info_callback)(byte* buf,uint buflen,void* param);
struct intf_cntfile
{
	file_info_callback cb_info;
	file_info_callback cb_error;
	file_info_callback cb_rec;
	void* param;
};
inline void init_intf_cntfile(intf_cntfile* cbdata)
{
	cbdata->cb_info=NULL;
	cbdata->cb_error=NULL;
	cbdata->cb_rec=NULL;
	cbdata->param=NULL;
}
#define FLAG_NODE_EXPANDED    1
#define FLAG_NODE_DELETED     2
#define FLAG_NODE_LOADERR     4
#define node_state_exp(node) (!!((node)->flags&FLAG_NODE_EXPANDED))
#define node_expand(node) ((node)->flags|=FLAG_NODE_EXPANDED)
#define node_foldup(node) ((node)->flags&=~FLAG_NODE_EXPANDED)
struct fnode
{
	dword flags;
	UInteger64 fl_start;
	UInteger64 fl_end;
	void* handle;
	fnode():flags(0),handle(NULL){}
};
struct dir_contents;
struct dir_node:public fnode
{
	dir_contents* contents;
	dir_node():contents(NULL){}
};
//here base fnode member fl_start & fl_end represents non-standard records.
//In subdir list, they only represent subdir name.
//In err_contents, they represent error record with path & error description.
struct dir_err_contents
{
	vector<fnode> err_dirs;
	vector<fnode> err_files;
};
struct err_dir_node:public fnode
{
	dir_err_contents* err_contents;
	vector<err_dir_node>* subdirs;
	err_dir_node():err_contents(NULL),subdirs(NULL){}
};
struct dir_contents
{
	vector<dir_node> dirs;
	vector<fnode> files;
	dir_err_contents* err_contents;
	dir_contents():err_contents(NULL){}
};
inline uint get_subdir_cnt(dir_node* dir)
{
	if(dir==NULL||dir->contents==NULL)
		return 0;
	return dir->contents->dirs.size();
}
inline uint get_subfile_cnt(dir_node* dir)
{
	if(dir==NULL||dir->contents==NULL)
		return 0;
	return dir->contents->files.size();
}
inline dir_node* get_subdir(dir_node* dir,int idx)
{
	if(dir==NULL||dir->contents==NULL)
		return NULL;
	if(idx<0||idx>=(int)dir->contents->dirs.size())
		return NULL;
	return &dir->contents->dirs[idx];
}
inline fnode* get_subfile(dir_node* dir,int idx)
{
	if(dir==NULL||dir->contents==NULL)
		return NULL;
	if(idx<0||idx>=(int)dir->contents->files.size())
		return NULL;
	return &dir->contents->files[idx];
}
struct node_info_base
{
	string name;
	dword type;
	node_info_base():type(0){}
};
struct file_node_info:public node_info_base
{
	UInteger64 size;
	CDateTime mod_time;
};
struct err_node_info:public node_info_base
{
	string err_desc;
};
struct ctx_flist_loader
{
	string listfile;
	string errfile;
	void* hlf;
	void* hef;
	dir_node* base_node;
	ctx_flist_loader()
	{
		base_node=NULL;
		hlf=NULL;
		hef=NULL;
	}
};
#endif