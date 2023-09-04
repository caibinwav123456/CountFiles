#ifndef _PROGRAM_DATA_H_
#define _PROGRAM_DATA_H_

struct ID2CWndPtrAssoc
{
	UINT id;
	INT_PTR offset;
	ID2CWndPtrAssoc(UINT _id,INT_PTR _off):id(_id),offset(_off){}
};
BOOL PDXAddToIDWndPtrMap(UINT id,CWnd* pWnd);
BOOL PDXRemoveFromIDWndPtrMap(UINT id);
CWnd* PDXGetWndFromID(UINT id);
template<typename T>
class CAddToWndMap
{
public:
	CAddToWndMap()
	{
		ID2CWndPtrAssoc* pAssoc=T::GetWndAssoc();
		VERIFY(PDXAddToIDWndPtrMap(pAssoc->id,(CWnd*)(((INT_PTR)this)-pAssoc->offset)));
	}
	~CAddToWndMap()
	{
		ID2CWndPtrAssoc* pAssoc=T::GetWndAssoc();
		PDXRemoveFromIDWndPtrMap(pAssoc->id);
	}
};
#define DECLARE_ID2WND_MAP(className) \
	static ID2CWndPtrAssoc* GetWndAssoc(); \
	CAddToWndMap<className> __thisMapObj;

#define IMPLEMENT_ID2WND_MAP(className,id) \
	ID2CWndPtrAssoc* className::GetWndAssoc() \
	{ \
		static ID2CWndPtrAssoc theAssoc(id,reinterpret_cast<INT_PTR> \
			(&(static_cast<className*>((CWnd*)NULL))->__thisMapObj)); \
		return &theAssoc; \
	}

#endif
