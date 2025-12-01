#include <intrin.h>
#include <limits.h>

#include "TimeConverter.h"

typedef enum _TIME_CONVERT_FILE_TIME_DIRECTION {
    TimeConvertUniversalToLocal,
    TimeConvertLocalToUniversal,
} TIME_CONVERT_FILE_TIME_DIRECTION;

_Success_(return) BOOL TimeConvertFileTime(
    _In_ CONST FILETIME *inputFileTime,
    _Out_ LPFILETIME outputFileTime,
    _In_ TIME_CONVERT_FILE_TIME_DIRECTION direction,
    _In_opt_ PDYNAMIC_TIME_ZONE_INFORMATION dynamicTimeZone) {
    SYSTEMTIME inputTime, outputTime;

    if (!FileTimeToSystemTime(inputFileTime, &inputTime))
        return FALSE;

    switch (direction) {
    case TimeConvertUniversalToLocal:
        if (!SystemTimeToTzSpecificLocalTimeEx(dynamicTimeZone, &inputTime, &outputTime))
            return FALSE;
        break;
    case TimeConvertLocalToUniversal:
        if (!TzSpecificLocalTimeToSystemTimeEx(dynamicTimeZone, &inputTime, &outputTime))
            return FALSE;
        break;
    default:
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (!SystemTimeToFileTime(&outputTime, outputFileTime))
        return FALSE;

    return TRUE;
}

_Success_(return) EXTERN_C BOOL WINAPI
    TimeConvertFileTimeToLocalFileTime(_In_ CONST FILETIME *lpFileTime, _Out_ LPFILETIME lpLocalFileTime) {
    return TimeConvertFileTime(lpFileTime, lpLocalFileTime, TimeConvertUniversalToLocal, NULL);
}

_Success_(return) EXTERN_C BOOL WINAPI
    TimeConvertLocalFileTimeToFileTime(_In_ CONST FILETIME *lpLocalFileTime, _Out_ LPFILETIME lpFileTime) {
    return TimeConvertFileTime(lpLocalFileTime, lpFileTime, TimeConvertLocalToUniversal, NULL);
}

static inline void
ShiftLeft64128(_Out_ UINT64 *Low, _Out_ UINT64 *High, _In_ UINT64 Value, _In_range_(0, 64) LONG Shift) {
    if (Shift == 64) {
        *Low = 0;
        *High = Value;
    } else {
        *Low = Value << Shift;
        *High = __shiftleft128(Value, 0, (BYTE)Shift);
    }
}

static inline UCHAR Add128(_Inout_ UINT64 *Low, _Inout_ UINT64 *High, _In_ UINT64 AddendLow, _In_ UINT64 AddendHigh) {
    UCHAR Carry = _addcarry_u64(0, *Low, AddendLow, Low);
    Carry = _addcarry_u64(Carry, *High, AddendHigh, High);
    return Carry;
}

_Success_(return) EXTERN_C BOOL InitTimestampConverterFixedShift(
    _Out_ UINT64 *Mul,
    _In_range_(0, 64) LONG Shift,
    _In_ UINT64 SourceFrequency,
    _In_ UINT64 DestFrequency) {
    UINT64 Temp, TempHigh, Remainder;

    if (!Mul || SourceFrequency == 0 || DestFrequency == 0 || Shift < 0)
        return FALSE;

    ShiftLeft64128(&Temp, &TempHigh, DestFrequency, Shift);
    // Fixed-width rounding factor and overflow checks
    if (Add128(&Temp, &TempHigh, SourceFrequency / 2, 0) || TempHigh >= SourceFrequency)
        return FALSE;

    *Mul = _udiv128(TempHigh, Temp, SourceFrequency, &Remainder);

    return TRUE;
}

_Success_(return) EXTERN_C BOOL InitTimestampConverter(
    _Out_ UINT64 *Mul,
    _Out_ LONG *Shift,
    _In_ UINT64 SourceFrequency,
    _In_ UINT64 DestFrequency) {
    LONG ShiftValue;

    if (!Mul || !Shift || SourceFrequency == 0 || DestFrequency == 0)
        return FALSE;

    for (ShiftValue = 64; ShiftValue > 0; ShiftValue--)
        if (InitTimestampConverterFixedShift(Mul, ShiftValue, SourceFrequency, DestFrequency))
            break;
    if (ShiftValue == 0)
        return FALSE;

    *Shift = ShiftValue;
    return TRUE;
}
