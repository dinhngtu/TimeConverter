#include <climits>
#include <cstdio>
#include <TimeConverter.h>

int main() {
    UINT64 Mul;
    LONG Shift;

    if (InitTimestampConverter(&Mul, &Shift, 2400000000, 3200000000ull))
        printf("InitTimestampConverter2 %llu %ld\n", Mul, Shift);

    if (InitTimestampConverterFixedShift(&Mul, 48, 2400000000, 3200000000ull))
        printf("InitTimestampConverterFixedShift %llu\n", Mul);

    return 0;
}
