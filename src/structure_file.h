#include "nsmbw/types.h"

#include <regex>
#include <string>
#include <iostream>
#include <utility>
#include <map>

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

typedef union {
    s32 si32;
    u32 us32;
    s16 si16;
    u16 us16;
    s8  si8;
    u8  us8;
    float floating;
    char *str;
    char data[256];
} baseTypeUnion;

class BasicType {
    public:
    void (*display)(std::string, baseTypeUnion);
    int size;

    BasicType();
    BasicType(int, void (*)(std::string, baseTypeUnion));
};

class Structure;

typedef struct {
    bool isBasic;
    bool modifyable;
    std::string name;

    union {
        BasicType *base;
        Structure *structure;
    } ptr;
} StructField;

struct Structure {
    std::string name;
    Structure *inherits;
    size_t size;
    std::vector<std::pair<u32, StructField>> fields;
};

class StructureInstance {
    std::vector<char> data;

    Structure *type;

    public:
    StructureInstance();
    StructureInstance(Structure *datatype, std::vector<char> data);

    void setType(Structure *datatype);
    void updateData(std::vector<char> data);
    int getReadSize();

    void renderInstance();
};

class StructureFile {
    std::vector<Structure> structs;

    public:
    StructureFile();
    StructureFile(std::string);

    Structure *getStruct(std::string);

    std::vector<Structure> getTabs(std::string);
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