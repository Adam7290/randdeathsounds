#pragma once
#include <Geode/Geode.hpp>

#define MBO(type, class, offset) *reinterpret_cast<type*>(reinterpret_cast<uintptr_t>(class) + offset)

namespace Util {
    geode::ByteVector patchToBytes(const char* patch);
}