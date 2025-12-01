#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

_Success_(return) EXTERN_C BOOL WINAPI
    TimeConvertFileTimeToLocalFileTime(_In_ CONST FILETIME *lpFileTime, _Out_ LPFILETIME lpLocalFileTime);
_Success_(return) EXTERN_C BOOL WINAPI
    TimeConvertLocalFileTimeToFileTime(_In_ CONST FILETIME *lpLocalFileTime, _Out_ LPFILETIME lpFileTime);

// Output values are for use with UnsignedMultiplyExtract128
_Success_(return) EXTERN_C BOOL InitTimestampConverterFixedShift(
    _Out_ UINT64 *Mul,
    _In_range_(0, 64) LONG Shift,
    _In_ UINT64 SourceFrequency,
    _In_ UINT64 DestFrequency);

// Output values are for use with UnsignedMultiplyExtract128
_Success_(return) EXTERN_C BOOL InitTimestampConverter(
    _Out_ UINT64 *Mul,
    _Out_ LONG *Shift,
    _In_ UINT64 SourceFrequency,
    _In_ UINT64 DestFrequency);
