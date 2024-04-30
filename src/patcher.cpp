// patches functions to skip playEffect("explode_11.ogg") and sets up hooks to play custom sound effect

#include <Geode/Geode.hpp>
using namespace geode::prelude;

#include "patcher.h"
#include "randomizer.h"
#include "util.h"

#pragma region PATCHES

struct DeathSoundPatch {
    uintptr_t address;
    const char* patch;
};

static const DeathSoundPatch DEATH_SOUND_PATCHES[] = {
#if defined(GEODE_IS_WINDOWS)
    // PlayLayer death
    // { // NOP out call to playEffect
    //     .address = 0x2E6BBA,
    //     .patch = "90 90 90 90 90",
    // },
    { // For some reason I cant nop the playEffect call so I replace JNZ with JMP in if(this->m_disabledDeathSFX) statement to unconditionally skip playEffect for now.
        .address = 0x2E6B7B,
        .patch = "EB",
    },
    { // NOP out stopAllEffects so we can manually call it after playing our death sound
        .address = 0x2E6B6F,
        .patch = "90 90 90 90 90",
    },
    
    // PlayLayer resetLevel
    { // NOP stopAllEffects
        .address = 0x2EA179,
        .patch = "90 90 90 90 90",
    },

    // MenuGameLayer death
    { // NOP out playEffect call
        .address = 0x27AED4,
        .patch = "90 90 90 90 90",
    },
#elif defined(GEODE_IS_ANDROID64)
    // PlayLayer Death
    { // NOP playEffect
        .address = 0x5BAB9C,
        .patch = "1F 20 03 D5",
    },
    { // NOP out stopAllEffects
        .address = 0x5BAB5C,
        .patch = "1F 20 03 D5",
    },

    // PlayLayer resetLevel
    {
        .address = 0x5CC8D0,
        .patch = "1F 20 03 D5",
    },

    // MenuGameLayer death
    { // NOP out playEffect call
        .address = 0x6A3138,
        .patch = "1F 20 03 D5",
    }

#elif defined(GEODE_IS_ANDROID32)
    // PlayLayer death
    { // NOP playEffect
        .address = 0x306DBE,
        .patch = "00 BF 00 BF",
    },
    { // NOP stopAllEffects
        .address = 0x306D8E,
        .patch = "00 BF 00 BF",
    },

    // PlayLayer resetLevel
    {
        .address = 0x31073E,
        .patch = "00 BF 00 BF",
    },

    // MenuGameLayer death
    { // NOP playEffect
        .address = 0x39447E,
        .patch = "00 BF 00 BF",
    }
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
            Util::patchToBytes(patch.patch)
        ).isErr()) {
            log::error("Failed to patch: 0x{:X}", patch.address);
        }
    }
}

#pragma endregion

#pragma region HOOKS

// I can't be bothered to learn how to add members to bindings
// TODO: Add m_disabledDeathSFX to bindings instead lol...
static const uintptr_t disabledDeathSFXOffset =
#if defined(GEODE_IS_WINDOWS) 
0x414
#elif defined(GEODE_IS_ANDROID64)
0x514
#elif defined(GEODE_IS_ANDROID32)
0x414
//#elif defined(GEODE_IS_MACOS)
#else
#error Undefined m_disabledDeathSFX offset for platform.
#endif
;

#include <Geode/modify/PlayerObject.hpp>
class $modify(PlayerObject) {
    $override void playerDestroyed(bool p0) {
        PlayerObject::playerDestroyed(p0);

        auto playLayer = PlayLayer::get();
        if (playLayer == nullptr) { return; }

        bool disabled = MBO(bool, playLayer, disabledDeathSFXOffset);
        if (Mod::get()->getSettingValue<bool>("force-play-sfx")) {
            disabled = false;
        }

        // Can't use p0 because of some stupid mod incompat
        if (!p0 && !disabled) {
            FMODAudioEngine::sharedEngine()->stopAllEffects(); // Since we NOPed it out...
            Randomizer::playRandomDeathSound();
        }
    }
};

#include <Geode/modify/PlayLayer.hpp>
class $modify(PlayLayer) {
    $override void resetLevel() {
        if (Mod::get()->getSettingValue<bool>("stop-sfx-on-respawn")) {
            // Bug (that I'm hoping no one will notice):
            // If you play a sound after a player dies with this option off it won't stop on respawn
            FMODAudioEngine::sharedEngine()->stopAllEffects();
        }

        PlayLayer::resetLevel();
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