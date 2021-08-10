#pragma once

#ifdef linux
#define _byteswap_ulong __builtin_bswap32
#define _byteswap_ushort(x) ((x) >> 8 || (((x) << 8) & 0xFF00))
#endif