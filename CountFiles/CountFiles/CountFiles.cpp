#include <tchar.h>
#include <stdio.h>
#include "common.h"
#include "GenFileList.h"
#define safe_fclose(hfile) \
	if(VALID(hfile)) \
	{ \
		sys_fclose(hfile); \
		hfile=NULL; \
	}
struct FileObject
{
	void* hFile;
	void* hFileErr;
	const char* file;
	const char* err_file;
	bool bMute;
};
static int cb_wr_file_info(byte* buf,uint buflen,void* param)
{
	FileObject* obj=(FileObject*)param;
	return sys_fwrite(obj->hFile,buf,buflen,NULL);
}
static int cb_wr_file_err(byte* buf,uint buflen,void* param)
{
	FileObject* obj=(FileObject*)param;
	if(!obj->bMute)
		printf("%s",buf);
	if(VALID(obj->hFileErr))
		return sys_fwrite(obj->hFileErr,buf,buflen,NULL);
	return 0;
}
int parse_options(int argc,char** argv,const char*& file,const char*& cnt_path,const char*& err_file,bool& bmute)
{
	file=NULL;
	cnt_path=NULL;
	err_file=NULL;
	bmute=false;
	for(int i=1;i<argc;i++)
	{
		char* cmditem=argv[i];
		if(*cmditem=='-')
		{
			char* option=cmditem+1;
			if(strlen(option)!=1)
			{
				printf("\"%s\": invalid option\n",option);
				return -1;
			}
			switch(*option)
			{
			case 'e':
				i++;
				if(i>=argc)
				{
					printf("option -e: file not specified\n");
					return -1;
				}
				err_file=argv[i];
				break;
			case 'm':
				bmute=true;
				break;
			default:
				printf("\"%s\": invalid option\n",option);
				return -1;
			}
		}
		else if(cnt_path==NULL)
			cnt_path=argv[i];
		else if(file==NULL)
			file=argv[i];
		else
		{
			printf("\"%s\": invalid param\n",argv[i]);
			return -1;
		}
	}
	if(cnt_path==NULL||file==NULL)
	{
		printf("count path or output file not specified\n");
		return -1;
	}
	return 0;
}
void clean_write_obj(FileObject* obj,bool bdelete)
{
	safe_fclose(obj->hFile);
	safe_fclose(obj->hFileErr);
	if(bdelete)
	{
		sys_fdelete((char*)obj->file);
		if(obj->err_file!=NULL)
			sys_fdelete((char*)obj->err_file);
	}
}
int _tmain(int argc,TCHAR** argv)
{
	if(argc==1||(argc==2&&strcmp(argv[1],"-v")==0))
	{
		printf("Count Files V1.0\n"
			"Copyright CaiBin 2023\n"
			"All Rights Reserved\n"
			"Type \'CountFiles -h\' for help\n\n"
		);
		return 0;
	}
	if(argc==2&&strcmp(argv[1],"-h")==0)
	{
		printf("Usage: CountFiles [count path] [output file] [options]\n"
			"Additional Options:\n"
			"-v: show version info\n"
			"-h: show help\n"
			"-m: do not display errors\n"
			"-e [filepath]: specify error output file\n\n"
		);
		return 0;
	}
	int ret=0;
	FileObject obj;
	intf_cntfile callback;
	const char* cnt_path;
	return_ret(ret,0,parse_options(argc,argv,obj.file,cnt_path,obj.err_file,obj.bMute));
	obj.hFile=sys_fopen((char*)obj.file,FILE_WRITE|FILE_CREATE_ALWAYS);
	obj.hFileErr=NULL;

	if(!VALID(obj.hFile))
	{
		printf("\"%s\": can not open file for writing\n",obj.file);
		return ERR_OPEN_FILE_FAILED;
	}
	if(obj.err_file!=NULL)
	{
		obj.hFileErr=sys_fopen((char*)obj.err_file,FILE_WRITE|FILE_CREATE_ALWAYS);
		if(!VALID(obj.hFileErr))
		{
			clean_write_obj(&obj,true);
			printf("\"%s\": can not open file for writing\n",obj.err_file);
			return ERR_OPEN_FILE_FAILED;
		}
	}
	callback.param=&obj;
	callback.cb_info=cb_wr_file_info;
	callback.cb_error=cb_wr_file_err;
	callback.cb_rec=NULL;
	fail_op(ret,0,GenFileList(cnt_path,&callback),
	{
		clean_write_obj(&obj,true);
		printf("GenFileList failed: %s\n",get_error_desc(ret));
		return ret;
	})
	clean_write_obj(&obj,false);
	return 0;
}
