#ifndef _UTILITY_H_
#define _UTILITY_H_
#include "common.h"
#include <vector>
#include <string>
#include <algorithm>
using namespace std;
DLL int compare_pathname(const string& a,const string& b);
enum E_MERGE_SIDE
{
	eMSBoth,
	eMSLeft,
	eMSRight,
};
#define err_return(ret,proc) {if(0!=(ret=proc))return ret;}
template<class iteratorT>
int merge_ordered_list(iteratorT l1,iteratorT l2,int (*cb)(iteratorT,iteratorT,E_MERGE_SIDE,void*),void* param)
{
	int ret=0;
	iteratorT *it1=&l1,*it2=&l2;
	bool bleft=true;
	for(;;)
	{
		for(;*it1;(*it1)++)
		{
			if(!*it2)
				err_return(ret,cb(l1,l2,eMSLeft,param))
			else if(**it1<**it2)
				err_return(ret,cb(l1,l2,bleft?eMSLeft:eMSRight,param))
			else if(!(**it2<**it1))
				err_return(ret,cb(l1,l2,eMSBoth,param))
			else
				break;
		}
		if(!(*it1||*it2))
			break;
		swap(it1,it2);
		bleft=!bleft;
	}
	return 0;
}
#endif