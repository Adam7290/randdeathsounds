// This is some nasty code because I made the WONDERFUL decision to put the deathsounds folder in saved dir instead of config (pencil button thingy)
// and now that I wanna add android support I need to dumb stuff down

#include "folder_manager.h"

#include <fstream>

#include <Geode/Geode.hpp>
#include <Geode/UI.hpp>
using namespace geode::prelude;

void Folder::setup() {
    auto folder = getFolder();
    auto legacyFolder = getLegacyFolder();

    // FIX MY STUPID FOLDER LOCATION!!! LOL!!!!!!!!!
    if (fs::exists(legacyFolder) && !fs::exists(folder)) {
        std::error_code err;
        fs::copy(legacyFolder, folder, err); // Can't simply fs::rename the folder to a new location in case the new location is on another filesystem
        if (!err) {
            fs::remove_all(legacyFolder);
            log::info("Successfully moved deathsounds folder from {} to {}", legacyFolder, folder);

            auto readmeForSMARTPeople = std::ofstream(legacyFolder.remove_filename()/"where_are_my_deathsounds.txt");
            readmeForSMARTPeople << fmt::format("Click the pencil icon next to the {} settings button!\n", Mod::get()->getName());
            readmeForSMARTPeople << fmt::format("If that didn't work go to {} :)\n", folder);
            readmeForSMARTPeople << "You can safely delete this file.";
            readmeForSMARTPeople.close();
        } 
        else {
            log::error("FAILED to move deathsounds folder: {}", err.message());
        }
    }

    // If there is no deathsounds folder create one
    if (!fs::exists(folder)) {
        fs::create_directory(folder);
        log::info("Creating directory: {}", folder);
    }
}

fs::path Folder::getLegacyFolder() {
    return Mod::get()->getSaveDir()/"deathsounds";
}

fs::path Folder::getFolder() {
    return Mod::get()->getConfigDir()/"deathsounds";
}