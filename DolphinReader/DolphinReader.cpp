#include "DolphinReader.h"

#include <iostream>
#include <utility>


char memoryBuffer[0x10000];

u32 dist;

DolphinComm::DolphinAccessor::DolphinStatus DolphinReader::hook() {
	DolphinComm::DolphinAccessor::hook();

	dist = DolphinComm::DolphinAccessor::getMEM1ToMEM2Distance();

	return DolphinComm::DolphinAccessor::getStatus();
}

u32 DolphinReader::readU32(u32 address) {
	DolphinComm::DolphinAccessor::readFromRAM(Common::dolphinAddrToOffset(address, dist), memoryBuffer, 4, true);

	return *(u32*) memoryBuffer;
}

u16 DolphinReader::readU16(u16 address) {
	DolphinComm::DolphinAccessor::readFromRAM(Common::dolphinAddrToOffset(address, dist), memoryBuffer, 2, true);

	return *(u16*) memoryBuffer;
}

u8 DolphinReader::readU16(u8 address) {
	DolphinComm::DolphinAccessor::readFromRAM(Common::dolphinAddrToOffset(address, dist), memoryBuffer, 1, true);

	return *(u8*) memoryBuffer;
}

float DolphinReader::readFloat(u32 address) {
	DolphinComm::DolphinAccessor::readFromRAM(Common::dolphinAddrToOffset(address, dist), memoryBuffer, 4, true);

	return *(float*) memoryBuffer;
}

void *DolphinReader::readValues(u32 address, u16 size) {
	DolphinComm::DolphinAccessor::readFromRAM(Common::dolphinAddrToOffset(address, dist), memoryBuffer, size, false);
	return memoryBuffer;
}

void DolphinReader::writeU32(u32 address, u32 value) {
	DolphinComm::DolphinAccessor::writeToRAM(Common::dolphinAddrToOffset(address, dist), (char *) &value, 4, true);
}

void DolphinReader::writeU16(u32 address, u16 value) {
	DolphinComm::DolphinAccessor::writeToRAM(Common::dolphinAddrToOffset(address, dist), (char *) &value, 2, true);
}

void DolphinReader::writeU8(u32 address, u8 value) {
	DolphinComm::DolphinAccessor::writeToRAM(Common::dolphinAddrToOffset(address, dist), (char *) &value, 1, true);
}

void DolphinReader::writeFloat(u32 address, float value) {
	writeU32(address, *(u32 *) &value);
}

void DolphinReader::writeValues(u32 address, void *data, u16 size) {
	DolphinComm::DolphinAccessor::writeToRAM(Common::dolphinAddrToOffset(address, dist), (char *) data, size, true);
}