#include "utility.h"
#include <string.h>
#include "stdlib.h"
#if !defined(BUILD_ON_WINDOWS)
#define CmpNoCase strncasecmp
#else
#define CmpNoCase strnicmp
#endif
DLL int compare_pathname(const string& a,const string& b)
{
	int ret=CmpNoCase(a.c_str(),b.c_str(),min(a.size(),b.size()));
	if(ret!=0)
		return ret;
	if(a.size()!=b.size())
		return a.size()<b.size()?-1:1;
	return 0;
}
DLL string format_segmented_u64(const UInteger64& u64)
{
	string sz=FormatI64(u64),dispsz;
	for(int i=0;i<(int)sz.size();i+=3)
	{
		string sec=((int)sz.size()>i+3?sz.substr(sz.size()-i-3,3):sz.substr(0,sz.size()-i));
		dispsz=sec+(i==0?"":",")+dispsz;
	}
	return dispsz;
}
struct rcopy_param
{
	char* from;
	char* to;
	char* start;
	char* tstart;
	file_recurse_callback* callback;
	file_recurse_cbdata* cbdata;
};
static int cb_copy(char* name, dword type, void* param, char dsym)
{
	int ret=0;
	rcopy_param* cpy=(rcopy_param*)param;
	file_recurse_callback* callback=cpy->callback;
	rcopy_param newcpy=*cpy;
	*(newcpy.start++)=dsym;
	*(newcpy.tstart++)=dsym;
	strcpy(newcpy.start,name);
	newcpy.start+=strlen(name);
	strcpy(newcpy.tstart,name);
	newcpy.tstart+=strlen(name);
	if(type==FILE_TYPE_DIR)
	{
		int retc=ret=callback->_mkdir_(newcpy.to);
		newcpy.cbdata!=NULL?ret=newcpy.cbdata->cb(ret,newcpy.to,type,newcpy.cbdata->param,dsym):0;
		return (ret==0&&retc==0)?callback->_ftraverse_(newcpy.from,cb_copy,&newcpy):ret;
	}
	else
	{
		ret=callback->_fcopy_(newcpy.from,newcpy.to);
		return newcpy.cbdata!=NULL?newcpy.cbdata->cb(ret,newcpy.to,type,newcpy.cbdata->param,dsym):ret;
	}
}
DLLAPI(int) recurse_fcopy(char* from, char* to, file_recurse_callback* callback, file_recurse_cbdata* cbdata, char dsym)
{
	int ret=0;
	dword type=0;
	if(0!=(ret=callback->_fstat_(from,&type)))
		return ret;
	rcopy_param rparam;
	rparam.from=new char[1024];
	rparam.to=new char[1024];
	strcpy(rparam.from,from);
	strcpy(rparam.to,to);
	rparam.start=rparam.from+strlen(rparam.from);
	rparam.tstart=rparam.to+strlen(rparam.to);
	rparam.callback=callback;
	rparam.cbdata=cbdata;
	if(rparam.start>rparam.from&&*(rparam.start-1)==dsym)
		*(--rparam.start)=0;
	if(rparam.tstart>rparam.to&&*(rparam.tstart-1)==dsym)
		*(--rparam.tstart)=0;
	if(strcmp(rparam.from,rparam.to)==0)
		goto end;
	if(type==FILE_TYPE_DIR)
		ret=callback->_mkdir_(rparam.to);
	else
		ret=callback->_fcopy_(from,to);
	{
		int retc=ret;
		cbdata!=NULL?ret=cbdata->cb(ret,rparam.to,type,cbdata->param,dsym):0;
		if(type==FILE_TYPE_DIR)
			(ret==0&&retc==0)?ret=callback->_ftraverse_(rparam.from,cb_copy,&rparam):0;
	}
end:
	delete[] rparam.from;
	delete[] rparam.to;
	return ret;
}
struct rdel_param
{
	char* path;
	char* start;
	file_recurse_callback* callback;
	file_recurse_cbdata* cbdata;
	int quitdel;
};
static int cb_delete(char* name, dword type, void* param, char dsym)
{
	int ret=0;
	rdel_param* del=(rdel_param*)param;
	file_recurse_callback* callback=del->callback;
	rdel_param newdel=*del;
	newdel.quitdel=0;
	*(newdel.start++)=dsym;
	strcpy(newdel.start,name);
	newdel.start+=strlen(name);
	if(type==FILE_TYPE_DIR)
		ret=callback->_ftraverse_(newdel.path,cb_delete,&newdel);
	*(newdel.start)=0;
	int retc=0;
	(ret==0&&newdel.quitdel==0)?ret=retc=callback->_fdelete_(newdel.path):0;
	ret!=0?del->quitdel=ret:(newdel.quitdel!=0?del->quitdel=newdel.quitdel:0);
	return (((ret==0&&newdel.quitdel==0)||retc!=0)&&newdel.cbdata!=NULL)?newdel.cbdata->cb(ret,newdel.path,type,newdel.cbdata->param,dsym):ret;
}
DLLAPI(int) recurse_fdelete(char* pathname, file_recurse_callback* callback, file_recurse_cbdata* cbdata, char dsym)
{
	dword type=0;
	int ret=callback->_fstat_(pathname,&type);
	if(ret==ERR_PATH_NOT_EXIST)
		return 0;
	if(ret!=0)
		return ret;
	rdel_param rparam;
	rparam.path=new char[1024];
	strcpy(rparam.path,pathname);
	rparam.start=rparam.path+strlen(rparam.path);
	rparam.callback=callback;
	rparam.cbdata=cbdata;
	rparam.quitdel=0;
	if(rparam.start>rparam.path&&*(rparam.start-1)==dsym)
		*(--rparam.start)=0;
	if(type==FILE_TYPE_DIR)
		ret=callback->_ftraverse_(rparam.path,cb_delete,&rparam);
	*rparam.start=0;
	int retc=0;
	(ret==0&&rparam.quitdel==0)?ret=retc=callback->_fdelete_(rparam.path):0;
	(((ret==0&&rparam.quitdel==0)||retc!=0)&&cbdata!=NULL)?ret=cbdata->cb(ret,rparam.path,type,cbdata->param,dsym):0;
	delete[] rparam.path;
	return ret;
}
struct rhandle_param
{
	char* path;
	char* start;
	file_recurse_handle_callback* callback;
	void* param;
	file_recurse_cbdata* cbdata;
	bool root_first;
};
static int cb_fhandle(char* name,dword type,void* param,char dsym);
static inline void __f_traverse__(rhandle_param* rparam,dword type,int& ret)
{
	if(type==FILE_TYPE_DIR)
	{
		ret==0?(ret=rparam->callback->_ftraverse_(rparam->path,cb_fhandle,rparam)):0;
		*(rparam->start)=0;
	}
}
static inline void __f_handle__(rhandle_param* rparam,dword type,int& ret,char dsym)
{
	int retc=0;
	ret==0?(ret=retc=rparam->callback->_handler_(rparam->path,type,rparam->param,dsym)):0;
	((ret==0||(retc!=0))&&rparam->cbdata!=NULL)?ret=rparam->cbdata->cb(ret,rparam->path,type,rparam->cbdata->param,dsym):0;
}
static int cb_fhandle(char* name,dword type,void* param,char dsym)
{
	int ret=0;
	rhandle_param* hstat=(rhandle_param*)param;
	file_recurse_handle_callback* callback=hstat->callback;
	rhandle_param newstat=*hstat;
	*(newstat.start++)=dsym;
	strcpy(newstat.start,name);
	newstat.start+=strlen(name);
	if(newstat.root_first)
	{
		__f_handle__(&newstat,type,ret,dsym);
		__f_traverse__(&newstat,type,ret);
	}
	else
	{
		__f_traverse__(&newstat,type,ret);
		__f_handle__(&newstat,type,ret,dsym);
	}
	return ret;
}
DLLAPI(int) recurse_fhandle(char* pathname,file_recurse_handle_callback* callback,bool root_first,void* param,file_recurse_cbdata* cbdata,char dsym)
{
	dword type=0;
	int ret=0;
	if(0!=(ret=callback->_fstat_(pathname,&type)))
		return ret;
	rhandle_param rparam;
	rparam.path=new char[1024];
	strcpy(rparam.path,pathname);
	rparam.start=rparam.path+strlen(rparam.path);
	rparam.callback=callback;
	rparam.param=param;
	rparam.cbdata=cbdata;
	rparam.root_first=root_first;
	if(rparam.start>rparam.path&&*(rparam.start-1)==dsym)
		*(--rparam.start)=0;
	if(rparam.root_first)
	{
		__f_handle__(&rparam,type,ret,dsym);
		__f_traverse__(&rparam,type,ret);
	}
	else
	{
		__f_traverse__(&rparam,type,ret);
		__f_handle__(&rparam,type,ret,dsym);
	}
	delete[] rparam.path;
	return ret;
}
static int cb_stat(char* pathname,dword type,void* param,char dsym)
{
	path_recurse_stat* pstat=(path_recurse_stat*)param;
	int ret=0;
	if(type==FILE_TYPE_NORMAL)
	{
		UInteger64 fsize(pstat->size.sizel,&pstat->size.sizeh),inc_size;
		void* hfile=sys_fopen(pathname,FILE_OPEN_EXISTING|FILE_READ);
		if(!VALID(hfile))
			return ERR_OPEN_FILE_FAILED;
		if(0!=(ret=sys_get_file_size(hfile,&inc_size.low,&inc_size.high)))
		{
			sys_fclose(hfile);
			return ret;
		}
		sys_fclose(hfile);
		fsize+=inc_size;
		pstat->size.sizel=fsize.low,pstat->size.sizeh=fsize.high;
	}
	switch(type)
	{
	case FILE_TYPE_NORMAL:
		pstat->nfile++;
		break;
	case FILE_TYPE_DIR:
		pstat->ndir++;
		break;
	}
	return 0;
}
DLLAPI(int) sys_recurse_fcopy(char* from,char* to,file_recurse_cbdata* cbdata)
{
	file_recurse_callback cb={sys_fstat,sys_ftraverse,sys_mkdir,sys_fcopy,sys_fdelete};
	return recurse_fcopy(from,to,&cb,cbdata,dir_symbol);
}
DLLAPI(int) sys_recurse_fdelete(char* pathname,file_recurse_cbdata* cbdata)
{
	file_recurse_callback cb={sys_fstat,sys_ftraverse,sys_mkdir,sys_fcopy,sys_fdelete};
	return recurse_fdelete(pathname,&cb,cbdata,dir_symbol);
}
DLLAPI(int) sys_recurse_fstat(char* pathname,path_recurse_stat* pstat,file_recurse_cbdata* cbdata)
{
	pstat->nfile=0;
	pstat->ndir=0;
	pstat->size.sizeh=pstat->size.sizel=0;
	file_recurse_handle_callback cb={sys_fstat,sys_ftraverse,cb_stat};
	return recurse_fhandle(pathname,&cb,true,pstat,cbdata,dir_symbol);
}
DLLAPI(void) random_integer64(UInteger64& num)
{
	for(int i=0;i<sizeof(UInteger64);i++)
	{
		((byte*)&num)[i]=rand()&0xff;
	}
}