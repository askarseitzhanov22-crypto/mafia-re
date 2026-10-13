#ifndef MOB_WINDOWS_H
#define MOB_WINDOWS_H

#include <stdint.h>
#include <stdio.h>

// Базовые типы данных Windows
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

// Зануляем Windows-специфичные макросы
#define __declspec(x)
#define dllexport
#define dllimport
#define __stdcall
#define __cdecl
#define __fastcall
#define EXPORT

// Исправление неизвестных типов из логов
typedef unsigned int UINT;
typedef long LONG;
typedef unsigned long ULONG;
typedef unsigned long ulong;

// Константы файловой системы Windows из логов
#define MAX_PATH 260
#define GENERIC_READ 0x80000000
#define FILE_SHARE_READ 1
#define OPEN_EXISTING 3
#define FILE_ATTRIBUTE_NORMAL 0x80
#define FILE_FLAG_RANDOM_ACCESS 0x10000000
#define INVALID_HANDLE_VALUE ((HANDLE)(long long)-1)
#define FILE_BEGIN 0
#define NO_ERROR 0L

// Заглушки функций Windows для работы с файлами на Android
inline HANDLE CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, void* lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
    return (HANDLE)fopen(lpFileName, "rb");
}

inline BOOL ReadFile(HANDLE hFile, void* lpBuffer, DWORD nNumberOfBytesToRead, DWORD* lpNumberOfBytesRead, void* lpOverlapped) {
    if (!hFile || hFile == INVALID_HANDLE_VALUE) return FALSE;
    size_t read = fread(lpBuffer, 1, nNumberOfBytesToRead, (FILE*)hFile);
    if (lpNumberOfBytesRead) *lpNumberOfBytesRead = (DWORD)read;
    return (read > 0) ? TRUE : FALSE;
}

inline DWORD SetFilePointer(HANDLE hFile, long lDistanceToMove, void* lpDistanceToMoveHigh, DWORD dwMoveMethod) {
    if (!hFile || hFile == INVALID_HANDLE_VALUE) return (DWORD)-1;
    fseek((FILE*)hFile, lDistanceToMove, SEEK_SET);
    return (DWORD)ftell((FILE*)hFile);
}

inline DWORD GetLastError() {
    return NO_ERROR;
}

#endif // MOB_WINDOWS_H
