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
typedef const wchar_t* LPCWSTR;
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

// Исправление типов
typedef unsigned int UINT;
typedef long LONG;
typedef unsigned long ULONG;
typedef unsigned long ulong;

// Константы файловой системы Windows
#define MAX_PATH 260
#define GENERIC_READ 0x80000000
#define FILE_SHARE_READ 1
#define OPEN_EXISTING 3
#define CREATE_ALWAYS 2
#define OPEN_ALWAYS 4
#define FILE_ATTRIBUTE_NORMAL 0x80
#define FILE_ATTRIBUTE_READONLY 1
#define FILE_ATTRIBUTE_HIDDEN 2
#define FILE_FLAG_RANDOM_ACCESS 0x10000000
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#define FILE_BEGIN 0
#define FILE_CURRENT 1
#define FILE_END 2
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
    if (!hFile || hFile == INVALID_HANDLE_VALUE) return INVALID_SET_FILE_POINTER;
    int origin = SEEK_SET;
    if (dwMoveMethod == FILE_CURRENT) origin = SEEK_CUR;
    if (dwMoveMethod == FILE_END) origin = SEEK_END;
    
    if (fseek((FILE*)hFile, lDistanceToMove, origin) != 0) return INVALID_SET_FILE_POINTER;
    return (DWORD)ftell((FILE*)hFile);
}

inline BOOL CloseHandle(HANDLE hObject) {
    if (hObject && hObject != INVALID_HANDLE_VALUE) {
        fclose((FILE*)hObject);
        return TRUE;
    }
    return FALSE;
}

inline BOOL DeleteFileA(LPCSTR lpFileName) {
    return remove(lpFileName) == 0;
}

inline BOOL DeleteFileW(LPCWSTR lpFileName) {
    return FALSE; // В Android не используется wchar_t для путей
}

inline DWORD GetFileAttributesA(LPCSTR lpFileName) {
    return FILE_ATTRIBUTE_NORMAL;
}

inline BOOL SetFileAttributesA(LPCSTR lpFileName, DWORD dwFileAttributes) {
    return TRUE;
}

inline BOOL MoveFileA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName) {
    return rename(lpExistingFileName, lpNewFileName) == 0;
}

inline DWORD GetLastError() {
    return NO_ERROR;
}

#endif // MOB_WINDOWS_H
