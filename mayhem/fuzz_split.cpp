#include <stdint.h>
#include <stdio.h>
#include <climits>

#include <fuzzer/FuzzedDataProvider.h>
#include "String.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    std::string str = provider.ConsumeRandomLengthString();
    auto vec = provider.ConsumeBytes<char>(1);
    if (vec.size() == 0)
    {
        return 0;
    }
    char delim = vec.at(0);
    bool keep_delim = provider.ConsumeBool();

    str::split(str, delim, keep_delim);

    return 0;
}