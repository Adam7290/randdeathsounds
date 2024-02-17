// This mod WILL break when geometry dash updates as it modifies a memory address
// I completely understand if this mod gets rejected because of that

#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>

const char* DEFAULT_DEATH_SOUND = "explode_11.ogg";
const std::set<ghc::filesystem::path> SUPPORTED_FILE_EXTENSIONS = {
	".ogg",
	".mp3",
	".wav",
};

#ifdef GEODE_IS_WINDOWS
constexpr uintptr_t DEATH_SOUND_OFFSET = 0x2E6B87;
#else
#error Undefined death sound offset for platform.
#endif

using namespace geode::prelude;

char* deathSound = new char[4096];

ghc::filesystem::path getFolder() {
	return Mod::get()->getSaveDir()/"deathsounds";
}

void updateSound() {
	log::info("Updating death sound.");

	if (!Mod::get()->getSettingValue<bool>("enabled")) {
		strcpy(deathSound, DEFAULT_DEATH_SOUND);
		return;
	}

	std::vector<std::string> filePool;
	for (const auto& file : ghc::filesystem::directory_iterator(getFolder())) {
		if (SUPPORTED_FILE_EXTENSIONS.find(file.path().extension()) != SUPPORTED_FILE_EXTENSIONS.end()) {
			filePool.push_back(file.path().string());
		}
	}
	
	if (!filePool.empty()) {
		const char* randSound = filePool[rand()%filePool.size()].c_str();
		log::info("Randomly chosen sound: {}", randSound);
		strcpy(deathSound, randSound);
	} else {
		log::info("Using default sound because folder is empty or doesn't have any valid sounds in it.");
		strcpy(deathSound, DEFAULT_DEATH_SOUND);
	}
}

class $modify(PlayerObject) {
	void playerDestroyed(bool p) {
		if (!p) {
			updateSound();
		}

		PlayerObject::playerDestroyed(p);
	}
};

$execute {
	// Create folder if it doesnt exist
	auto folder = getFolder();
	if (!ghc::filesystem::exists(folder)) {
		ghc::filesystem::create_directory(folder);
	}

	srand(time(NULL)); // Seed rng
	
	// Make push instructions push our deathSound var instead of gd "explode_11.ogg" string
	log::info("Patching death sound");
	if (Mod::get()->patch(
		reinterpret_cast<void*>(geode::base::get() + DEATH_SOUND_OFFSET),
		geode::toByteArray(deathSound)
	).isErr()) {
		log::error("Failed to patch death sounds");
	}
};
