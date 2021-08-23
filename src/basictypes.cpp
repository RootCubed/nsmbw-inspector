#include "basictypes.h"
#include <sstream>
#include <cmath>

// ----------------
//    base type
// ----------------

BasicType::BasicType(int s = 0, std::string t = "") {
    typeSize = s;
    typeName = t;
}

void BasicType::display(std::string name, baseTypeStruct d) {
    char buf[256];
    snprintf(buf, 256, "[%s].display not found.", typeName.c_str());
    ImGui::InputText(name.c_str(), buf, 256, ImGuiInputTextFlags_ReadOnly);
}

std::string BasicType::preview(baseTypeStruct d) {
    char buf[256];
    snprintf(buf, 256, "[%s].preview not found.", typeName.c_str());
    return std::string(buf);
}

// ----------------
//       u32
// ----------------

void BasicTypeU32::display(std::string name, baseTypeStruct d) {
    u32 val = _byteswap_ulong(d.data.us32);
    if (ImGui::InputScalar(name.c_str(), ImGuiDataType_U32, &val)) {
        DolphinReader::writeU32(d.addr, val);
    }
}
std::string BasicTypeU32::preview(baseTypeStruct d) {
    u32 val = _byteswap_ulong(d.data.us32);
    return std::to_string(val);
}

// ----------------
//       ptr
// ----------------

void BasicTypePtr::display(std::string name, baseTypeStruct d) {
    u32 val = _byteswap_ulong(d.data.us32);
    if (ImGui::InputScalar(name.c_str(), ImGuiDataType_U32, &val, NULL, NULL, "0x%08X", ImGuiInputTextFlags_CharsHexadecimal)) {
        DolphinReader::writeU32(d.addr, val);
    }
}
std::string BasicTypePtr::preview(baseTypeStruct d) {
    u32 val = _byteswap_ulong(d.data.us32);
    std::stringstream stream;
    stream << "0x" << std::hex << val;
    return stream.str();
}

// ----------------
//       s32
// ----------------

void BasicTypeS32::display(std::string name, baseTypeStruct d) {
    s32 val = _byteswap_ulong(d.data.si32);
    if (ImGui::InputScalar(name.c_str(), ImGuiDataType_S32, &val)) {
        DolphinReader::writeU32(d.addr, val);
    }
}
std::string BasicTypeS32::preview(baseTypeStruct d) {
    s32 val = _byteswap_ushort(d.data.si32);
    return std::to_string(val);
}

// ----------------
//       u16
// ----------------

void BasicTypeU16::display(std::string name, baseTypeStruct d) {
    u16 val = _byteswap_ushort(d.data.us16);
    if (ImGui::InputScalar(name.c_str(), ImGuiDataType_U16, &val)) {
        DolphinReader::writeU16(d.addr, val);
    }
}
std::string BasicTypeU16::preview(baseTypeStruct d) {
    u16 val = _byteswap_ushort(d.data.us16);
    return std::to_string(val);
}

// ----------------
//       s16
// ----------------

void BasicTypeS16::display(std::string name, baseTypeStruct d) {
    s16 val = _byteswap_ushort(d.data.si16);
    if (ImGui::InputScalar(name.c_str(), ImGuiDataType_U16, &val)) {
        DolphinReader::writeU16(d.addr, val);
    }
}
std::string BasicTypeS16::preview(baseTypeStruct d) {
    s16 val = _byteswap_ushort(d.data.si16);
    return std::to_string(val);
}

// ----------------
//     s16angle
// ----------------

void BasicTypeS16Ang::display(std::string name, baseTypeStruct d) {
    s16 val = _byteswap_ushort(d.data.si16);
    float num = (float) val / 0xFFFF * 360;
    if (ImGui::InputScalar(name.c_str(), ImGuiDataType_Float, &num, 0, 0, "%.2fdeg")) {
        s16 newVal = std::round(num / 360.0f * 0xFFFF);
        DolphinReader::writeU16(d.addr, newVal);
    }
}
std::string BasicTypeS16Ang::preview(baseTypeStruct d) {
    s16 val = _byteswap_ushort(d.data.si16);
    float num = (float) val / 0xFFFF * 360;
    return std::to_string(num) + "°";
}

// ----------------
//       u8
// ----------------

void BasicTypeU8::display(std::string name, baseTypeStruct d) {
    u8 val = d.data.us8;
    if (ImGui::InputScalar(name.c_str(), ImGuiDataType_U8, &val)) {
        DolphinReader::writeU8(d.addr, val);
    }
}
std::string BasicTypeU8::preview(baseTypeStruct d) {
    u8 val = d.data.us8;
    return std::to_string(val);
}

// ----------------
//       s8
// ----------------

void BasicTypeS8::display(std::string name, baseTypeStruct d) {
    s8 val = d.data.si8;
    if (ImGui::InputScalar(name.c_str(), ImGuiDataType_S8, &val)) {
        DolphinReader::writeU8(d.addr, val);
    }
}
std::string BasicTypeS8::preview(baseTypeStruct d) {
    s8 val = d.data.si8;
    return std::to_string(val);
}

// ----------------
//      float
// ----------------

void BasicTypeFloat::display(std::string name, baseTypeStruct d) {
    u32 tmp = _byteswap_ulong(d.data.us32);
    float val = *(float *) &tmp;
    if (ImGui::InputFloat(name.c_str(), &val)) {
        DolphinReader::writeFloat(d.addr, val);
    }
}
std::string BasicTypeFloat::preview(baseTypeStruct d) {
    u32 tmp = _byteswap_ulong(d.data.us32);
    float val = *(float *) &tmp;
    return std::to_string(val);
}

// ----------------
//     string
// ----------------

void BasicTypeStr::display(std::string name, baseTypeStruct d) {
    char strBuf[256];
    u32 tmpAddr = _byteswap_ulong(d.data.us32);
    void *tmp = DolphinReader::readValues(tmpAddr, 256);
    memcpy(strBuf, tmp, 256);
    ImGui::InputText(name.c_str(), strBuf, 256);
    // TODO: implement writing (-> create new buffer or modify actual buffer?)
}
std::string BasicTypeStr::preview(baseTypeStruct d) {
    char strBuf[256];
    u32 tmpAddr = _byteswap_ulong(d.data.us32);
    void *tmp = DolphinReader::readValues(tmpAddr, 256);
    memcpy(strBuf, tmp, 256);
    return strBuf;
}

// ----------------
//    stringJIS
// ----------------

void BasicTypeJIS::display(std::string name, baseTypeStruct d) {
    char strBuf[256];
    u32 tmpAddr = _byteswap_ulong(d.data.us32);
    char *tmp = (char *) DolphinReader::readValues(tmpAddr, 256);
    strcpy(strBuf, "[stringJIS]");
    ImGui::InputText(name.c_str(), strBuf, 256);
}
