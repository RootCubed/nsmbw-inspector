#pragma once

#include "nsmbw/types.h"

#include <regex>
#include <string>
#include <iostream>
#include <utility>
#include <map>

#include "helper.h"

#include <imgui.h>

/*typedef enum {
    signed32,
    unsigned32,
    pointer,
    signed16,
    singed16ang,
    unsigned16,
    signed8,
    unsigned8,
    floatingPoint,
    stringASCII,
    stringJIS
} ebaseType;*/

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
    void (*display)(std::string, baseTypeStruct);
    int size;

    BasicType();
    BasicType(int, void (*)(std::string, baseTypeStruct));
};

class Structure;

typedef struct {
    bool isBasic;
    bool modifyable;
    std::string name;
    u32 offset;

    union {
        BasicType *base;
        Structure *structure;
    } ptr;
} StructField;

struct Previewer {
    std::vector<StructField *> fields;
    std::vector<std::string> str;
};

struct Structure {
    std::string name;
    Structure *inherits;
    int size;
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
    std::vector<char> data;

    Structure *type;

    public:
    StructureInstance();
    StructureInstance(Structure *);

    void setType(Structure *);
    void updateData(std::vector<char> data);
    int getReadSize();

    void drawInstance(u32 ptr);
};

class StructureFile {
    std::vector<Structure> structs;

    void parseStructureBlocks(std::string);

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
        std::strcpy(msg, "Error parsing structure file: ");
        std::strcat(msg, message.c_str());
    }

	const char * what () const throw () {
        return msg;
    }
};