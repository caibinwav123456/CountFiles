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
#define hiptr(ptr) ((uint)(((unsigned long long)(ptr))>>32))
#define loptr(ptr) ((uint)(((unsigned long long)(ptr))&0xffffffff))
#define mkptr(hi,lo) ((void*)((((unsigned long long)(uint)(hi))<<32)|(((unsigned long long)(uint)(lo))&0xffffffff)))
#define next_id(id) ((id)+1==0?1:(id)+1)
#else
#define ptr_to_uint(ptr) ((uint)(ptr))
#define uint_to_ptr(n) ((void*)(n))
#define hiptr(ptr) ((uint)(((unsigned long long)(ptr))>>16))
#define loptr(ptr) ((uint)(((unsigned long long)(ptr))&0xffff))
#define mkptr(hi,lo) ((void*)(((hi)<<16)|((lo)&0xffff)))
#define next_id(id) ((uint)((unsigned short)(id)+(unsigned short)1)==0? \
					1:(uint)((unsigned short)(id)+(unsigned short)1))
#endif
#define hiword(x) ((x)>>16)
#define loword(x) ((x)&0xffff)
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
#include "sys.h"
#include "defines.h"
#endif