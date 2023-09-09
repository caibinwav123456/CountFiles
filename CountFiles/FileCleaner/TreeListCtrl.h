#ifndef _TREE_LIST_CTRL_H_
#define _TREE_LIST_CTRL_H_
#include "FileListLoader.h"
#include "DrawObject.h"
#include <vector>
#include <map>
#include <set>
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
enum E_TREE_ITEM_TYPE
{
	eITypeNone,
	eITypeDir,
	eITypeFile,
	eITypeErrDir,
	eITypeErrFile,
};
COLORREF GetDispColor(E_FOLDER_STATE state);
struct TLItem;
struct TLItemDir;
struct ItStkItem;
class ListCtrlIterator;
class TreeListCtrl;
struct SortedSelItemNode
{
	TLItem* pItem;
	int iline;
	map<int,SortedSelItemNode*> map_sub;
	SortedSelItemNode():pItem(NULL),iline(-1){}
	~SortedSelItemNode()
	{
		clear();
	}
	void clear();
	void extract();
};
class ItemSelector
{
public:
	struct SelItem
	{
		TLItem* item;
		int iline;
		SelItem(TLItem* p=NULL,int i=-1):item(p),iline(i){}
		void clear()
		{
			item=NULL;
			iline=-1;
		}
		bool operator<(const SelItem& other) const
		{
			return iline<other.iline;
		}
	};
	ItemSelector(TreeListCtrl* pOwner):m_pOwner(pOwner),m_iItemSel(-1),
		m_iDragStart(-1),m_iDragEnd(-1),m_bCancelRgn(false){}
	int GetFocus();
	bool IsFocus(int iline);
	bool IsSelected(TLItem* item,int iline);
	bool InDragRegion(int iline,bool* cancel=NULL);
	void SetSel(TLItem* item,int iline);
	void AddSel(TLItem* item,int iline);
	void CancelSel(TLItem* item,int iline);
	void ToggleSel(TLItem* item,int iline);
	bool BeginDragSel(int iline,bool cancel);
	bool DragSelTo(int iline);
	bool CompoundSel(int iline);
	void EndDragSel();
	void SortSelection(SortedSelItemNode& tree);
	bool valid(int iline);
private:
	set<SelItem> m_setSel;
	int m_iItemSel;
	int m_iDragStart;
	int m_iDragEnd;
	TreeListCtrl* m_pOwner;
	bool m_bCancelRgn;
};
struct TLItemPair
{
	TLItem* left;
	TLItem* right;
};
struct TLItemSplice
{
	vector<TLItemPair> map;
	void clear();
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
	TLItemDir* parent;
	int parentidx;
	int org;
	bool issel;
	TLItem():type(eITypeNone),state(eFSNone),node(NULL),parent(NULL),parentidx(-1),org(-1),issel(false)
	{
	}
	virtual ~TLItem(){}
	virtual void Release()=0;
	virtual uint GetDispLength()
	{
		return 1;
	}
	TLItem** GetPeerItem(TLItem*** _this=NULL);
	ItStkItem* FromLineNum(int iline,int& lvl);
	int ToLineNum();
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
	TLItemSplice* subpairs;
	bool isopen;
	uint open_length;
	uint dir_border;
	FileListLoader* ctx;
	TLItemDir(FileListLoader* loader):subpairs(NULL),isopen(false),open_length(0),dir_border(0),ctx(loader)
	{
	}
	int OpenDir(bool open,bool release=false);
	void Detach();
	virtual void Release();
	virtual uint GetDispLength();
private:
	void clear();
	int construct_list();
	void update_displen(int diff);
};
struct ItStkItem
{
	TLItem* m_pLItem;
	int parentidx;
	ItStkItem* next;
	ItStkItem(TLItem* pItem):m_pLItem(pItem),parentidx(-1),next(NULL){}
};
class ListCtrlIterator
{
public:
	ListCtrlIterator(TLItem* root,int iline,TreeListCtrl* pList=NULL);
	ListCtrlIterator(ListCtrlIterator& other);
	~ListCtrlIterator();
	operator bool();
	void operator++(int);
	void operator--(int);
	bool operator==(const ListCtrlIterator& other) const;
	bool operator!=(const ListCtrlIterator& other) const;
	bool operator>(const ListCtrlIterator& other) const;
	bool operator<(const ListCtrlIterator& other) const;
	bool operator>=(const ListCtrlIterator& other) const;
	bool operator<=(const ListCtrlIterator& other) const;
	ItStkItem* m_pStkItem;
	int lvl;
	int m_iline;
private:
	TreeListCtrl* m_pList;
	bool end;
};
class TreeListCtrl
{
	friend class ListCtrlIterator;
	friend class ItemSelector;
public:
//Constructor/Destructor
	TreeListCtrl(CWnd* pWnd);
	virtual ~TreeListCtrl();

	int Init();
	void Exit();

	int Load(const char* lfile,const char* efile);
	void UnLoad();

//Draw callbacks
	void Draw(CDC* pClientDC,bool buffered);

//Message handlers
	void OnLBDown(const CPoint& pt,UINT nFlags);
	void OnLBUp(const CPoint& pt,UINT nFlags);
	void OnLBDblClick(const CPoint& pt,UINT nFlags);
	void OnRBDown(const CPoint& pt,UINT nFlags);
	void OnRBUp(const CPoint& pt,UINT nFlags);
	void OnMMove(const CPoint& pt,UINT nFlags);

public:
//Get scroll position
	virtual CPoint GetScrollPos() const=0;
//Set scroll sizes
	virtual void SetScrollSizes(const CSize& size)=0;
//Get scroll sizes
	virtual CSize GetScrollSizes()=0;

protected:
	CWnd* m_pWnd;

private:
	CBitmap m_bmpFolder;
	CBitmap m_bmpFolderMask;
	CBitmap m_bmpFolderExp;
	CBitmap m_bmpFolderExpMask;

	int m_iCurLine;

//List data
protected:
	FileListLoader m_ListLoader;
	uint m_nTotalLine;
	TLItem* m_pRootItem;
	ItemSelector m_ItemSel;

//private functions
private:
	void Invalidate();
	void GetCanvasRect(RECT* rc);

//protected functions
protected:
	void DrawFolder(CDrawer* drawer,POINT* pt,E_FOLDER_STATE state,BOOL expand);
	ListCtrlIterator GetDrawIter(POINT* pt=NULL);
	ListCtrlIterator GetListIter(int iline);
	int LineNumFromPt(POINT* pt);
	bool EndOfDraw(int iline);
	void DrawLine(CDrawer& drawer,int iline,TLItem* pItem=NULL);
	void DrawConn(CDrawer& drawer,const ListCtrlIterator& iter);
	void DrawLine(CDrawer& drawer,const ListCtrlIterator& iter);
	void UpdateListStat();
};
#endif
