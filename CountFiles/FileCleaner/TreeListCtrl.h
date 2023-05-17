#ifndef _TREE_LIST_CTRL_H_
#define _TREE_LIST_CTRL_H_
#include "FileListLoader.h"
#include "DrawObject.h"
string ConvertTStrToAnsiStr(LPCTSTR from);
CString ConvertAnsiStrToTStr(LPCSTR from);
CString ConvertAnsiStrToTStr(const string& from);
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
