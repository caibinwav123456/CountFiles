#ifndef _LRU_CACHE_H_
#define _LRU_CACHE_H_
struct LRUCacheItem
{
	uint idx;
	virtual ~LRUCacheItem(){}
};
class LRUCache
{
	struct HandleSlot
	{
		HandleSlot* prev;
		HandleSlot* next;
		uint sid;
		uint oldsid;
		uint newslot;
		LRUCacheItem* item;
	};
public:
	LRUCache(uint _capacity,void (*_free_item)(LRUCacheItem* item));
	~LRUCache();
	void clear();
	LRUCacheItem* get(void** phandle);
	void put(LRUCacheItem* item,void** phandle);
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
	void (*free_item)(LRUCacheItem* item);
	uint next_sid;
};
#endif
