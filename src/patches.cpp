#include "patches.h"
#include <string_view>

#include <Geode/Geode.hpp>
using namespace geode::prelude;

// This gets replaced with address to deathSound variable
const char* DEATH_SOUND_MAGIC_STRING = "^DEATH_SOUND^";

struct DeathSoundPatch {
    uintptr_t address;
    const char* patch;
};

#if defined(GEODE_IS_WINDOWS)
// Simply replace the explode_11.ogg string address with our deathSound variable
const DeathSoundPatch DEATH_SOUND_PATCHES[] = {
    {
        .address = 0x2E6B87,
        .patch = "^DEATH_SOUND^"
    }
};
#elif defined(GEODE_IS_ANDROID64)
// Same as windows but we have to NOP some random ass instruction that adds to the address for some reason (idk arm assembly)
const uintptr_t DEATH_SOUND_PATCH_ADDRESS = 0x5BAB78;
const std::array<const char*, 2> DEATH_SOUND_PATCHES = {"^DEATH_SOUND^"};
#else
#error Undefined death sound patches for platform.
#endif

cpp::result<void, std::string> Patches::setup(char* deathSoundVar) {
    for (const DeathSoundPatch& rawPatch : DEATH_SOUND_PATCHES) {
        // Replace magic string occurance with deathSoundVar
        std::string patch = rawPatch.patch;
        size_t magicPos = patch.find(DEATH_SOUND_MAGIC_STRING);
        if (magicPos != patch.npos) {
            patch.replace(magicPos, std::strlen(DEATH_SOUND_MAGIC_STRING), std::string_view(reinterpret_cast<char*>(&deathSoundVar), sizeof(uintptr_t)));
        }
        
        auto result = Mod::get()->patch(
            reinterpret_cast<void*>(geode::base::get() + rawPatch.address),
            ByteVector(patch.begin(), patch.end())
        );

        if (result.isErr()) {
            return cpp::fail(result.unwrapErr());
        }
    }

    return {};
}