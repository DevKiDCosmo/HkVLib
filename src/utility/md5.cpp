#include "utility.h"

String Utility::toHex(const uint8_t *data, size_t len)
{
    static const char *hex = "0123456789abcdef";
    String out;
    out.reserve(len * 2);
    for (size_t i = 0; i < len; ++i)
    {
        out += hex[(data[i] >> 4) & 0x0F];
        out += hex[data[i] & 0x0F];
    }
    return out;
}