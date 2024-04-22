#pragma once
#include <result.hpp>

namespace Patches {
    cpp::result<void, std::string> setup(char* deathSoundVar);
}