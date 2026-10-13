#ifndef MOB_WINDOWS_H
#define MOB_WINDOWS_H

// Основные типы данных Windows
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

// Зануляем Windows-специфичные макросы экспорта, которые ломают Android-сборку
#define __declspec(x)
#define dllexport
#define dllimport
#define __stdcall
#define __cdecl
#define __fastcall

// Дополнительные заглушки для совместимости типов в функциях Mafia
typedef unsigned int UINT;
typedef long LONG;
typedef unsigned long ULONG;

#endif // MOB_WINDOWS_H
