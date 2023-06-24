#ifndef _TREE_LIST_CTRL_H_
#define _TREE_LIST_CTRL_H_
#include "FileListLoader.h"
#include "DrawObject.h"
#include <vector>
#define t2a(p) ConvertTStrToAnsiStr(p)
#define a2t(p) ConvertAnsiStrToTStr(p)
enum E_FOLDER_STATE
{
	eFSEqual=1,
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
struct TLItem
{
	HDNODE node;
	vector<TLItem*> subitems;
	BOOL isopen;
	BOOL issel;
	uint open_length;
	TLItem* next_sel;
	TLItem():node(NULL),isopen(FALSE),issel(FALSE),open_length(0),next_sel(NULL)
	{
	}
};
struct ListCtrlDrawIterator
{
	friend class TreeListCtrl;
	operator bool();
	void operator++(int);
private:
	ListCtrlDrawIterator(TreeListCtrl* tl):m_pList(tl),m_pItem(NULL),m_iline(-1)
	{
	}
	TreeListCtrl* m_pList;
	TLItem* m_pItem;
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
	void DrawLine(CDrawer& drawer,int iline);
	void DrawLine(CDrawer& drawer,const ListCtrlDrawIterator& iter);
};
#endif
