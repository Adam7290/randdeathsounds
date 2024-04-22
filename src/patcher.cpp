// patches functions to skip playEffect("explode_11.ogg") and sets up hooks to play custom sound effect

#include <Geode/Geode.hpp>
using namespace geode::prelude;

#include "patcher.h"
#include "randomizer.h"
#include "util.h"

#pragma region PATCHES

struct DeathSoundPatch {
    uintptr_t address;
    const char* patch; // NULL terminated
};

const DeathSoundPatch DEATH_SOUND_PATCHES[] = {
#if defined(GEODE_IS_WINDOWS)
    // PlayLayer death
    // { // NOP out call to playEffect
    //     .address = 0x2E6BBA,
    //     .patch = "\x90\x90\x90\x90\x90",
    // },
    { // For some reason I cant nop the playEffect call so I replace JNZ with JMP in if(this->m_disabledDeathSFX) statement to unconditionally skip playEffect for now.
        .address = 0x2E6B7B,
        .patch = "\xEB",
    },
    { // NOP out stopAllEffects so we can manually call it after playing our death sound
        .address = 0x2E6B6F,
        .patch = "\x90\x90\x90\x90\x90",
    },

    // MenuGameLayer death
    { // NOP out playEffect call
        .address = 0x27AED4,
        .patch = "\x90\x90\x90\x90\x90",
    },
#elif defined(GEODE_IS_ANDROID64)
// TODO: Android64 support
    // PlayLayer Death
    { // nop playEffect
        .address = 0x5BAB9C,
        .patch = "\x1F\x20\x03\xD5",
    },
    { // NOP out stopAllEffects
        .address = 0x5BAB5C,
        .patch =  "\x1F\x20\x03\xD5",
    },

    // MenuGameLayer death
    { // NOP out playEffect call
        .address = 0x6A3138,
        .patch = "\x1F\x20\x03\xD5",
    }
//#elif defined(GEODE_IS_ANDROID32)
// TODO: Android32 support
//#elif defined(GEODE_IS_MACOS)
// TODO: MacOS support
#else
#error Undefined patches for platform.
#endif
};

void Patcher::setup() {
    for (const DeathSoundPatch& patch : DEATH_SOUND_PATCHES) {
        if(Mod::get()->patch(
            reinterpret_cast<void*>(geode::base::get() + patch.address),
            ByteVector(patch.patch, patch.patch + std::strlen(patch.patch))
        ).isErr()) {
            log::error("Failed to patch: 0x{:X}", patch.address);
        }
    }
}

#pragma endregion

#pragma region HOOKS

// I can't be bothered to learn how to add members to bindings
// TODO: Add member to bindings instead lol...
const uintptr_t disableDeathSFXOffset =
#if defined(GEODE_IS_WINDOWS) 
0x414
#elif defined(GEODE_IS_ANDROID64)
0x514
//#elif defined(GEODE_IS_ANDROID32)
//#elif defined(GEODE_IS_MACOS)
#else
#error Undefined m_disableDeathSFX offset for platform.
#endif
;

#include <Geode/modify/PlayerObject.hpp>
class $modify(PlayerObject) {
    $override void playerDestroyed(bool otherAlivePlayer) {
        PlayerObject::playerDestroyed(otherAlivePlayer);

        if (!otherAlivePlayer && MBO(bool, PlayLayer::get(), disableDeathSFXOffset) == false) {
            FMODAudioEngine::sharedEngine()->stopAllEffects(); // Since we NOPed it out...
            Randomizer::playRandomDeathSound();
        }
    }
};

#include <Geode/modify/MenuGameLayer.hpp>
class $modify(MenuGameLayer) {
    $override void destroyPlayer() {
        MenuGameLayer::destroyPlayer();
        Randomizer::playRandomDeathSound();
    }
};

#pragma endregion