#include "ASTError.h"
#undef DEF_ERROR
#define DEF_ERROR(code,str) str
static const char* err_descs[]=
{
#include "error.h"
};
DLL const char* get_error_desc(int errcode)
{
	if(_r(errcode)<0||_r(errcode)>=sizeof(err_descs)/sizeof(char*))
		errcode=ERR_GENERIC;
	return err_descs[_r(errcode)];
}
