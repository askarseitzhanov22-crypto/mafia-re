#pragma once

#define WIN32_LEAN_AND_MEAN
#ifdef __ANDROID__
#include "../android/win_compat.h"
#else
#include <windows.h>
#endif
#include <string.h>
#include <vector>
#include <types.h>

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif

class C_multithrerad_protection
{
public:
    CRITICAL_SECTION section;
    C_multithrerad_protection();
    ~C_multithrerad_protection();
    void Lock() { EnterCriticalSection(&section); }
    void Unlock() { LeaveCriticalSection(&section); }
};

class C_leak_detector
{
public:
    C_leak_detector();
    ~C_leak_detector();
    void AddRef() { ++ref_count; }
    void Release() { --ref_count; }
    void AddObject() { ++obj_count; }
    void ReleaseObject() { --obj_count; }
    void SetDumpMemoryLeaks(bool dump) { dump_memory_leaks = dump; }
private:
    int ref_count;
    int obj_count;
    bool dump_memory_leaks;
};

struct S_DTAHEADER { uint file_count; uint file_table_offset; uint file_table_size; uint unk; };

struct S_FILES_IN
{
    union { uint hash; struct { ushort file_name_check_sum; ushort file_name_length; }; };
    uint header_offset;
    uint data_offset;
    char file_name[16];
};

struct S_FILEHEADER
{
    uint unk1;
    uint unk2;
    ulong timestamp;
    uint size;
    uint block_count;
    uint name_length_flags;
    uint unk3;
};

struct S_HANDLE
{
    HANDLE file;
    __int64 key;
    char file_name[MAX_PATH];
    std::vector<S_FILES_IN> file_table;
};

struct S_WAVHEADER
{
    uint riff_chunk_id;
    uint riff_chunk_size;
    uint riff_type;
    uint format_chunk_id;
    uint format_chunk_size;
    ushort format_tag;
    ushort channel_count;
    uint samples_per_second;
    uint bytes_per_second;
    ushort block_align;
    ushort bits_per_sample;
    uint data_chunk_id;
    uint data_chunk_size;
};

struct S_FILEHANDLE
{
    HANDLE file;
    uint flags;
    ulong timestamp;
    uint size;
    uint data_offset;
    uint block_start_file_pointer;
    uint file_pointer;
    uint block_file_pointer;
    uint block_count;
    uint current_block;
    uint compressed_block_size;
    uint previous_compressed_block_size;
    S_WAVHEADER *wav_header;
    uint block_size;
    __int64 key;
    bool is_block_in_memory;
    bool encrypted;
    char *tmp_file_name;
    uchar *block_content;
};

#define DTA_FROM_FILESYSTEM 0x1
#define DTA_WRITE 0x2
#define DTA_BUFFER 0x4
#define DTA_FAILED_WRITING_TO_BUFFER 0x80
#define DTA_TEMPORARY 0x1
#define DTA_FILE_BUFFER 0x100
#define DTA_CREATE_FILE 0x10000
#define DTA_CHANGE_ATTRIBUTES 0x100000

class C_rw_data
{
public:
    virtual ~C_rw_data() {}
    virtual int AddRef() = 0;
    virtual int Release() = 0;
    virtual bool UnlockPack(__int64 key) = 0;
};

class C_rw_data_inter : public C_rw_data
{
    virtual ~C_rw_data_inter();
public:
    C_rw_data_inter(const char *file_name, bool &is_valid);
    virtual int AddRef();
    virtual int Release();
    virtual bool UnlockPack(__int64 key);
private:
    uint ref_count;
    HANDLE file;
};
typedef class C_rw_data_inter *LPC_rw_data_inter;

extern "C"
{
    LPC_rw_data_inter EXPORT dtaCreate(const char *file_name);
    void EXPORT dtaSetDtaFirstForce();
    int EXPORT dtaOpen(const char *file_name, bool force_dta);
    int EXPORT dtaOpenWrite(const char *file_name, uint flags);
    uint EXPORT dtaRead(int handle, uchar *buffer, uint len);
    int EXPORT dtaWrite(int handle, const uchar *buffer, uint len);
    int EXPORT dtaSeek(int handle, int distance_to_move, int move_method);
    bool EXPORT dtaClose(int handle);
    bool EXPORT dtaDelete(const char *file_name);
    bool EXPORT dtaGetTime(int handle, ulong *time, bool force_check_from_file);
    uint EXPORT dtaBin2Text(char *buffer, uint len);
    void EXPORT dtaDumpMemoryLeaks(bool dump);
}
