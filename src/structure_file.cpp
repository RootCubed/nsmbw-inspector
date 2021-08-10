#include "../DolphinReader/DolphinReader.h"
#include "structure_file.h"

#include <unordered_set>
#include <sstream>

#include "helper.h"


BasicType::BasicType(int s, void (*disp)(std::string, baseTypeStruct)) {
    size = s;
    display = disp;
}

BasicType::BasicType() {
    size = 0;
    display = NULL;
}

void basicTypeU32_display(std::string name, baseTypeStruct d) {
    u32 val = _byteswap_ulong(d.data.us32);
    if (ImGui::InputScalar(name.c_str(), ImGuiDataType_U32, &val)) {
        DolphinReader::writeU32(d.addr, val);
    }
}
void basicTypePtr_display(std::string name, baseTypeStruct d) {
    u32 val = _byteswap_ulong(d.data.us32);
    ImGui::InputScalar(name.c_str(), ImGuiDataType_U32, &val, NULL, NULL, "%08X", ImGuiInputTextFlags_CharsHexadecimal);
}
void basicTypeS32_display(std::string name, baseTypeStruct d) {
    u32 val = _byteswap_ulong(d.data.us32);
    ImGui::InputScalar(name.c_str(), ImGuiDataType_S32, &val);
}
void basicTypeU16_display(std::string name, baseTypeStruct d) {
    u16 val = _byteswap_ushort(d.data.us16);
    ImGui::InputScalar(name.c_str(), ImGuiDataType_U16, &val);
}
void basicTypeS16_display(std::string name, baseTypeStruct d) {
    u16 val = _byteswap_ushort(d.data.us16);
    ImGui::InputScalar(name.c_str(), ImGuiDataType_S16, &val);
}
void basicTypeS16Ang_display(std::string name, baseTypeStruct d) {
    u16 val = _byteswap_ushort(d.data.us16);
    float num = d.data.si16 / 0xFFFF * 360;
    ImGui::InputScalar(name.c_str(), ImGuiDataType_S16, &num, NULL, NULL, "%f°");
}
void basicTypeU8_display(std::string name, baseTypeStruct d) {
    u8 val = d.data.us8;
    ImGui::InputScalar(name.c_str(), ImGuiDataType_U8, &val);
}
void basicTypeS8_display(std::string name, baseTypeStruct d) {
    s8 val = d.data.si8;
    ImGui::InputScalar(name.c_str(), ImGuiDataType_S8, &val);
}
void basicTypeFloat_display(std::string name, baseTypeStruct d) {
    u32 tmp = _byteswap_ulong(d.data.us32);
    float val = *(float *) &tmp;
    ImGui::InputFloat(name.c_str(), &val);
}
char tmp[] = "WIP";
void basicTypeStr_display(std::string name, baseTypeStruct d) {
    ImGui::InputText(name.c_str(), tmp, 3);
}
void basicTypeJIS_display(std::string name, baseTypeStruct d) {
    ImGui::InputText(name.c_str(), tmp, 3);
}
BasicType basicTypeU32    (4, basicTypeU32_display);
BasicType basicTypePtr    (4, basicTypePtr_display);
BasicType basicTypeS32    (4, basicTypeS32_display);
BasicType basicTypeU16    (2, basicTypeU16_display);
BasicType basicTypeS16    (2, basicTypeS16_display);
BasicType basicTypeS16Ang (2, basicTypeS16Ang_display);
BasicType basicTypeU8     (1, basicTypeU8_display);
BasicType basicTypeS8     (1, basicTypeS8_display);
BasicType basicTypeFloat  (4, basicTypeFloat_display);
BasicType basicTypeStr    (4, basicTypeStr_display);
BasicType basicTypeJIS    (4, basicTypeJIS_display);

std::map<std::string, BasicType> basicTypes = {
    {std::string("u32"),       basicTypeU32},
    {std::string("ptr"),       basicTypePtr},
    {std::string("s32"),       basicTypeU32},
    {std::string("u16"),       basicTypeU16},
    {std::string("s16"),       basicTypeS16},
    {std::string("s16angle"),  basicTypeS16Ang},
    {std::string("u8"),        basicTypeU8},
    {std::string("s8"),        basicTypeS8},
    {std::string("float"),     basicTypeFloat},
    {std::string("string"),    basicTypeStr},
    {std::string("stringJIS"), basicTypeJIS}
};

Structure &TempStructure::stru(std::vector<Structure> &vec) const {
    return vec.at(posInVector);
}

StructureInstance::StructureInstance() {
    type = NULL;
    data = std::vector<char>(0);
}

StructureInstance::StructureInstance(Structure *_type, std::vector<char> _data) {
    setType(_type);
    updateData(_data);
}

void StructureInstance::setType(Structure *_type) {
    type = _type;
}

void StructureInstance::updateData(std::vector<char> _data) {
    if (_data.size() < type->size) {
        std::cout << "big oof.";
    }
    data = _data;
}

int StructureInstance::getReadSize() {
    if (type == NULL) return -1;
    Structure *curr = type;
    int size = 0;
    do {
        size += curr->size;
    } while (curr = curr->inherits);
    return size;
}

void StructureInstance::drawInstance(u32 ptr) {
    int rS = getReadSize();
    if (rS > 0) {
        std::vector<char> data(rS, 0);
        void *tmp = DolphinReader::readValues(ptr, rS);
        std::memcpy(&data[0], tmp, rS);
        updateData(data);
    } else {
        printf("data read error: readSize <= 0\n");
    }
    Structure *curr = type;
    ImGui::BeginTabBar("Structs");
    do {
        if (ImGui::BeginTabItem(curr->name.c_str())) {
            for (auto &field : curr->fields) {
                if (field.second.isBasic) {
                    baseTypeStruct thisData;
                    thisData.addr = ptr + field.first;
                    std::memcpy(thisData.data.binary, &data[0] + field.first, field.second.ptr.base->size);
                    field.second.ptr.base->display(field.second.name, thisData);
                }
            }
            ImGui::EndTabItem();
        }
    } while (curr = curr->inherits);
    ImGui::EndTabBar();
}

StructureFile::StructureFile() {
    structs = std::vector<Structure>(0);
}

int decHexToInt(std::string str) {
    if (str.rfind("0x", 0) == 0) {
        return std::stoi(str, 0, 16);
    }
    return std::stoi(str, 0, 10);
}

StructureFile::StructureFile(std::string fileWithLineBreaks) {
    structs = std::vector<Structure>(0);
    printf("loading file...");

    std::regex whitespace("\\n|\\r");

    std::string file = std::regex_replace(fileWithLineBreaks, whitespace, "");

    parseStructureBlocks(file);

    // todo: preview, display, object
}

void StructureFile::parseStructureBlocks(std::string file) {
    std::regex structureBlockRegex("structure ([^:]+):([^:]+):([^\\{ ]+) *\\{([^\\}]*)\\}", std::regex::ECMAScript);
    std::smatch structureBlock;
    
    std::map<std::string, TempStructure> tmpStructures;

    std::string::const_iterator structureBlocksStart(file.cbegin());    
    while (std::regex_search(structureBlocksStart, file.cend(), structureBlock, structureBlockRegex)) {
        std::string name = structureBlock[1].str();
        std::string inherit = structureBlock[2].str();
        std::string size = structureBlock[3].str();
        std::string content = structureBlock[4].str();
        
        Structure s = Structure();
        s.name = name;
        s.size = -1; // will be changed later to the full size including inherited classes
        structs.push_back(s);

        TempStructure tempS;
        tempS.posInVector = structs.size() - 1;
        tempS.localSize = decHexToInt(size);
        tempS.content = content;
        tempS.inherit = inherit;

        if (tmpStructures.contains(name)) {
            std::ostringstream msg;
            msg << "Duplicate structure name \"" << name << "\"";
            throw StructureFileException(msg.str());
        }
        tmpStructures[name] = tempS;

        structureBlocksStart = structureBlock.suffix().first;
    }

    // test for self-inheriting structs
    for (auto &stru : tmpStructures) {
        std::unordered_set<std::string> visited;
        TempStructure *currInh = &stru.second;
        TempStructure *nextInh = NULL;
        while (currInh->inherit != "-") {
            if (!tmpStructures.contains(currInh->inherit)) {
                std::ostringstream msg;
                msg << "Structure " << stru.first << " inherits from " << nextInh->inherit << ", which couldn't be found";
                throw StructureFileException(msg.str());
            }
            nextInh = &tmpStructures[currInh->inherit];
            if (nextInh->stru(structs).inherits != NULL) break; // already has inheritance info
            if (visited.contains(nextInh->stru(structs).name)) {
                std::ostringstream msg;
                msg << "Structure " << stru.first << " is self-referential!";
                throw StructureFileException(msg.str());
            }
            visited.insert(currInh->stru(structs).name);
            currInh->stru(structs).inherits = &nextInh->stru(structs);
            currInh = nextInh;
        }
    }

    // set structure sizes
    for (auto &stru : tmpStructures) {
        std::vector<TempStructure *> path(0);
        auto tmp = &stru.second;
        path.push_back(tmp);
        while (tmp->inherit != "-" && tmp->stru(structs).size == -1) {
            tmp = &tmpStructures[tmp->inherit];
            path.push_back(tmp);
        }
        int size = 0;
        while (path.size() > 0) {
            size += path[path.size() - 1]->localSize;
            path[path.size() - 1]->stru(structs).size = size;
            path.pop_back();
        }
    }

    // finally parse the fields
    for (auto const& stru : tmpStructures) {
        std::regex fieldBlock("([^ ]+) ([^:]+):([^;]+);", std::regex::ECMAScript);
        std::smatch fieldBlocks;

        Structure &s = stru.second.stru(structs);
        auto content = stru.second.content;

        std::string::const_iterator fieldBlockStart(content.cbegin());    
        while (std::regex_search(fieldBlockStart, content.cend(), fieldBlocks, fieldBlock)) {
            std::string offset = fieldBlocks[1].str().c_str();
            std::string name = fieldBlocks[2].str().c_str();
            std::string typeName = fieldBlocks[3].str().c_str();

            StructField field;
            field.modifyable = true;
            field.name = name;
            if (typeName[0] == '!') {
                field.modifyable = false;
                typeName.erase(0, 1);
            }
            if (tmpStructures.contains(typeName)) {
                field.isBasic = false;
                field.ptr.structure = &tmpStructures[typeName].stru(structs);
            } else if (basicTypes.contains(typeName)) {
                field.isBasic = true;
                field.ptr.base = &basicTypes[typeName];
            } else {
                std::ostringstream msg;
                msg << "Structure " << stru.first << " contains type " << typeName << ", which couldn't be found.";
                throw StructureFileException(msg.str());
            }

            int useBaseOffset = 0;
            if (offset[0] == '+') {
                if (s.inherits != NULL) useBaseOffset = s.inherits->size;
            }
            u32 offset_int = decHexToInt(offset) + useBaseOffset;
            s.fields.push_back(std::pair<u32, StructField>(offset_int, field));

            fieldBlockStart = fieldBlocks.suffix().first;
        }
    }
}

Structure *StructureFile::getStruct(std::string find) {
    for (auto &s : structs) {
        if (s.name == find) return &s;
    }
    return NULL;
}