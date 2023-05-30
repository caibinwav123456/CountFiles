#ifndef _LRU_CACHE_H_
#define _LRU_CACHE_H_
class LRUCache
{
	struct HandleSlot
	{
		HandleSlot* prev;
		HandleSlot* next;
		uint sid;
		uint oldsid;
		uint newslot;
		void* item;
	};
public:
	LRUCache(uint _capacity,void (*_free_item)(void*));
	~LRUCache();
	void clear();
	void* get(void** phandle);
	void put(void* item,void** phandle);
private:
	void move_to_front(HandleSlot* slot);
	void add_to_front(HandleSlot* slot);
	void add_to_free(HandleSlot* slot);
	void remove(HandleSlot* slot);
	void sanitize(uint nslot);
	void sanitize_one_slot(uint nslot);
	HandleSlot head;
	HandleSlot tail;
	HandleSlot free_head;
	HandleSlot free_tail;
	uint size;
	uint capacity;
	HandleSlot* lookup_table;
	void (*free_item)(void*);
	uint next_sid;
};
#endif
