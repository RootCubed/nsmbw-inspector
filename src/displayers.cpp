#include "displayers.h"

void basicTypeU32_display(std::string name, baseTypeStruct d) {
    u32 val = _byteswap_ulong(d.data.us32);
    if (ImGui::InputScalar(name.c_str(), ImGuiDataType_U32, &val)) {
        DolphinReader::writeU32(d.addr, val);
    }
}
void basicTypePtr_display(std::string name, baseTypeStruct d) {
    u32 val = _byteswap_ulong(d.data.us32);
    if (ImGui::InputScalar(name.c_str(), ImGuiDataType_U32, &val, NULL, NULL, "0x%08X", ImGuiInputTextFlags_CharsHexadecimal)) {
        DolphinReader::writeU32(d.addr, val);
    }
}
void basicTypeS32_display(std::string name, baseTypeStruct d) {
    s32 val = _byteswap_ulong(d.data.si32);
    if (ImGui::InputScalar(name.c_str(), ImGuiDataType_S32, &val)) {
        DolphinReader::writeU32(d.addr, val);
    }
}
void basicTypeU16_display(std::string name, baseTypeStruct d) {
    u16 val = _byteswap_ushort(d.data.us16);
    if (ImGui::InputScalar(name.c_str(), ImGuiDataType_U16, &val)) {
        DolphinReader::writeU16(d.addr, val);
    }
}
void basicTypeS16_display(std::string name, baseTypeStruct d) {
    s16 val = _byteswap_ushort(d.data.si16);
    if (ImGui::InputScalar(name.c_str(), ImGuiDataType_U16, &val)) {
        DolphinReader::writeU16(d.addr, val);
    }
}
void basicTypeS16Ang_display(std::string name, baseTypeStruct d) {
    s16 val = _byteswap_ushort(d.data.si16);
    float num = (float) val / 0xFFFF * 360;
    if (ImGui::InputScalar(name.c_str(), ImGuiDataType_Float, &num, 0, 0, "%.2fdeg")) {
        s16 newVal = round(num / 360.0f * 0xFFFF);
        DolphinReader::writeU16(d.addr, newVal);
    }
}
void basicTypeU8_display(std::string name, baseTypeStruct d) {
    u8 val = d.data.us8;
    if (ImGui::InputScalar(name.c_str(), ImGuiDataType_U8, &val)) {
        DolphinReader::writeU8(d.addr, val);
    }
}
void basicTypeS8_display(std::string name, baseTypeStruct d) {
    s8 val = d.data.si8;
    if (ImGui::InputScalar(name.c_str(), ImGuiDataType_S8, &val)) {
        DolphinReader::writeU8(d.addr, val);
    }
}
void basicTypeFloat_display(std::string name, baseTypeStruct d) {
    u32 tmp = _byteswap_ulong(d.data.us32);
    float val = *(float *) &tmp;
    if (ImGui::InputFloat(name.c_str(), &val)) {
        DolphinReader::writeFloat(d.addr, val);
    }
}
void basicTypeStr_display(std::string name, baseTypeStruct d) {
    char strBuf[256];
    u32 tmpAddr = _byteswap_ulong(d.data.us32);
    void *tmp = DolphinReader::readValues(tmpAddr, 256);
    memcpy(strBuf, tmp, 256);
    ImGui::InputText(name.c_str(), strBuf, 256);
    // TODO: implement writing (-> create new buffer or modify actual buffer?)
}

void basicTypeJIS_display(std::string name, baseTypeStruct d) {
    char strBuf[256];
    u32 tmpAddr = _byteswap_ulong(d.data.us32);
    char *tmp = (char *) DolphinReader::readValues(tmpAddr, 256);
    strcpy(strBuf, "ShiftJIS strings not supported");
    ImGui::InputText(name.c_str(), strBuf, 256);
}