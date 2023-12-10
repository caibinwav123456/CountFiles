#ifndef _LRU_CACHE_H_
#define _LRU_CACHE_H_
#define container_of(ptr,item,container_class) ((container_class*)(((byte*)ptr)- \
	(byte*)&(((container_class*)NULL)->item)))
#define add_to_free(slot) add_to_free_list(&(slot)->itemlru)
#define remove_all(slot) remove_from_list(&(slot)->itemlru);remove_from_list(&(slot)->itemsid)
#define move_to_front_lru(slot) move_to_front(&(slot)->itemlru,&head_lru)
#define move_to_front_sid(slot) move_to_front(&(slot)->itemsid,&head_sid)
#define add_to_front_lru(slot) add_to_front(&(slot)->itemlru,&head_lru)
#define add_to_front_sid(slot) add_to_front(&(slot)->itemsid,&head_sid)
struct listitem
{
	listitem* prev;
	listitem* next;
};
struct HandleSlot
{
	listitem itemlru;
	listitem itemsid;
	uint sid;
	uint oldsid;
	uint newslot;
	void* item;
};
class LRUCache
{
public:
	LRUCache(uint _capacity,void (*_free_item)(void*));
	~LRUCache();
	void clear();
	void* get(void** phandle);
	void put(void* item,void** phandle);
private:
	void move_to_front(listitem* slot,listitem* head);
	void add_to_front(listitem* slot,listitem* head);
	void add_to_free_list(listitem* slot);
	void remove_from_list(listitem* slot);
	void sanitize(uint nslot);
	void sanitize_one_slot(uint nslot);
	listitem head_lru;
	listitem tail_lru;
	listitem head_sid;
	listitem tail_sid;
	listitem head_free;
	listitem tail_free;
	uint size;
	uint capacity;
	HandleSlot* lookup_table;
	void (*free_item)(void*);
	uint next_sid;
};
#endif
