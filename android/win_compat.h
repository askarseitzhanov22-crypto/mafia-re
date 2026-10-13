#pragma once

// Minimal Win32 compatibility layer for the Android/ARM64 build.
// The original Windows build is intentionally left untouched.

#include <cerrno>
#include <cstdint>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>
#include <pthread.h>
#include <limits.h>

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

typedef int HANDLE;
typedef uint32_t DWORD;
typedef uint32_t UINT;
typedef int BOOL;
typedef uint64_t ULONGLONG;
typedef long LONG;
struct FILETIME { uint32_t dwLowDateTime; uint32_t dwHighDateTime; };
typedef FILETIME *LPFILETIME;

#define TRUE 1
#define FALSE 0
#define INVALID_HANDLE_VALUE (-1)
#define INVALID_SET_FILE_POINTER ((DWORD)-1)

#define GENERIC_READ  0x80000000u
#define GENERIC_WRITE 0x40000000u
#define FILE_SHARE_READ  0x00000001u
#define FILE_SHARE_WRITE 0x00000002u
#define OPEN_EXISTING 3
#define CREATE_ALWAYS 2
#define OPEN_ALWAYS 4
#define FILE_ATTRIBUTE_NORMAL 0x00000080u
#define FILE_ATTRIBUTE_HIDDEN 0x00000002u
#define FILE_ATTRIBUTE_READONLY 0x00000001u
#define FILE_FLAG_RANDOM_ACCESS 0x10000000u
#define FILE_BEGIN SEEK_SET
#define FILE_CURRENT SEEK_CUR
#define FILE_END SEEK_END
#define NO_ERROR 0
#define MB_OK 0

struct CRITICAL_SECTION { pthread_mutex_t mutex; };

static inline void InitializeCriticalSection(CRITICAL_SECTION *s) { pthread_mutex_init(&s->mutex, 0); }
static inline void DeleteCriticalSection(CRITICAL_SECTION *s) { pthread_mutex_destroy(&s->mutex); }
static inline void EnterCriticalSection(CRITICAL_SECTION *s) { pthread_mutex_lock(&s->mutex); }
static inline void LeaveCriticalSection(CRITICAL_SECTION *s) { pthread_mutex_unlock(&s->mutex); }

static inline DWORD GetLastError() { return (DWORD)errno; }

static inline HANDLE CreateFileA(const char *name, DWORD access, DWORD, void *, DWORD disposition, DWORD, void *)
{
    int flags = 0;
    if ((access & GENERIC_WRITE) != 0) flags = O_RDWR;
    else flags = O_RDONLY;
    if (disposition == CREATE_ALWAYS) flags |= O_CREAT | O_TRUNC;
    else if (disposition == OPEN_ALWAYS) flags |= O_CREAT;
    return open(name, flags, 0666);
}

static inline BOOL CloseHandle(HANDLE h) { return close(h) == 0; }

static inline BOOL ReadFile(HANDLE h, void *buffer, DWORD len, DWORD *read_count, void *)
{
    ssize_t n = read(h, buffer, len);
    if (n < 0) { if (read_count) *read_count = 0; return FALSE; }
    if (read_count) *read_count = (DWORD)n;
    return TRUE;
}

static inline BOOL WriteFile(HANDLE h, const void *buffer, DWORD len, DWORD *written_count, void *)
{
    ssize_t n = write(h, buffer, len);
    if (n < 0) { if (written_count) *written_count = 0; return FALSE; }
    if (written_count) *written_count = (DWORD)n;
    return TRUE;
}

static inline DWORD SetFilePointer(HANDLE h, LONG distance, LONG *, DWORD method)
{
    off_t p = lseek(h, (off_t)distance, (int)method);
    return p < 0 ? INVALID_SET_FILE_POINTER : (DWORD)p;
}

static inline BOOL DeleteFileA(const char *name) { return unlink(name) == 0; }
static inline BOOL MoveFileA(const char *from, const char *to) { return rename(from, to) == 0; }
static inline BOOL CopyFileA(const char *from, const char *to, BOOL fail_if_exists)
{
    if (fail_if_exists && access(to, F_OK) == 0) return FALSE;
    int in = open(from, O_RDONLY);
    if (in < 0) return FALSE;
    int out = open(to, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (out < 0) { close(in); return FALSE; }
    char buf[16384];
    bool ok = true;
    for (;;) {
        ssize_t n = read(in, buf, sizeof(buf));
        if (n == 0) break;
        if (n < 0) { ok = false; break; }
        char *p = buf;
        while (n > 0) {
            ssize_t w = write(out, p, (size_t)n);
            if (w <= 0) { ok = false; break; }
            p += w; n -= w;
        }
        if (!ok) break;
    }
    close(in); close(out);
    if (!ok) unlink(to);
    return ok ? TRUE : FALSE;
}

static inline DWORD GetFileAttributesA(const char *name)
{
    struct stat st;
    if (stat(name, &st) != 0) return (DWORD)-1;
    DWORD a = FILE_ATTRIBUTE_NORMAL;
    if ((st.st_mode & S_IWUSR) == 0) a |= FILE_ATTRIBUTE_READONLY;
    return a;
}

static inline BOOL SetFileAttributesA(const char *name, DWORD attributes)
{
    struct stat st;
    if (stat(name, &st) != 0) return FALSE;
    mode_t mode = st.st_mode;
    if (attributes & FILE_ATTRIBUTE_READONLY) mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
    else mode |= S_IWUSR;
    return chmod(name, mode) == 0;
}

// Windows FILETIME is 100-ns ticks since 1601-01-01. Android/Linux stat time is Unix epoch.
static inline void unix_ns_to_filetime(int64_t sec, long nsec, FILETIME *out)
{
    const uint64_t EPOCH_DIFF = 11644473600ULL;
    uint64_t ticks = (uint64_t)(sec + (int64_t)EPOCH_DIFF) * 10000000ULL + (uint64_t)(nsec / 100);
    out->dwLowDateTime = (uint32_t)ticks;
    out->dwHighDateTime = (uint32_t)(ticks >> 32);
}

static inline BOOL GetFileTime(HANDLE h, LPFILETIME creation, LPFILETIME access, LPFILETIME write_time)
{
    struct stat st;
    if (fstat(h, &st) != 0) return FALSE;
    if (creation) unix_ns_to_filetime(st.st_ctime, 0, creation);
    if (access) unix_ns_to_filetime(st.st_atime, 0, access);
    if (write_time) unix_ns_to_filetime(st.st_mtime, 0, write_time);
    return TRUE;
}

static inline char *strupr(char *s)
{
    for (char *p = s; p && *p; ++p) {
        if (*p >= 'a' && *p <= 'z') *p = (char)(*p - 'a' + 'A');
    }
    return s;
}

static inline int wsprintfA(char *dst, const char *fmt, ...)
{
    va_list ap; va_start(ap, fmt);
    int r = vsnprintf(dst, INT_MAX, fmt, ap);
    va_end(ap);
    return r;
}

static inline int MessageBoxA(void *, const char *text, const char *, unsigned int)
{
    return fprintf(stderr, "%s\n", text ? text : "") >= 0 ? 1 : 0;
}
