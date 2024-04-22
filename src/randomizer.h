#pragma once

namespace Randomizer {
    void setup();
    bool isEnabled();
    void randomizeSound();
    void loadSounds();
    void playRandomDeathSound();
    bool isValidExtension(const std::string& filePath);
}