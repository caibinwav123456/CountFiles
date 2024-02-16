#ifndef _STRUCT_H_
#define _STRUCT_H_
#include "common.h"
#include "datetime.h"
#include "Integer64.h"
#include <string>
#define fllapi DLL //FileListLoader API's
#define FLL_DECLARE_HANDLE(M) typedef struct _ ## M{}*H ## M;
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

FLL_DECLARE_HANDLE(FNODE) //HFNODE
FLL_DECLARE_HANDLE(DNODE) //HDNODE
FLL_DECLARE_HANDLE(ENODE) //HENODE

struct node_info_base
{
	string name;
	dword type;
	node_info_base():type(0){}
	virtual ~node_info_base(){};
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
#endif