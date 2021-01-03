#include "Dolphin-memory-engine/Source/DolphinProcess/DolphinAccessor.h"
#include "Dolphin-memory-engine/Source/Common/CommonUtils.h"

#include <iostream>
#include <utility>
#include <map>

#include <nan.h>

std::map<unsigned int, std::pair<u32, unsigned int>> regAddrs;

unsigned int currentID = 0;

char memoryBuffer[0x10000];

NAN_METHOD(hook) {
	DolphinComm::DolphinAccessor::hook();

	int status = (int) DolphinComm::DolphinAccessor::getStatus();

	v8::Local<v8::Number> res = Nan::New(status);
	
	info.GetReturnValue().Set(res);
}

NAN_METHOD(readU32) {
	if (info.Length() != 1) {
		Nan::ThrowTypeError("Wrong number of arguments");
		return;
	}

	if (!info[0]->IsNumber()) {
		Nan::ThrowTypeError("Wrong argument type (must be integer)");
		return;
	}

	v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();

	u32 address = info[0]->NumberValue(context).FromJust();

	DolphinComm::DolphinAccessor::readFromRAM(Common::dolphinAddrToOffset(address), memoryBuffer, 4, true);

	info.GetReturnValue().Set(Nan::New(*(u32 *) memoryBuffer));
}

NAN_METHOD(readU16) {
	if (info.Length() != 1) {
		Nan::ThrowTypeError("Wrong number of arguments");
		return;
	}

	if (!info[0]->IsNumber()) {
		Nan::ThrowTypeError("Wrong argument type (must be integer)");
		return;
	}

	v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();

	u32 address = info[0]->NumberValue(context).FromJust();

	DolphinComm::DolphinAccessor::readFromRAM(Common::dolphinAddrToOffset(address), memoryBuffer, 2, true);

	info.GetReturnValue().Set(Nan::New(*(u16 *) memoryBuffer));
}

NAN_METHOD(readU8) {
	if (info.Length() != 1) {
		Nan::ThrowTypeError("Wrong number of arguments");
		return;
	}

	if (!info[0]->IsNumber()) {
		Nan::ThrowTypeError("Wrong argument type (must be integer)");
		return;
	}

	v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();

	u32 address = info[0]->NumberValue(context).FromJust();

	DolphinComm::DolphinAccessor::readFromRAM(Common::dolphinAddrToOffset(address), memoryBuffer, 1, true);

	info.GetReturnValue().Set(Nan::New(*(u8 *) memoryBuffer));
}

NAN_METHOD(readFloat) {
	if (info.Length() != 1) {
		Nan::ThrowTypeError("Wrong number of arguments");
		return;
	}

	if (!info[0]->IsNumber()) {
		Nan::ThrowTypeError("Wrong argument type (must be integer)");
		return;
	}

	v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();

	u32 address = info[0]->NumberValue(context).FromJust();

	DolphinComm::DolphinAccessor::readFromRAM(Common::dolphinAddrToOffset(address), memoryBuffer, 4, true);

	float res = *(float *) &memoryBuffer;

	info.GetReturnValue().Set(Nan::New(res));
}

NAN_METHOD(readValues) {
	if (info.Length() != 2) {
		Nan::ThrowTypeError("Wrong number of arguments");
		return;
	}

	if (!info[0]->IsNumber() || !info[1]->IsNumber()) {
		Nan::ThrowTypeError("Wrong argument types");
		return;
	}

	v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();

	u32 address = info[0]->NumberValue(context).FromJust();
	int size = info[1]->NumberValue(context).FromJust();

	DolphinComm::DolphinAccessor::readFromRAM(Common::dolphinAddrToOffset(address), memoryBuffer, size, false);
	v8::Local<v8::Array> arr = Nan::New<v8::Array>(size);

	for (int i = 0; i < size; i++) {
		Nan::Set(arr, i, Nan::New(memoryBuffer[i]));
	}

	info.GetReturnValue().Set(arr);
}

NAN_METHOD(writeU32) {
	if (info.Length() != 2) {
		Nan::ThrowTypeError("Wrong number of arguments");
		return;
	}

	if (!info[0]->IsNumber() || !info[1]->IsNumber()) {
		Nan::ThrowTypeError("Wrong argument type");
		return;
	}

	v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();

	u32 address = info[0]->NumberValue(context).FromJust();
	u32 value = info[1]->NumberValue(context).FromJust();

	DolphinComm::DolphinAccessor::writeToRAM(Common::dolphinAddrToOffset(address), (char *) &value, 4, true);
}

NAN_METHOD(writeU16) {
	if (info.Length() != 2) {
		Nan::ThrowTypeError("Wrong number of arguments");
		return;
	}

	if (!info[0]->IsNumber() || !info[1]->IsNumber()) {
		Nan::ThrowTypeError("Wrong argument type");
		return;
	}

	v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();

	u32 address = info[0]->NumberValue(context).FromJust();
	u16 value = info[1]->NumberValue(context).FromJust();

	DolphinComm::DolphinAccessor::writeToRAM(Common::dolphinAddrToOffset(address), (char *) &value, 2, true);
}

NAN_METHOD(writeU8) {
	if (info.Length() != 2) {
		Nan::ThrowTypeError("Wrong number of arguments");
		return;
	}

	if (!info[0]->IsNumber() || !info[1]->IsNumber()) {
		Nan::ThrowTypeError("Wrong argument type");
		return;
	}

	v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();

	u32 address = info[0]->NumberValue(context).FromJust();
	u8 value = info[1]->NumberValue(context).FromJust();

	DolphinComm::DolphinAccessor::writeToRAM(Common::dolphinAddrToOffset(address), (char *) &value, 1, true);
}

NAN_METHOD(writeFloat) {
	if (info.Length() != 2) {
		Nan::ThrowTypeError("Wrong number of arguments");
		return;
	}

	if (!info[0]->IsNumber() || !info[1]->IsNumber()) {
		Nan::ThrowTypeError("Wrong argument type");
		return;
	}

	v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();

	u32 address = info[0]->NumberValue(context).FromJust();
	float value = info[1]->NumberValue(context).FromJust();

	DolphinComm::DolphinAccessor::writeToRAM(Common::dolphinAddrToOffset(address), (char *) &value, 4, true);
}

NAN_METHOD(writeValues) {
	if (info.Length() != 2) {
		Nan::ThrowTypeError("Wrong number of arguments");
		return;
	}

	if (!info[0]->IsNumber() || !info[1]->IsArray()) {
		Nan::ThrowTypeError("Wrong argument type");
		return;
	}

	v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();

	u32 address = info[0]->NumberValue(context).FromJust();
	Nan::TypedArrayContents<uint8_t> data(info[1]);

	DolphinComm::DolphinAccessor::writeToRAM(Common::dolphinAddrToOffset(address), (char *) *data, data.length(), true);
}

NAN_MODULE_INIT(init) {
	DolphinComm::DolphinAccessor::init();
	Nan::SetMethod(target, "hook", hook);
	Nan::SetMethod(target, "readU32", readU32);
	Nan::SetMethod(target, "readU16", readU16);
	Nan::SetMethod(target, "readU8", readU8);
	Nan::SetMethod(target, "readValues", readValues);
	Nan::SetMethod(target, "readFloat", readFloat);

	Nan::SetMethod(target, "writeU32", writeU32);
	Nan::SetMethod(target, "writeU16", writeU16);
	Nan::SetMethod(target, "writeU8", writeU8);
	Nan::SetMethod(target, "writeValues", writeValues);
	Nan::SetMethod(target, "writeFloat", writeFloat);
}

NAN_MODULE_WORKER_ENABLED(hook, init);