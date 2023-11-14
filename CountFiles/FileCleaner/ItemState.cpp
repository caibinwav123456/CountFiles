#include "pch.h"
#include "TreeListCtrl.h"
struct state_attrib
{
	bool solo;
	bool old;
	bool New;
	state_attrib():solo(false),old(false),New(false){}
};
E_FOLDER_STATE attr2state_map[2][2][2]=
	{{{eFSEqual,eFSNew},{eFSOld,eFSNewOld}},
	{{eFSSolo,eFSNewSolo},{eFSSoloOld,eFSNewSolo}}};
static inline void map_attrib(E_FOLDER_STATE state,state_attrib& attr)
{
	memset(&attr,0,sizeof(attr));
	switch(state)
	{
	case eFSOld:
		attr.old=true;
		break;
	case eFSNew:
		attr.New=true;
		break;
	case eFSSolo:
		attr.solo=true;
		break;
	case eFSNewOld:
		attr.New=true;
		attr.old=true;
		break;
	case eFSSoloOld:
		attr.solo=true;
		attr.old=true;
		break;
	case eFSNewSolo:
		attr.New=true;
		attr.solo=true;
		break;
	default:
		break;
	}
}
void merge_attrib(state_attrib& dest,const state_attrib& src)
{
	if(src.solo)
		dest.solo=true;
	if(src.old)
		dest.old=true;
	if(src.New)
		dest.New=true;
}
E_FOLDER_STATE make_state(const state_attrib& attr)
{
	return attr2state_map[attr.solo?1:0][attr.old?1:0][attr.New?1:0];
}
void TLItem::update_state()
{
	TLItem* cur;
	TLItemDir* pp;
	for(cur=this,pp=parent;!pp->IsBase();cur=pp,pp=pp->parent)
	{
		state_attrib attr,peerattr,
			pattr,ppeerattr; 
		TLItem **peer=cur->GetPeerItem(),**ppp=pp->GetPeerItem();
		if(peer==NULL||*peer==NULL)
		{
			attr.solo=true;
		}
		else
		{
			map_attrib(cur->state,attr);
			map_attrib((*peer)->state,peerattr);
		}
		if(ppp==NULL||*ppp==NULL)
		{
			//already solo
			//map_attrib(pp->state,pattr);
			//merge_attrib(pattr,attr);
		}
		else
		{
			map_attrib(pp->state,pattr);
			merge_attrib(pattr,attr);
			pp->state=make_state(pattr);
			map_attrib((*ppp)->state,ppeerattr);
			merge_attrib(ppeerattr,peerattr);
			(*ppp)->state=make_state(ppeerattr);
		}
	}
}
