#include "randomizer.h"
#include "folder_manager.h"
#include "util.h"
#include <random>

using namespace geode::prelude;

const char* DEFAULT_DEATH_SOUND = "explode_11.ogg";

gd::string customDeathSound = DEFAULT_DEATH_SOUND;
std::vector<gd::string> soundPool;
unsigned long poolHash = 0; // Used to detect when to reload sounds
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
	// Hash all the sound paths and last modified timestamps to detect when to reload sounds
	unsigned int oldHash = poolHash;
	poolHash = 0;

	soundPool.clear();
	for (const auto& file : fs::directory_iterator(Folder::getFolder())) {
		if (isValidExtension(file.path().extension().string())) {
			gd::string sound = file.path().string();
			soundPool.push_back(sound);

			poolHash = Util::hash2(poolHash, geode::utils::hash(sound.c_str()));
			poolHash = Util::hash2(poolHash, fs::last_write_time(file).time_since_epoch().count());
		}
	}

	if (poolHash != oldHash) {
		if (oldHash != 0) { // Check that we are actually "reloading" death sounds and not loading for the first time
			log::info("Reloading death sounds!");
		}

		// Reload effects
		for (auto& sound : soundPool) {
			FMODAudioEngine::sharedEngine()->preloadEffect(sound);
		}
	}
}

void Randomizer::playRandomDeathSound() {
    randomizeSound();

	auto controlVanillaSFX = Loader::get()->getLoadedMod("raydeeux.controlvanillasfx");
	// Added by ery
	if (controlVanillaSFX && controlVanillaSFX->getSettingValue<bool>("enabled")) {
		float volume = controlVanillaSFX->getSettingValue<int64_t>("volume") / 100.0f;
        volume *= controlVanillaSFX->getSettingValue<double>("volumeBoost");
        
        auto system = FMODAudioEngine::sharedEngine()->m_system;

        FMOD::Channel* channel;
        FMOD::Sound* sound;
        
        system->createSound(customDeathSound.c_str(), FMOD_DEFAULT, nullptr, &sound);
        system->playSound(sound, nullptr, false, &channel);
        channel->setVolume(volume);
    } else {
        FMODAudioEngine::sharedEngine()->playEffect(customDeathSound);
    }
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