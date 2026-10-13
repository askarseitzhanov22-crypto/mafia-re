#ifndef MOB_WINDOWS_H
#define MOB_WINDOWS_H

// Обманка для компилятора: притворяемся Windows средой на Android
typedef unsigned long DWORD;
typedef int BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef float FLOAT;
typedef void* HANDLE;
typedef void* HWND;
typedef const char* LPCSTR;
typedef char* LPSTR;
typedef long long INT_PTR, *PINT_PTR;
typedef unsigned long long UINT_PTR, *PUINT_PTR;
typedef long long LONG_PTR, *PLONG_PTR;

#define FALSE 0
#define TRUE 1
#define WINAPI

#endif // MOB_WINDOWS_H
