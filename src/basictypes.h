#pragma once

#include "structure_file.h"

class BasicTypeU32 : public BasicType {
    public:
    BasicTypeU32() : BasicType(4, "u32") {};

    void display(std::string, baseTypeStruct) override;
    std::string preview(baseTypeStruct) override;
};

class BasicTypePtr : public BasicType {
    public:
    BasicTypePtr() : BasicType(4, "ptr") {};
    
    void display(std::string, baseTypeStruct) override;
    std::string preview(baseTypeStruct) override;
};

class BasicTypeS32 : public BasicType {
    public:
    BasicTypeS32() : BasicType(4, "s32") {};
    
    void display(std::string, baseTypeStruct) override;
    std::string preview(baseTypeStruct) override;
};

class BasicTypeU16 : public BasicType {
    public:
    BasicTypeU16() : BasicType(2, "u16") {};
    
    void display(std::string, baseTypeStruct) override;
    std::string preview(baseTypeStruct) override;
};

class BasicTypeS16 : public BasicType {
    public:
    BasicTypeS16() : BasicType(2, "s16") {};
    
    void display(std::string, baseTypeStruct) override;
    std::string preview(baseTypeStruct) override;
};

class BasicTypeS16Ang : public BasicType {
    public:
    BasicTypeS16Ang() : BasicType(2, "s16angle") {};
    
    void display(std::string, baseTypeStruct) override;
    std::string preview(baseTypeStruct) override;
};

class BasicTypeU8 : public BasicType {
    public:
    BasicTypeU8() : BasicType(1, "u8") {};
    
    void display(std::string, baseTypeStruct) override;
    std::string preview(baseTypeStruct) override;
};

class BasicTypeS8 : public BasicType {
    public:
    BasicTypeS8() : BasicType(1, "s8") {};
    
    void display(std::string, baseTypeStruct) override;
    std::string preview(baseTypeStruct) override;
};

class BasicTypeFloat : public BasicType {
    public:
    BasicTypeFloat() : BasicType(4, "float") {};
    
    void display(std::string, baseTypeStruct) override;
    std::string preview(baseTypeStruct) override;
};

class BasicTypeStr : public BasicType {
    public:
    BasicTypeStr() : BasicType(4, "string") {};
    
    void display(std::string, baseTypeStruct) override;
    std::string preview(baseTypeStruct) override;
};

class BasicTypeJIS : public BasicType {
    public:
    BasicTypeJIS() : BasicType(4, "stringJIS") {};
    
    void display(std::string, baseTypeStruct) override;
};