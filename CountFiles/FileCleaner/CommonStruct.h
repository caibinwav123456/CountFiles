#ifndef _COMMON_STRUCT_H_
#define _COMMON_STRUCT_H_
#define FILE_LIST_ATTRIB_MAIN 1
#define FILE_LIST_ATTRIB_REF  2
struct FListLoadData
{
	CString left;
	CString right;
	UINT mask;
	FListLoadData(const CString& _l,const CString& _r,UINT _mask)
		:left(_l),right(_r),mask(_mask){}
};
#endif