#ifndef _TREE_LIST_CTRL_H_
#define _TREE_LIST_CTRL_H_
#include "FileListLoader.h"
#include "DrawObject.h"
string ConvertTStrToAnsiStr(LPCTSTR from);
CString ConvertAnsiStrToTStr(LPCSTR from);
CString ConvertAnsiStrToTStr(const string& from);
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
	bool IsFocus(int iline);
	bool IsSelected(TLItem* item,int iline);
	bool InDragRegion(int iline,bool* cancel=NULL);
	void SetSel(TLItem* item,int iline);
	void AddSel(TLItem* item,int iline);
	void CancelSel(TLItem* item,int iline);
	void ToggleSel(TLItem* item,int iline);
	bool BeginDragSel(int iline,bool cancel);
	bool DragSelTo(int iline);
	void EndDragSel();
	void SortSelection(SortedSelItemNode& tree);
private:
	set<SelItem> m_setSel;
	int m_iItemSel;
	int m_iDragStart;
	int m_iDragEnd;
	TreeListCtrl* m_pOwner;
	bool m_bCancelRgn;
	bool valid(int iline);
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

//private functions
private:
	void Invalidate();

//protected functions
protected:
	void DrawFolder(CDrawer* drawer,POINT* pt,int state,BOOL expand);
};
#endif
