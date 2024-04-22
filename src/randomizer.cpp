#include "randomizer.h"
#include "folder_manager.h"
#include <random>

using namespace geode::prelude;

const char* DEFAULT_DEATH_SOUND = "explode_11.ogg";

gd::string customDeathSound = DEFAULT_DEATH_SOUND;
std::vector<gd::string> soundPool;
std::mt19937 rng;

// Reload sounds when entering a level
#include <Geode/modify/PlayLayer.hpp>
class $modify(PlayLayer) {
	$override bool init(GJGameLevel* p0, bool p1, bool p2) {
		if (!PlayLayer::init(p0, p1, p2)) {
			return false;
		}

		Randomizer::loadSounds();

		return true;
	}
};

void Randomizer::setup() {
    // Seed rng
	rng.seed(time(NULL));

    // Preload death sounds for clicking on icons in MenuLayer
	loadSounds();
	randomizeSound();
}

bool Randomizer::isEnabled() {
    return Mod::get()->getSettingValue<bool>("enabled") && !soundPool.empty();
}

void Randomizer::randomizeSound() {
	if (isEnabled()) {
		customDeathSound = soundPool[std::uniform_int_distribution<int>(0, soundPool.size()-1)(rng)];
	} else {
		customDeathSound = DEFAULT_DEATH_SOUND;
	}
}

void Randomizer::loadSounds() {
	soundPool.clear();
	for (const auto& file : fs::directory_iterator(Folder::getFolder())) {
		if (isValidExtension(file.path().extension().string())) {
			gd::string sound = file.path().string();
			soundPool.push_back(sound);
			FMODAudioEngine::sharedEngine()->preloadEffect(sound);
		}
	}
}

void Randomizer::playRandomDeathSound() {
    randomizeSound();

    // TODO: ControllVanillaSFX support
    FMODAudioEngine::sharedEngine()->playEffect(customDeathSound, 1, 1, GameManager::get()->m_sfxVolume);
}

bool Randomizer::isValidExtension(const std::string &filePath) {
    switch (geode::utils::hash(filePath.c_str())) {
        case ".ogg"_h:
        case ".mp3"_h:
        case ".wav"_h:
            return true;
    }

    return false;
}