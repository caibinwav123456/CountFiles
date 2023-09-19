#ifndef _COMMON_STRUCT_H_
#define _COMMON_STRUCT_H_
#include <vector>
using namespace std;
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
struct TabItem
{
	CRect rect;
	CString title;
	TabItem(){}
	TabItem(const CRect& rc,const CString& str):rect(rc),title(str){}
};
#define TLTAB_NAME   1
#define TLTAB_SIZE   2
#define TLTAB_MODIFY 4
#define TLTAB_ALL     (TLTAB_NAME|TLTAB_SIZE|TLTAB_MODIFY)
#define TLTAB_INITIAL (TLTAB_NAME|TLTAB_SIZE|TLTAB_MODIFY)
struct TreeListTabGrid
{
	UINT mask;
	CRect rcTotal;
	vector<TabItem> arrTab;
	TreeListTabGrid():mask(0){}
};
struct TabInfo
{
	TreeListTabGrid* left;
	TreeListTabGrid* right;
	TabInfo(TreeListTabGrid* _l,TreeListTabGrid* _r):left(_l),right(_r){}
};
#endif