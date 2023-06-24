#ifndef _TREE_LIST_CTRL_H_
#define _TREE_LIST_CTRL_H_
#include "FileListLoader.h"
#include "DrawObject.h"
#include <vector>
#define t2a(p) ConvertTStrToAnsiStr(p)
#define a2t(p) ConvertAnsiStrToTStr(p)
enum E_FOLDER_STATE
{
	eFSNone,
	eFSEqual,
	eFSOld,
	eFSNew,
	eFSSolo,
	eFSNewOld,
	eFSSoloOld,
	eFSNewSolo,
	eFSNReady,
	eFSError,
	eFSMax,
};
#define eFSAnormal eFSNReady
string ConvertTStrToAnsiStr(LPCTSTR from);
CString ConvertAnsiStrToTStr(LPCSTR from);
CString ConvertAnsiStrToTStr(const string& from);
enum E_TREE_ITEM_TYPE
{
	eITypeNone,
	eITypeDir,
	eITypeFile,
	eITypeErrDir,
	eITypeErrFile,
};
struct TLItem
{
	E_TREE_ITEM_TYPE type;
	E_FOLDER_STATE state;
	union
	{
		void* node;
		HDNODE dirnode;
		HFNODE filenode;
		HENODE errnode;
	};
	TLItem* parent;
	int parentidx;
	vector<TLItem*> subitems;
	bool isopen;
	bool issel;
	uint open_length;
	TLItem* next_sel;
	TLItem():type(eITypeNone),state(eFSNone),node(NULL),parent(NULL),parentidx(-1),isopen(false),issel(false),open_length(0),next_sel(NULL)
	{
	}
};
struct ItStkItem
{
	TLItem* m_pLItem;
	ItStkItem* next;
};
struct ListCtrlDrawIterator
{
	friend class TreeListCtrl;
	ListCtrlDrawIterator(ListCtrlDrawIterator& iter);
	~ListCtrlDrawIterator();
	operator bool();
	void operator++(int);
private:
	ListCtrlDrawIterator(TreeListCtrl* tl):m_pList(tl),m_pStkItem(NULL),lvl(0),m_iline(-1)
	{
	}
	TreeListCtrl* m_pList;
	ItStkItem* m_pStkItem;
	int lvl;
	int m_iline;
};
class TreeListCtrl
{
	friend struct ListCtrlDrawIterator;
public:
//Constructor/Destructor
	TreeListCtrl(CWnd* pWnd);
	virtual ~TreeListCtrl();

	int Init();
	void Exit();

//Draw callbacks
	void Draw(CDC* pClientDC,bool buffered);

//Message handlers
	void OnLBDown(const CPoint& pt);
	void OnLBUp(const CPoint& pt);
	void OnLBDblClick(const CPoint& pt);
	void OnRBDown(const CPoint& pt);
	void OnRBUp(const CPoint& pt);
	void OnMMove(const CPoint& pt);

protected:
//Get scroll pos
	virtual CPoint GetScrollPos() const=0;
//Set scroll sizes
	virtual void SetScrollSizes(const CSize& size)=0;

protected:
	CWnd* m_pWnd;

private:
	CBitmap m_bmpFolder;
	CBitmap m_bmpFolderMask;
	CBitmap m_bmpFolderExp;
	CBitmap m_bmpFolderExpMask;

//List data
private:
	FileListLoader m_ListLoader;
	uint m_nTotalLine;
	TLItem* m_pRootItem;
	TLItem* m_pItemSel;

//private functions
private:
	void Invalidate();
	void GetCanvasRect(RECT* rc);

//protected functions
protected:
	void DrawFolder(CDrawer* drawer,POINT* pt,int state,BOOL expand);
	ListCtrlDrawIterator GetDrawIter();
	int LineNumFromPt(POINT* pt);
	bool EndOfDraw(int iline);
	void DrawLine(CDrawer& drawer,int iline,TLItem* pItem=NULL);
	void DrawLine(CDrawer& drawer,const ListCtrlDrawIterator& iter);
};
#endif
