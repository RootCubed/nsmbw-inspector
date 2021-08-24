#pragma once

#include "Dolphin-memory-engine/Source/DolphinProcess/DolphinAccessor.h"
#include "Dolphin-memory-engine/Source/Common/CommonUtils.h"

#define MEMBUF_SIZE 0x100000

class DolphinReader {
    public:
    static DolphinComm::DolphinAccessor::DolphinStatus hook();

    static u32 readU32(u32 address);
    static u16 readU16(u16 address);
    static u8 readU16(u8 address);
    static float readFloat(u32 address);
    static void *readValues(u32 address, u16 size);

    static void writeU32(u32 address, u32 value);
    static void writeU16(u32 address, u16 value);
    static void writeU8(u32 address, u8 value);
    static void writeFloat(u32 address, float value);
    static void writeValues(u32 address, void *data, u16 size);
};