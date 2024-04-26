#include "util.h"

#include <sstream>

using geode::ByteVector;

// I love having to overcomplicate things because std::vector just doesn't work????????????????????????????????????
ByteVector Util::patchToBytes(const char* patch) {
    ByteVector bytes;

    // Why does the c++ standard so bloated but so useless
    // WHY IS THERE NOT A STRING::SPLIT??????????????????
    std::istringstream stream(patch);
    std::string byteStr;
    while (std::getline(stream, byteStr, ' ')) {
        bytes.emplace_back(std::stoi(byteStr, 0, 16)); // just let it throw an error I dont fucking care at this point
    }

    return bytes;
}

unsigned long Util::hash2(unsigned long hash1, unsigned long hash2) {
    return 5345567981 * hash1 + hash2;
}