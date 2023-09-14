#ifndef _UTILITY_H_
#define _UTILITY_H_
#include "common.h"
#include <vector>
#include <string>
#include <algorithm>
#include "Integer64.h"
using namespace std;
DLL int compare_pathname(const string& a,const string& b);
DLL string format_segmented_u64(const UInteger64& u64);
enum E_MERGE_SIDE
{
	eMSBoth,
	eMSLeft,
	eMSRight,
};
template<class T>
struct iterator_base
{
	typename vector<T>::iterator it;
	typename vector<T>::iterator end;
	iterator_base(vector<T>& v)
	{
		it=v.begin();
		end=v.end();
	}
	operator bool()
	{
		return it!=end;
	}
	void operator++(int)
	{
		if(it<end)
			it++;
	}
};
#define err_return(ret,proc) {if(0!=(ret=proc))return ret;}
#define cur_left_valid() (bleft?(bool)l1:(bool)l2)
#define cur_right_valid() (bleft?(bool)l2:(bool)l1)
#define cur_left_next() (bleft?l1++:l2++)
#define cur_right_next() (bleft?l2++:l1++)
#define cur_left() (bleft?*l1:*l2)
#define cur_right() (bleft?*l2:*l1)
template<class iteratorT1,class iteratorT2>
int merge_ordered_list(iteratorT1 l1,iteratorT2 l2,int (*cb)(iteratorT1,iteratorT2,E_MERGE_SIDE,void*),void* param)
{
	int ret=0;
	bool bleft=true;
	for(;;)
	{
		for(;cur_left_valid();)
		{
			if((!cur_right_valid())||cur_left()<cur_right())
			{
				err_return(ret,cb(l1,l2,bleft?eMSLeft:eMSRight,param))
				cur_left_next();
			}
			else if(!(cur_right()<cur_left()))
			{
				err_return(ret,cb(l1,l2,eMSBoth,param))
				l1++,l2++;
			}
			else
				break;
		}
		if(!(l1||l2))
			break;
		bleft=!bleft;
	}
	return 0;
}
#endif