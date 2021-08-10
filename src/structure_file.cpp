#include "structure_file.h"

#include <unordered_set>
#include <sstream>

BasicType::BasicType(int s, void (*disp)(std::string, baseTypeUnion)) {
    size = s;
    display = disp;
}

BasicType::BasicType() {
    size = 0;
    display = NULL;
}

void basicTypeU32_display(std::string name, baseTypeUnion data) {
    ImGui::InputScalar(name.c_str(), ImGuiDataType_U32, &data.us32);
}
void basicTypePtr_display(std::string name, baseTypeUnion data) {
    ImGui::InputScalar(name.c_str(), ImGuiDataType_U32, &data.us32, NULL, NULL, "%08X", ImGuiInputTextFlags_CharsHexadecimal);
}
void basicTypeS32_display(std::string name, baseTypeUnion data) {
    ImGui::InputScalar(name.c_str(), ImGuiDataType_S32, &data.si32);
}
void basicTypeU16_display(std::string name, baseTypeUnion data) {
    ImGui::InputScalar(name.c_str(), ImGuiDataType_U16, &data.us16);
}
void basicTypeS16_display(std::string name, baseTypeUnion data) {
    ImGui::InputScalar(name.c_str(), ImGuiDataType_S16, &data.si16);
}
void basicTypeS16Ang_display(std::string name, baseTypeUnion data) {
    float num = data.si16 / 0xFFFF * 360;
    ImGui::InputScalar(name.c_str(), ImGuiDataType_S16, &num, NULL, NULL, "%f°");
}
void basicTypeU8_display(std::string name, baseTypeUnion data) {
    ImGui::InputScalar(name.c_str(), ImGuiDataType_U8, &data.us8);
}
void basicTypeS8_display(std::string name, baseTypeUnion data) {
    ImGui::InputScalar(name.c_str(), ImGuiDataType_S8, &data.si8);
}
void basicTypeFloat_display(std::string name, baseTypeUnion data) {
    ImGui::InputFloat(name.c_str(), &data.floating);
}
char tmp[] = "WIP";
void basicTypeStr_display(std::string name, baseTypeUnion data) {
    ImGui::InputText(name.c_str(), tmp, 3);
}
void basicTypeJIS_display(std::string name, baseTypeUnion data) {
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
    return type->size;
}

void StructureInstance::renderInstance() {
    for (auto &field : type->fields) {
        if (field.second.isBasic) {
            baseTypeUnion thisData;
            std::memcpy(thisData.data, &data[0], field.second.ptr.base->size);
            field.second.ptr.base->display(field.second.name, thisData);
        }
    }
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

    std::regex structureBlock("structure ([^:]+):([^:]+):([^\\{ ]+) *\\{([^\\}]+)\\}", std::regex::ECMAScript);
    std::smatch structureBlocks;
    
    std::map<std::string, std::tuple<size_t, std::string, std::string>> tmpStructures;

    std::string::const_iterator structureBlocksStart(file.cbegin());    
    while (std::regex_search(structureBlocksStart, file.cend(), structureBlocks, structureBlock)) {
        std::string name = structureBlocks[1].str().c_str();
        std::string inherits = structureBlocks[2].str().c_str();
        std::string size = structureBlocks[3].str().c_str();
        std::string content = structureBlocks[4].str().c_str();
        Structure s;
        s.name = name;
        s.size = decHexToInt(size);
        s.inherits = NULL;
        if (tmpStructures.contains(name)) {
            std::ostringstream msg;
            msg << "Duplicate structure name \"" << name << "\"";
            throw StructureFileException(msg.str());
        }
        structs.push_back(s);
        tmpStructures[name] = std::tuple<size_t, std::string, std::string>(structs.size() - 1, inherits, content);
        structureBlocksStart = structureBlocks.suffix().first;
    }

    // test for self-inheriting structs
    for (auto const& stru : tmpStructures) {
        std::unordered_set<std::string> visited;
        std::string currInh = stru.first;
        std::string nextInh = std::get<1>(stru.second);
        while (nextInh != "-") {
            if (!tmpStructures.contains(nextInh)) {
                std::ostringstream msg;
                msg << "Structure " << stru.first << " inherits from " << nextInh << ", which couldn't be found";
                throw StructureFileException(msg.str());
            }
            int idxOld = std::get<0>(tmpStructures[currInh]);
            if (structs[idxOld].inherits != NULL) break; // already has inheritance info
            if (visited.contains(nextInh)) {
                std::ostringstream msg;
                msg << "Structure " << stru.first << " is self-referential!";
                throw StructureFileException(msg.str());
            }
            visited.insert(currInh);
            int idxNew = std::get<0>(tmpStructures[nextInh]);
            structs[idxOld].inherits = &structs[idxNew];
            currInh = nextInh;
            nextInh = std::get<1>(tmpStructures[currInh]);
        }
    }

    // finally parse the fields and add the structure definition to the vector
    for (auto const& stru : tmpStructures) {
        std::regex fieldBlock("([^ ]+) ([^:]+):([^;]+);", std::regex::ECMAScript);
        std::smatch fieldBlocks;

        Structure *s = &structs[std::get<0>(stru.second)];
        auto content = std::get<2>(stru.second);

        std::string::const_iterator fieldBlockStart(content.cbegin());    
        while (std::regex_search(fieldBlockStart, content.cend(), fieldBlocks, fieldBlock)) {
            std::string offset = fieldBlocks[1].str().c_str();
            std::string name = fieldBlocks[2].str().c_str();
            std::string typeName = fieldBlocks[3].str().c_str();
            u32 offset_int = decHexToInt(offset);

            StructField field;
            field.modifyable = true;
            field.name = name;
            if (typeName[0] == '!') {
                field.modifyable = false;
                typeName.erase(0, 1);
            }
            if (tmpStructures.contains(typeName)) {
                field.isBasic = false;
                field.ptr.structure = &structs[std::get<0>(tmpStructures[typeName])];
            } else if (basicTypes.contains(typeName)) {
                field.isBasic = true;
                field.ptr.base = &basicTypes[typeName];
            } else {
                std::ostringstream msg;
                msg << "Structure " << stru.first << " contains type " << typeName << " which couldn't be found.";
                throw StructureFileException(msg.str());
            }
            s->fields.push_back(std::pair<u32, StructField>(offset_int, field));

            fieldBlockStart = fieldBlocks.suffix().first;
        }
    }

    // todo: preview, display, object
}

Structure *StructureFile::getStruct(std::string find) {
    for (auto &s : structs) {
        if (s.name == find) return &s;
    }
    return NULL;
}