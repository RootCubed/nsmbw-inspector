#include "DolphinReader/DolphinReader.h"

#include "basictypes.h"
#include "structure_file.h"

#include <unordered_set>
#include <sstream>
#include <iostream>
#include <map>
#include <regex>

#include <imgui.h>

#ifdef WIN32
    #include <Windows.h>
#else
#endif

int decHexToInt(std::string str) {
    if (str.rfind("0x", 0) == 0) {
        return std::stoi(str, 0, 16);
    }
    return std::stoi(str, 0, 10);
}

std::map<std::string, BasicType *> basicTypes = {
    {std::string("u32"),       new BasicTypeU32()},
    {std::string("ptr"),       new BasicTypePtr()},
    {std::string("s32"),       new BasicTypeU32()},
    {std::string("u16"),       new BasicTypeU16()},
    {std::string("s16"),       new BasicTypeS16()},
    {std::string("s16angle"),  new BasicTypeS16Ang()},
    {std::string("u8"),        new BasicTypeU8()},
    {std::string("s8"),        new BasicTypeS8()},
    {std::string("float"),     new BasicTypeFloat()},
    {std::string("string"),    new BasicTypeStr()},
    {std::string("stringJIS"), new BasicTypeJIS()},
    {std::string("bool"),      new BasicTypeBool()}
};

Structure &TempStructure::stru(std::vector<Structure> &vec) const {
    return vec.at(posInVector);
}

StructureInstance::StructureInstance() {
    type = NULL;
}

StructureInstance::StructureInstance(Structure *_type) {
    setType(_type);
}

void StructureInstance::setType(Structure *_type) {
    type = _type;
}

int StructureInstance::getReadSize() {
    if (type == NULL) return -1;
    return type->size;
}

void StructureInstance::drawInstance(u32 ptr) {
    int rS = getReadSize();
    std::vector<char> data(rS, 0);
    if (rS > 0 && rS < MEMBUF_SIZE) {
        void *tmp = DolphinReader::readValues(ptr, rS);
        std::memcpy(&data[0], tmp, rS);
    } else {
        printf("data read error: readSize = %d\n", rS);
        return;
    }
    Structure *curr = type;

    bool noInherit = curr->inherits == NULL;
    if (!noInherit) ImGui::BeginTabBar("Structs");
    do {
        bool shouldDraw = true;
        if (curr->fields.size() == 0 && !showAllFields) {
            continue;
        }
        if (!noInherit) shouldDraw = ImGui::BeginTabItem(curr->name.c_str());
        if (shouldDraw) {
            for (auto &field : curr->fields) {
                u32 valuePtr = ptr + field.offset;
                if (field.isPointer) {
                    valuePtr = _byteswap_ulong(*(u32 *) DolphinReader::readValues(valuePtr, 4));
                }
                if (field.isBasic) {
                    baseTypeStruct thisData;
                    thisData.addr = valuePtr;
                    if (field.isPointer) {
                        void *tmp = DolphinReader::readValues(valuePtr, field.ptr.base->typeSize);
                        std::memcpy(thisData.data.binary, &tmp, field.ptr.base->typeSize);
                    } else {
                        std::memcpy(thisData.data.binary, &data[0] + field.offset, field.ptr.base->typeSize);
                    }
                    field.ptr.base->display(field.name, thisData);
                } else {
                    // build preview
                    std::ostringstream treeNodeStr;
                    treeNodeStr << field.name << "[";
                    
                    // we pass the original pointer into here intentionally
                    // this makes the recursive preview building approach a bit easier to implement
                    std::string prev = buildPreview(&field, ptr);
                    if (prev == "") {
                        treeNodeStr << field.ptr.structure->name;
                    } else {
                        treeNodeStr << prev;
                    }
                    treeNodeStr << "]";
                    treeNodeStr << "###" << field.name;
                    if (ImGui::TreeNode(treeNodeStr.str().c_str())) {
                        StructureInstance selected(field.ptr.structure);
                        selected.drawInstance(valuePtr); // recursively draw the instances
                        ImGui::TreePop();
                    }
                }
            }
            if (!noInherit) ImGui::EndTabItem();
        }
    } while (curr = curr->inherits);
    if (!noInherit) ImGui::EndTabBar();
}

std::string StructureInstance::buildPreview(StructField *field, u32 ptr) {
    u32 valuePtr = ptr + field->offset;

    int size;
    if (field->isBasic) {
        size = field->ptr.base->typeSize;
    } else {
        size = field->ptr.structure->size;
    }
    
    if (field->isPointer) {
        valuePtr = _byteswap_ulong(*(u32 *) DolphinReader::readValues(valuePtr, 4));
    }
    
    void *fieldData = DolphinReader::readValues(valuePtr, size);

    if (field->isBasic) {
        baseTypeStruct thisData;
        thisData.addr = ptr + field->offset;
        std::memcpy(thisData.data.binary, fieldData, field->ptr.base->typeSize);
        return field->ptr.base->preview(thisData);
    }

    std::ostringstream previewString;
    Structure *curr = field->ptr.structure;
    while (curr != NULL) {
        if (!curr->previewer.useDefaultPreview) {
            for (int i = 0; i < curr->previewer.str.size(); i++) {
                previewString << curr->previewer.str[i];
                previewString << buildPreview(curr->previewer.fields[i], valuePtr);
            }
        }
        curr = curr->inherits;
    }
    return previewString.str();
}

StructureFile::StructureFile() {
    structs = std::vector<Structure>(0);
}

StructureFile::StructureFile(std::string fileWithLineBreaks) {
    structs = std::vector<Structure>(0);

    std::regex whitespace("\\n|\\r");

    std::string file = std::regex_replace(fileWithLineBreaks, whitespace, "");

    parseStructureBlocks(file);
    parsePreviewBlocks(file);

    // todo: display
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
        s.previewer.useDefaultPreview = true;
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
            //if (nextInh->stru(structs).inherits != NULL) break; // already has inheritance info
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
        TempStructure *tmp = &stru.second;
        path.push_back(tmp);
        while (tmp->inherit != "-") {
            tmp = &tmpStructures[tmp->inherit];
            path.push_back(tmp);
        }
        int size = 0;
        while (path.size() > 0) {
            TempStructure *el = path[path.size() - 1];
            size += el->localSize;
            el->stru(structs).size = size;
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
            field.isModifyable = true;
            field.isPointer = false;
            field.name = name;
            while (typeName[0] == '!' || typeName[0] == '*') {
                if (typeName[0] == '!') field.isModifyable = false;
                if (typeName[0] == '*') field.isPointer = true;
                typeName.erase(0, 1);
            }
            if (tmpStructures.contains(typeName)) {
                field.isBasic = false;
                field.ptr.structure = &tmpStructures[typeName].stru(structs);
            } else if (basicTypes.contains(typeName)) {
                field.isBasic = true;
                field.ptr.base = basicTypes[typeName];
            } else {
                std::ostringstream msg;
                msg << "Structure " << stru.first << " contains type " << typeName << ", which couldn't be found.";
                throw StructureFileException(msg.str());
            }

            int useBaseOffset = 0;
            if (offset[0] == '+') {
                if (s.inherits != NULL) useBaseOffset = s.inherits->size;
                offset.erase(0, 1);
            }
            u32 offset_int = decHexToInt(offset) + useBaseOffset;
            field.offset = offset_int;
            s.fields.push_back(field);

            fieldBlockStart = fieldBlocks.suffix().first;
        }
    }
}

void StructureFile::parsePreviewBlocks(std::string file) {
    std::regex previewBlockRegex("preview ([^ ]+) \\\"([^\\\"]+)\\\";", std::regex::ECMAScript);
    std::smatch previewBlock;

    std::string::const_iterator previewBlocksStart(file.cbegin());    
    while (std::regex_search(previewBlocksStart, file.cend(), previewBlock, previewBlockRegex)) {
        std::string structName = previewBlock[1].str();
        std::string dispString = previewBlock[2].str();

        Structure *structRef = NULL;
        for (int i = 0; i < structs.size(); i++) {
            if (structs[i].name == structName) {
                structRef = &structs[i];
                break;
            }
        }

        if (structRef == NULL) {
            std::ostringstream msg;
            msg << "Previewer: structure \"" << structName << "\" could not be found.";
            throw StructureFileException(msg.str());
        }

        structRef->previewer.useDefaultPreview = false;
        
        structRef->previewer.fields = std::vector<StructField *>(0);
        structRef->previewer.str = std::vector<std::string>(0);

        std::string currFieldName = "";
        std::string currStrPart = "";
        bool inField = false;
        for (int i = 0; i < dispString.length(); i++) {
            if (dispString[i] == '$') {
                if (inField) {
                    StructField *field = NULL;
                    for (int j = 0; j < structRef->fields.size(); j++) {
                        if (structRef->fields[j].name == currFieldName) {
                            field = &structRef->fields[j];
                        }
                    }
                    if (field == NULL) {
                        std::ostringstream msg;
                        msg << "In previewer for structure \"" << structName << "\": field \"" << currFieldName << "\" could not be found.";
                        throw StructureFileException(msg.str());
                    }
                    structRef->previewer.fields.push_back(field);
                } else {
                    structRef->previewer.str.push_back(currStrPart);
                }
                currFieldName = "";
                currStrPart = "";
                inField = !inField;
                continue;
            }
            if (inField) {
                currFieldName += dispString[i];
            } else {
                currStrPart += dispString[i];
            }
        }

        previewBlocksStart = previewBlock.suffix().first;
    }
}


Structure *StructureFile::getStruct(std::string find) {
    for (auto &s : structs) {
        if (s.name == find) return &s;
    }
    return NULL;
}