#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include <cxcore.h>
#include <cv.h>
#include <highgui.h>
#include <Windows.h>
#define SAFE_FREE(ptr) \
	if(ptr!=NULL) \
	{ \
		free(ptr); \
		ptr=NULL; \
	}
enum op_type
{
	eOpMask=0,
	eOpForeGnd,
	eOpBlend,
	eOpBlendMono,
};
struct op_st
{
	op_type op;
	LPCTSTR tag;
};
op_st op_table[]=
{
	{eOpMask,_T("mask")},
	{eOpForeGnd,_T("bmp")},
	{eOpBlend,_T("blend")},
	{eOpBlendMono,_T("blend-mono")},
	{(op_type)-1,NULL},
};
CvScalar blend_color=cvScalarAll(1.0);
enum error_type
{
	eParamErr=1,
	eLoadErr,
	eSaveErr,
};
char* file_in=NULL;
char* file_out=NULL;
int op_code=eOpMask;
const char* get_err_desc(int err)
{
	switch(err)
	{
	case -eParamErr:
		return "Invalid Parameters";
	case -eLoadErr:
		return "Load Image Failed";
	case -eSaveErr:
		return "Save Image Failed";
	default:
		return "Unknown Error";
	}
}
template <class T>
inline T clamp(T val,T upper,T lower)
{
	T retval=val;
	retval>upper?retval=upper:0;
	retval<lower?retval=lower:0;
	return retval;
}
char* conv_string(LPCTSTR tstr)
{
#ifdef UNICODE
	int len=WideCharToMultiByte(CP_ACP,0,tstr,-1,NULL,0,NULL,NULL);
	char* strout=(char*)malloc(len);
	WideCharToMultiByte(CP_ACP,0,tstr,-1,strout,len,NULL,NULL);
	return strout;
#else
	return strdup(tstr);
#endif
}
void clean_up()
{
	SAFE_FREE(file_in);
	SAFE_FREE(file_out);
}
void print_usage()
{
	puts("\nConvImage - Convert Images\n"
		"Usage:\n"
		"\tConvImage [-h] [-bmp] [-mask] [-blend] [-blend-mono] (source-file-path) (destination-file-path)\n\n"
		"The Options are:\n\n"
		"-h: Display this message.\n"
		"-bmp: Make the foreground bitmap.\n"
		"-mask: Make the mask bitmap.This is the default option.\n"
		"-blend: Blend the bitmap using the specified blend color. When using this option, you should specify the blend color with this syntax:\n"
		"\t-blend (B):(G):(R)\n"
		"\tIn which B,G & R is the color component value represented in decimal integers in range 0~255.\n"
		"-blend-mono: Make a mono-color blended bitmap, using syntax as the \"-blend\" option to specify the blend color.\n"
	);
}
int parse_args(int argc,TCHAR** argv)
{
	if(argc<=1)
	{
		print_usage();
		return 1;
	}
	for(int i=1;i<argc;i++)
	{
		LPCTSTR arg=argv[i];
		if(arg[0]=='-')
		{
			LPCTSTR code=arg+1;
			if(_tcscmp(code,_T("h"))==0)
			{
				print_usage();
				return 1;
			}
			bool found=false;
			for(op_st* p=op_table;p->tag!=NULL;p++)
			{
				if(_tcscmp(code,p->tag)==0)
				{
					found=true;
					op_code=p->op;
					break;
				}
			}
			if(!found)
				return -eParamErr;
			switch(op_code)
			{
			case eOpBlend:
			case eOpBlendMono:
				{
					if((++i)>=argc)
						return -eParamErr;
					LPTSTR s=_tcsdup(argv[i]);
					LPTSTR start=s,end=s;
					double* pval=blend_color.val;
					for(int j=0;j<3;j++,start=end)
					{
						for(;*end!=0&&*end!=':';end++);
						if((j<2&&*end!=':')||(j==2&&*end!=0))
							goto err;
						*end=0;
						int v=-1;
						_stscanf_s(start,_T("%d"),&v);
						if(v<0)
							goto err;
						*(pval++)=(double)clamp(v,255,0)/255.0;
						end++;
					}
					free(s);
					break;
				err:
					free(s);
					return -eParamErr;
				}
			}
		}
		else if(file_in==NULL)
		{
			file_in=conv_string(arg);
		}
		else if (file_out == NULL)
		{
			file_out=conv_string(arg);
		}
	}
	if(file_in==NULL||file_out==NULL)
		return -eParamErr;
	return 0;
}
int ConvImage(IplImage* src,IplImage** pdest,int op)
{
	IplImage* image=cvCreateImage(cvSize(src->width,src->height),IPL_DEPTH_8U,3);
	*pdest=image;
	BYTE* ppx=(BYTE*)src->imageData;
	BYTE rgb[3];
	BYTE whitepx[3]={255,255,255};
	BYTE blackpx[3]={0,0,0};
	memcpy(rgb,ppx,3);
	for(int i=0;i<image->height;i++)
	{
		BYTE* srcline=((BYTE*)src->imageData)+src->widthStep*i;
		BYTE* destline=((BYTE*)image->imageData)+image->widthStep*i;
		for(int j=0;j<image->width;j++)
		{
			BYTE* srcpx=srcline+3*j;
			BYTE* destpx=destline+3*j;
			switch(op)
			{
			case eOpMask:
				if(memcmp(srcpx,rgb,3)==0)
					memcpy(destpx,whitepx,3);
				else
					memcpy(destpx,blackpx,3);
				break;
			case eOpForeGnd:
				if(memcmp(srcpx,rgb,3)==0)
					memcpy(destpx,blackpx,3);
				else
					memcpy(destpx,srcpx,3);
				break;
			case eOpBlend:
				if(memcmp(srcpx,rgb,3)==0)
					memcpy(destpx,srcpx,3);
				else
				{
					for(int i=0;i<3;i++)
					{
						double val=(double)(UINT)srcpx[i]*blend_color.val[i];
						destpx[i]=(BYTE)clamp(val,255.0,0.0);
					}
				}
				break;
			case eOpBlendMono:
				if(memcmp(srcpx,rgb,3)==0)
					memcpy(destpx,srcpx,3);
				else
				{
					int l=0;
					for(int i=0;i<3;i++)
						l+=(int)(UINT)srcpx[i];
					double fl=(double)l/3.0;
					for(int i=0;i<3;i++)
					{
						double val=fl*blend_color.val[i];
						destpx[i]=(BYTE)clamp(val,255.0,0.0);
					}
				}
				break;
			default:
				return -eParamErr;
			}
		}
	}
	return 0;
}
int _tmain(int argc,TCHAR** argv)
{
	int ret=parse_args(argc,argv);
	if(ret>0)
		return 0;
	else if(ret!=0)
		goto end;
	{
		IplImage* image_in=cvLoadImage(file_in);
		if(image_in==NULL)
		{
			ret=-eLoadErr;
			goto end;
		}
		IplImage* image_out=NULL;
		ret=ConvImage(image_in,&image_out,op_code);
		if(ret!=0)
			goto release;
		if(!cvSaveImage(file_out,image_out))
			ret=-eSaveErr;
	release:
		cvReleaseImage(&image_in);
		cvReleaseImage(&image_out);
	}
end:
	if(ret!=0)
		printf("Convert failed: %s.\n", get_err_desc(ret));
	else
		printf("Convert succeeded.\n");
	clean_up();
	return ret;
}