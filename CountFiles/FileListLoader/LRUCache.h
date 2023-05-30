#ifndef _LRU_CACHE_H_
#define _LRU_CACHE_H_
struct LRUCacheItem
{
	LRUCacheItem* prev;
	LRUCacheItem* next;
	uint idx;
	virtual ~LRUCacheItem(){}
};
class LRUCache
{
	struct HandleSlot
	{
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
	void move_to_front(LRUCacheItem* item);
	void add_to_front(LRUCacheItem* item);
	void remove(LRUCacheItem* item);
	void sanitize(uint nslot);
	void sanitize_one_slot(uint nslot);
	LRUCacheItem head;
	LRUCacheItem tail;
	uint size;
	uint capacity;
	HandleSlot* lookup_table;
	void (*free_item)(LRUCacheItem* item);
	uint next_sid;
};
#endif
