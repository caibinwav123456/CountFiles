#include "common.h"
#include "LRUCache.h"
#include <stdio.h>
#include <string.h>
#define clear_mem(m) memset(&m,0,sizeof(m))
#define NULLSLOT ((uint)-1)
#ifdef CONFIG_X64
#define hiptr(ptr) ((uint)(((unsigned long long)(ptr))>>32))
#define loptr(ptr) ((uint)(((unsigned long long)(ptr))&0xffffffff))
#define mkptr(hi,lo) ((void*)((((unsigned long long)(uint)(hi))<<32)|(((unsigned long long)(uint)(lo))&0xffffffff)))
static inline uint next_id(uint id)
{
	if(id+1==0)
		return 1;
	else
		return id+1;
}
static inline uint expire_id(uint id,uint refid)
{
	int sid=(int)id;
	int srefid=(int)refid;
	if(srefid-sid>=0)
		return id;
	int rid=srefid-0x7fffffff;
	return (uint)rid;
}
#else
#define hiptr(ptr) ((uint)(((unsigned long long)(ptr))>>16))
#define loptr(ptr) ((uint)(((unsigned long long)(ptr))&0xffff))
#define mkptr(hi,lo) ((void*)(((hi)<<16)|((lo)&0xffff)))
static inline uint next_id(uint id)
{
	if(id>=0xffff)
		return 1;
	else
		return id+1;
}
static inline uint expire_id(uint id,uint refid)
{
	short sid=(short)(ushort)id;
	short srefid=(short)(ushort)refid;
	if(srefid-sid>=0)
		return id;
	short rid=srefid-(short)0x7fff;
	return (uint)(ushort)rid;
}
#endif
LRUCache::LRUCache(uint _capacity,void (*_free_item)(void*))
{
	size=0;
	capacity=_capacity;
	free_item=_free_item;
	next_sid=1;
	if(capacity<1)
	{
		printf("capacity underflow, reset to 1\n");
		capacity=1;
	}
	else if(capacity>3000)
	{
		printf("capacity overflow, reset to 3000\n");
		capacity=3000;
	}
	clear_mem(head);
	clear_mem(tail);
	clear_mem(free_head);
	clear_mem(free_tail);
	head.next=&tail;
	tail.prev=&head;
	free_head.next=&free_tail;
	free_tail.prev=&free_head;
	lookup_table=new HandleSlot[capacity];
	memset(lookup_table,0,capacity*sizeof(HandleSlot));
	for(int i=0;i<(int)capacity;i++)
	{
		lookup_table[i].newslot=NULLSLOT;
		add_to_free(lookup_table+i);
	}
}
LRUCache::~LRUCache()
{
	clear();
	delete[] lookup_table;
}
void LRUCache::clear()
{
	head.next=&tail;
	tail.prev=&head;
	free_head.next=&free_tail;
	free_tail.prev=&free_head;
	if(size==0)
		return;
	size=0;
	for(int i=0;i<(int)capacity;i++)
	{
		if(lookup_table[i].item!=NULL)
			free_item(lookup_table[i].item);
		clear_mem(lookup_table[i]);
		lookup_table[i].newslot=NULLSLOT;
		add_to_free(lookup_table+i);
	}
}
void* LRUCache::get(void** phandle)
{
	void*& handle=*phandle;
	if(handle==NULL)
		return NULL;
	uint slot=hiptr(handle);
	if(slot>=capacity)
		return NULL;
	uint sid=loptr(handle);
	sanitize(slot);
	HandleSlot& hslot=lookup_table[slot];
	if(hslot.sid==sid&&hslot.item!=NULL)
	{
		move_to_front(&hslot);
		return hslot.item;
	}
	else if(hslot.oldsid==sid)
	{
		if(hslot.newslot==NULLSLOT)
			return NULL;
		HandleSlot& newslot=lookup_table[hslot.newslot];
		handle=mkptr(hslot.newslot,newslot.sid);
		move_to_front(&newslot);
		return newslot.item;
	}
	else
	{
		return NULL;
	}
}
void LRUCache::put(void* item,void** phandle)
{
	void*& handle=*phandle;
	HandleSlot* pslot;
	bool full;
	if(free_head.next!=&free_tail)
	{
		pslot=free_head.next;
		remove(pslot);
		add_to_front(pslot);
		size++;
		full=false;
	}
	else
	{
		pslot=tail.prev;
		free_item(pslot->item);
		pslot->oldsid=pslot->sid;
		pslot->newslot=NULLSLOT;
		move_to_front(pslot);
		full=true;
	}
	HandleSlot& slot=*pslot;
	uint idx=pslot-lookup_table;
	if(full&&handle!=NULL)
	{
		uint oldslot=hiptr(handle);
		if(oldslot>=capacity)
			goto end_update_new_slot;
		HandleSlot& holdslot=lookup_table[oldslot];
		uint s=expire_id(holdslot.oldsid,next_sid);
		if(s!=holdslot.oldsid)
		{
			holdslot.oldsid=0;
			holdslot.newslot=NULLSLOT;
		}
		else if(holdslot.oldsid==loptr(handle)
			&&holdslot.newslot==NULLSLOT)
		{
			holdslot.newslot=idx;
		}
	}
end_update_new_slot:
	slot.item=item;
	slot.sid=next_sid;
	next_sid=next_id(next_sid);
	handle=mkptr(idx,slot.sid);
}
void LRUCache::move_to_front(HandleSlot* slot)
{
	slot->prev->next=slot->next;
	slot->next->prev=slot->prev;
	HandleSlot* next=head.next;
	next->prev=slot;
	head.next=slot;
	slot->prev=&head;
	slot->next=next;
}
void LRUCache::add_to_front(HandleSlot* slot)
{
	HandleSlot* next=head.next;
	head.next=slot;
	next->prev=slot;
	slot->prev=&head;
	slot->next=next;
}
void LRUCache::add_to_free(HandleSlot* slot)
{
	HandleSlot* prev=free_tail.prev;
	free_tail.prev=slot;
	prev->next=slot;
	slot->prev=prev;
	slot->next=&free_tail;
}
void LRUCache::remove(HandleSlot* slot)
{
	slot->prev->next=slot->next;
	slot->next->prev=slot->prev;
	slot->prev=slot->next=slot;
}
inline void LRUCache::sanitize_one_slot(uint nslot)
{
	HandleSlot& hslot=lookup_table[nslot];
	uint s=expire_id(hslot.sid,next_sid);
	if(s==hslot.sid)
		return;
	hslot.sid=0;
	remove(&hslot);
	add_to_free(&hslot);
	size--;
	if(hslot.item!=NULL)
	{
		free_item(hslot.item);
		hslot.item=NULL;
	}
}
void LRUCache::sanitize(uint nslot)
{
	sanitize_one_slot(nslot);
	HandleSlot& hslot=lookup_table[nslot];
	uint s=expire_id(hslot.oldsid,next_sid);
	if(s!=hslot.oldsid)
	{
		hslot.oldsid=0;
		if(hslot.newslot!=NULLSLOT&&hslot.newslot!=nslot)
			sanitize_one_slot(hslot.newslot);
		hslot.newslot=NULLSLOT;
	}
}
