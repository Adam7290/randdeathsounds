#include <Geode/Geode.hpp>
#include <random>
#include "folder_manager.h"

using namespace geode::prelude;

const char* DEFAULT_DEATH_SOUND = "explode_11.ogg";
const std::set<fs::path> SUPPORTED_FILE_EXTENSIONS = {
	".ogg",
	".mp3",
	".wav",
};

#if defined(GEODE_IS_WINDOWS)
constexpr uintptr_t DEATH_SOUND_OFFSET = 0x2E6B87;
#elif defined(GEODE_IS_ANDROID64)
constexpr uintptr_t DEATH_SOUND_OFFSET = 0x006bab78;
#else
#error Undefined death sound offset for platform.
#endif

char* deathSound = new char[4096];
std::vector<gd::string> soundPool;
std::mt19937 rng;

void updateSound() {
	if (Mod::get()->getSettingValue<bool>("enabled") && !soundPool.empty()) {
		const char* randSound = soundPool[std::uniform_int_distribution<int>(0, soundPool.size()-1)(rng)].c_str();
		strcpy(deathSound, randSound);
	} else {
		strcpy(deathSound, DEFAULT_DEATH_SOUND);
	}
}

#include <Geode/modify/PlayLayer.hpp>
class $modify(PlayLayer) {
	$override bool init(GJGameLevel* p0, bool p1, bool p2) {
		if (!PlayLayer::init(p0, p1, p2)) {
			return false;
		}

		soundPool.clear();
		for (const auto& file : fs::directory_iterator(Folder::getFolder())) {
			if (SUPPORTED_FILE_EXTENSIONS.find(file.path().extension()) != SUPPORTED_FILE_EXTENSIONS.end()) {
				gd::string sound = file.path().string();
				soundPool.push_back(sound);
				FMODAudioEngine::sharedEngine()->preloadEffect(sound);
			}
		}

		return true;
	}
};

#include <Geode/modify/PlayerObject.hpp>
class $modify(PlayerObject) {
	$override void playerDestroyed(bool p) {
		if (!p) {
			updateSound();
		}

		PlayerObject::playerDestroyed(p);
	}
};

$execute {
	// Create death sounds folder
	Folder::setup();

	// Seed rng
	rng.seed(time(NULL));
	
	// Make push instructions push our deathSound var instead of gd "explode_11.ogg" string
	const auto address = reinterpret_cast<void*>(geode::base::get() + DEATH_SOUND_OFFSET);
	log::info("Patching death sound (0x{:X})", (uintptr_t)address);
	if (Mod::get()->patch(
		address,
		geode::toByteArray(deathSound)
	).isErr()) {
		log::error("Failed to patch death sounds");
	}
};
