#include "common.h"
#include "LRUCache.h"
#include <stdio.h>
#include <string.h>
#define NULLSLOT ((uint)-1)
LRUCache::LRUCache(uint _capacity,void (*_free_item)(LRUCacheItem* item))
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
	lookup_table=new HandleSlot[capacity];
	memset(lookup_table,0,capacity*sizeof(HandleSlot));
	for(int i=0;i<(int)capacity;i++)
	{
		lookup_table[i].newslot=NULLSLOT;
	}
	head.idx=0;
	tail.idx=0;
	head.next=&tail;
	head.prev=NULL;
	tail.prev=&head;
	tail.next=NULL;
}
LRUCache::~LRUCache()
{
	clear();
	delete[] lookup_table;
}
void LRUCache::clear()
{
	head.idx=0;
	tail.idx=0;
	head.next=&tail;
	tail.prev=&head;
	if(size==0)
		return;
	size=0;
	for(int i=0;i<(int)capacity;i++)
	{
		if(lookup_table[i].item!=NULL)
		{
			free_item(lookup_table[i].item);
			memset(&lookup_table[i],0,sizeof(lookup_table[i]));
			lookup_table[i].newslot=NULLSLOT;
		}
	}
}
LRUCacheItem* LRUCache::get(void** phandle)
{
	void*& handle=*phandle;
	if(handle==NULL)
		return NULL;
	uint slot=hiptr(handle);
	if(slot>=size)
		return NULL;
	uint sid=loptr(handle);
	HandleSlot& hslot=lookup_table[slot];
	if(hslot.sid==sid&&hslot.item!=NULL)
	{
		move_to_front(hslot.item);
		return hslot.item;
	}
	else if(hslot.oldsid==sid)
	{
		if(hslot.newslot==NULLSLOT)
			return NULL;
		HandleSlot& newslot=lookup_table[hslot.newslot];
		handle=mkptr(hslot.newslot,newslot.sid);
		move_to_front(newslot.item);
		return newslot.item;
	}
	else
	{
		return NULL;
	}
}
void LRUCache::put(LRUCacheItem* item,void** phandle)
{
	void*& handle=*phandle;
	if(size<capacity)
	{
		uint nslot=(size++);
		HandleSlot& slot=lookup_table[nslot];
		item->idx=nslot;
		slot.item=item;
		slot.sid=next_sid;
		next_sid=next_id(next_sid);
		add_to_front(item);
		handle=mkptr(nslot,slot.sid);
	}
	else
	{
		LRUCacheItem* last=tail.prev;
		uint idx=last->idx;
		HandleSlot& slot=lookup_table[idx];
		if(handle!=NULL)
		{
			uint oldslot=hiptr(handle);
			if(lookup_table[oldslot].oldsid==loptr(handle)
				&&lookup_table[oldslot].newslot==NULLSLOT)
			{
				lookup_table[oldslot].newslot=idx;
			}
		}
		item->idx=idx;
		slot.item=item;
		slot.oldsid=slot.sid;
		slot.newslot=NULLSLOT;
		slot.sid=next_sid;
		next_sid=next_id(next_sid);
		remove(last);
		free_item(last);
		add_to_front(item);
		handle=mkptr(idx,slot.sid);
	}
}
void LRUCache::move_to_front(LRUCacheItem* item)
{
	item->prev->next=item->next;
	item->next->prev=item->prev;
	LRUCacheItem* next=head.next;
	next->prev=item;
	head.next=item;
	item->prev=&head;
	item->next=next;
}
void LRUCache::add_to_front(LRUCacheItem* item)
{
	LRUCacheItem* next=head.next;
	head.next=item;
	next->prev=item;
	item->prev=&head;
	item->next=next;
}
void LRUCache::remove(LRUCacheItem* item)
{
	item->prev->next=item->next;
	item->next->prev=item->prev;
	item->prev=item->next=item;
}
