#pragma once

#ifdef __ANDROID__
#define EXPORT __attribute__((visibility("default")))
#define EXPORT_CDECL __attribute__((visibility("default")))
#else
#define EXPORT __declspec(dllexport) __stdcall
#define EXPORT_CDECL __declspec(dllexport)
#endif

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned __int64 ulong;
