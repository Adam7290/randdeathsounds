#include <Geode/Geode.hpp>

#include "folder_manager.h"
#include "patcher.h"
#include "randomizer.h"

$execute {
	// Patch death sound
	Patcher::setup();

	// Create death sounds folder
	Folder::setup();

	// Setup rng and preload sounds and stuff
	Randomizer::setup();
};
