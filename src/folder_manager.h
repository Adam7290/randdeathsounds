#pragma once

#include <ghc/fs_fwd.hpp>
namespace fs = ghc::filesystem;

namespace Folder {
    void setup();
    fs::path getFolder();
    fs::path getLegacyFolder();
    // void showLegacyMoveAlert(bool success);
}