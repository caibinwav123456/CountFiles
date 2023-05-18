#define DLL_IMPORT
#include "struct.h"
#include "datetime.h"
#include "Integer64.h"
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;
#if !defined(BUILD_ON_WINDOWS)
#define CmpNoCase strncasecmp
#else
#define CmpNoCase strnicmp
#endif
struct file_rec
{
	string name;
	dword type;
	UInteger64 size;
	CDateTime moddate;
};
struct file_list
{
	vector<file_rec> list;
};
struct file_list_ref
{
	vector<file_rec*> frefs;
	vector<file_rec*> drefs;
};
struct file_cnt_param
{
	UInteger64 total_size;
	CDateTime update_time;
	uint rec_len;
	bool start_flag;
	intf_cntfile* user_callback;
};
static bool less_rec(file_rec* a,file_rec* b)
{
	int ret=CmpNoCase(a->name.c_str(),b->name.c_str(),min(a->name.size(),b->name.size()));
	if(ret!=0)
		return ret<0;
	ret=(a->name.size()<b->name.size());
	if(ret!=0)
		return ret<0;
	return a->name<b->name;
}
static int init_param(file_cnt_param* param)
{
	//param->update_time=CDateTime();
	param->start_flag=true;
	param->total_size=UInteger64(0);
	param->rec_len=0;
	return 0;
}
static inline void log_error(const string& path,int ret,intf_cntfile* callback)
{
	char* errbuf=new char[path.size()+100];
	sprintf(errbuf,"\"%s\" error: %s\n",path.c_str(),get_error_desc(ret));
	printf("%s",errbuf);
	if(callback->cb_error!=NULL&&0!=(ret=callback->cb_error
		((byte*)errbuf,strlen(errbuf),callback->param)))
		printf("log error failed: %s\n",get_error_desc(ret));
	delete[] errbuf;
}
static inline int log_rec(const string& path,byte* buf,uint buflen,intf_cntfile* callback,file_cnt_param* param)
{
	int ret=0;
	param->rec_len+=buflen;
	if(0!=(ret=callback->cb_info(buf,buflen,callback->param)))
		log_error(path,ret,callback);
	return ret;
}
static inline int query_file_info(const string& path,UInteger64& size,CDateTime& date,intf_cntfile* callback)
{
	int ret=0;
	if(0!=(ret=sys_get_file_time((char*)path.c_str(),NULL,&date,NULL)))
	{
		log_error(path,ret,callback);
		return ret;
	}
	void* hFile=sys_fopen((char*)path.c_str(),FILE_READ|FILE_OPEN_EXISTING);
	if(!VALID(hFile))
	{
		ret=ERR_OPEN_FILE_FAILED;
		log_error(path,ret,callback);
		return ret;
	}
	if(0!=(ret=sys_get_file_size(hFile,&size.low,&size.high)))
		log_error(path,ret,callback);
	sys_fclose(hFile);
	return ret;
}
static int log_file_info(const string& path,const string& name,dword type,file_cnt_param* param,const file_cnt_param* ref_param)
{
	int ret=0;
	intf_cntfile* callback=param->user_callback;
	bool bDir=type==FILE_TYPE_DIR;
	const byte *Tag=(const byte*)(bDir?TAG_TYPE_DIR:TAG_TYPE_FILE);
	UInteger64 size;
	CDateTime date;
	string strsize,strdate;
	string pathname;
	byte buf[100];
	if(bDir)
	{
		param->rec_len+=ref_param->rec_len;
		size=ref_param->total_size;
		date=ref_param->update_time;
	}
	else
	{
		if(query_file_info(path,size,date,callback)!=0)
			return 0;
	}

	if(0!=(ret=log_rec(path,(byte*)Tag,strlen((const char*)Tag),callback,param)))
		return ret;
	if(bDir)
	{
		sprintf((char*)buf,"%s%u ",TAG_DRSIZE,ref_param->rec_len);
		if(0!=(ret=log_rec(path,buf,strlen((char*)buf),callback,param)))
			return ret;
	}
	pathname=string("\"")+name+"\" ";
	if(0!=(ret=log_rec(path,(byte*)pathname.c_str(),pathname.size(),callback,param)))
		return ret;
	strsize=FormatI64(size);
	date.Format(strdate,FORMAT_DATE|FORMAT_TIME|FORMAT_WEEKDAY);
	sprintf((char*)buf,"%s%s %s\n",TAG_SIZE,strsize.c_str(),strdate.c_str());
	if(0!=(ret=log_rec(path,buf,strlen((char*)buf),callback,param)))
		return ret;
	param->total_size+=size;
	if(param->start_flag)
	{
		param->update_time=date;
		param->start_flag=false;
	}
	else if(param->update_time<date)
		param->update_time=date;

	return 0;
}
static int cb_poll_file(char* filename,dword type,void* param,char dsym)
{
	file_list* flist=(file_list*)param;
	file_rec rec;
	rec.name=filename;
	rec.type=type;
	flist->list.push_back(rec);
	return 0;
}
static int recurse_cnt_file(const string& path,const string& name,file_cnt_param* param,dword type,char dsym,int recurse_lvl)
{
	int ret=0;
	file_list flist;
	file_list_ref ref;
	file_cnt_param next_param;
	next_param.user_callback=param->user_callback;
	init_param(&next_param);
	if(type==FILE_TYPE_NORMAL)
		return log_file_info(path,name,type,param,NULL);
	if(0!=(ret=sys_ftraverse((char*)(path).c_str(),cb_poll_file,&flist)))
		return ret;
	for(int i=0;i<(int)flist.list.size();i++)
	{
		switch(flist.list[i].type)
		{
		case FILE_TYPE_NORMAL:
			ref.frefs.push_back(&flist.list[i]);
			break;
		case FILE_TYPE_DIR:
			ref.drefs.push_back(&flist.list[i]);
			break;
		default:
			break;
		}
	}
	sort(ref.frefs.begin(),ref.frefs.end(),less_rec);
	sort(ref.drefs.begin(),ref.drefs.end(),less_rec);
	for(int i=0;i<(int)ref.frefs.size();i++)
	{
		if(0!=(ret=recurse_cnt_file(path+dsym+ref.frefs[i]->name,ref.frefs[i]->name,&next_param,
			ref.frefs[i]->type,dsym,recurse_lvl+1)))
			return ret;
	}
	for(int i=0;i<(int)ref.drefs.size();i++)
	{
		if(0!=(ret=recurse_cnt_file(path+dsym+ref.drefs[i]->name,ref.drefs[i]->name,&next_param,
			ref.drefs[i]->type,dsym,recurse_lvl+1)))
			return ret;
	}
	return log_file_info(path,name,type,param,&next_param);
}
int GenFileListInternal(char* path,intf_cntfile* callback,char dsym)
{
	if(callback==NULL)
		return ERR_GENERIC;
	int ret=0;
	dword type;
	string strpath=path;
	if((!strpath.empty())&&strpath.back()==dsym)
		strpath=strpath.substr(0,strpath.size()-1);
	uint pos=strpath.rfind(dsym);
	string name=pos==string::npos?strpath:strpath.substr(pos+1);
	file_cnt_param cnt_param;
	cnt_param.user_callback=callback;
	init_param(&cnt_param);
	if(0!=(ret=sys_fstat((char*)strpath.c_str(),&type)))
		goto end;
	if(0!=(ret=recurse_cnt_file(strpath,name,&cnt_param,type,dsym,0)))
		goto end;
end:
	return ret;
}