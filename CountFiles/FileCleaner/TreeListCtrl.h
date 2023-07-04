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
struct TLItemDir;
struct ItStkItem;
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
	TLItemDir* parent;
	int parentidx;
	bool issel;
	TLItem* next_sel;
	TLItem():type(eITypeNone),state(eFSNone),node(NULL),parent(NULL),parentidx(-1),issel(false),next_sel(NULL)
	{
	}
	virtual ~TLItem(){}
	virtual void Release()=0;
	virtual uint GetDispLength()
	{
		return 1;
	}
	ItStkItem* FromLineNum(int iline,int& lvl);
};
struct TLItemFile:public TLItem
{
	virtual void Release(){};
};
struct TLItemErrDir:public TLItem
{
	virtual void Release(){};
};
struct TLItemErrFile:public TLItem
{
	virtual void Release(){};
};
struct TLItemDir:public TLItem
{
	vector<TLItem*> subitems;
	vector<TLItemDir*> subdirs;
	vector<TLItemFile*> subfiles;
	vector<TLItemErrDir*> errdirs;
	vector<TLItemErrFile*> errfiles;
	bool isopen;
	uint open_length;
	uint dir_border;
	FileListLoader* ctx;
	TLItemDir(FileListLoader* loader):isopen(false),open_length(0),dir_border(0),ctx(loader)
	{
	}
	int OpenDir(bool open,bool release=false);
	virtual void Release();
	virtual uint GetDispLength();
private:
	void clear();
	int construct_list(bool all=true);
	void update_displen(int diff);
};
struct ItStkItem
{
	TLItem* m_pLItem;
	int parentidx;
	ItStkItem* next;
	ItStkItem(TLItem* pItem):m_pLItem(pItem),parentidx(-1),next(NULL){}
};
struct ListCtrlDrawIterator
{
	friend class TreeListCtrl;
	ListCtrlDrawIterator(ListCtrlDrawIterator& other);
	~ListCtrlDrawIterator();
	operator bool();
	void operator++(int);
	void operator--(int);
	bool operator==(const ListCtrlDrawIterator& other) const;
private:
	ListCtrlDrawIterator(TreeListCtrl* tl):m_pList(tl),m_pStkItem(NULL),lvl(0),m_iline(-1),end(false)
	{
	}
	TreeListCtrl* m_pList;
	ItStkItem* m_pStkItem;
	int lvl;
	int m_iline;
	bool end;
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
	void OnLBDown(const CPoint& pt,UINT nFlags);
	void OnLBUp(const CPoint& pt,UINT nFlags);
	void OnLBDblClick(const CPoint& pt,UINT nFlags);
	void OnRBDown(const CPoint& pt,UINT nFlags);
	void OnRBUp(const CPoint& pt,UINT nFlags);
	void OnMMove(const CPoint& pt,UINT nFlags);

protected:
//Get scroll position
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
	ListCtrlDrawIterator GetDrawIter(POINT* pt=NULL);
	int LineNumFromPt(POINT* pt);
	void SetSel(TLItem* item);
	void AddSel(TLItem* item);
	bool EndOfDraw(int iline);
	void DrawLine(CDrawer& drawer,int iline,TLItem* pItem=NULL);
	void DrawConn(CDrawer& drawer,const ListCtrlDrawIterator& iter);
	void DrawLine(CDrawer& drawer,const ListCtrlDrawIterator& iter);
};
#endif
