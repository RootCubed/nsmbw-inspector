#pragma once

#include "helper.h"
#include "nsmbw/types.h"

#include <string>
#include <vector>
#include <cstring>

typedef struct {
    u32 addr;
    union {
        s32 si32;
        u32 us32;
        s16 si16;
        u16 us16;
        s8  si8;
        u8  us8;
        float floating;
        char *str;
        char binary[256];
    } data;
} baseTypeStruct;

class BasicType {
public:
    int typeSize;
    std::string typeName;
    
    BasicType(int s, std::string t);

    virtual void display(std::string, baseTypeStruct);
    virtual std::string preview(baseTypeStruct);
};

class Structure;

typedef struct {
    bool isBasic;
    bool isModifyable;
    bool isPointer;
    std::string name;
    u32 offset;

    union {
        BasicType *base;
        Structure *structure;
    } ptr;
} StructField;

struct Previewer {
    bool useDefaultPreview;
    std::vector<StructField *> fields;
    std::vector<std::string> str;
};

struct Structure {
    std::string name;
    Structure *inherits;
    int size;
    Previewer previewer;
    std::vector<StructField> fields;
};

class StructureFile;

struct TempStructure {
    std::string inherit;
    size_t localSize;
    std::string content;
    int posInVector;

    Structure &stru(std::vector<Structure> &) const;
};

class StructureInstance {
    Structure *type;

    std::string buildPreview(StructField *, u32);

    public:
    bool showAllFields = false;
    StructureInstance();
    StructureInstance(Structure *);

    void setType(Structure *);
    int getReadSize();

    void drawInstance(u32 ptr);
};

class StructureFile {
    std::vector<Structure> structs;

    void parseStructureBlocks(std::string);
    void parsePreviewBlocks(std::string);

    public:
    StructureFile();
    StructureFile(std::string);

    Structure *getStruct(std::string);
};

struct StructureFileException : public std::exception {
    private:
    char msg[256];

    public:
    StructureFileException(const std::string &message) noexcept {
        strcpy(msg, "Error parsing structure file: ");
        strcat(msg, message.c_str());
    }

	const char * what () const throw () {
        return msg;
    }
};