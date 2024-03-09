#ifndef _COMMON_H_
#define _COMMON_H_
#ifndef NULL
#define NULL 0
#endif
#define VALID(M) ((M)==0?false:((M)==(void*)-1?((M)=0,false):true))
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long dword;
#ifdef CONFIG_X64
#define ptr_to_uint(ptr) ((uint)(signed int)(signed long long)(ptr))
#define uint_to_ptr(n) ((void*)(signed long long)(signed int)(n))
#else
#define ptr_to_uint(ptr) ((uint)(ptr))
#define uint_to_ptr(n) ((void*)(n))
#endif
#define hiword(x) ((x)>>16)
#define loword(x) ((x)&0xffff)
#define fail_op(ret,retok,expression,op) \
	if((ret=expression)!=retok) \
		op
#define fail_op_no_retval(retok,expression,op) \
	if((expression)!=retok) \
		op
#define fail_return(retok,expression) fail_op_no_retval(retok,expression,return)
#define fail_goto(ret,retok,expression,tag) \
	fail_op(ret,retok,expression,goto tag)
#define return_ret_val(ret,retok,expression,retval) \
	fail_op(ret,retok,expression,return retval)
#define return_ret(ret,retok,expression) return_ret_val(ret,retok,expression,ret)
#if (defined(DEBUG) || defined(_DEBUG)) && !defined(NDEBUG)
#define verify(m) assert(m)
#else
#define verify(m) (m)
#endif
#if defined(WIN32) || defined(WINDOWS) || defined(_WINDOWS)
#define BUILD_ON_WINDOWS
#pragma warning(disable:4996)
#pragma warning(disable:4251)
#pragma warning(disable:4244)
#pragma warning(disable:4267)
#ifndef DLL_IMPORT
#define DLL __declspec(dllexport)
#else
#define DLL __declspec(dllimport)
#endif
#define DLLAPI(T) DLL T __stdcall
#define main_entry WINAPI _tWinMain
#define main_args HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nShowCmd
#else
#ifndef DLL_IMPORT
#define DLL __attribute__((visibility("default")))
#else
#define DLL
#endif
#define DLLAPI(T) DLL T
#define main_entry main
#define main_args int argc, char** argv
#endif
#include "ASTError.h"
#include "defines.h"
#include "sys.h"
#endif