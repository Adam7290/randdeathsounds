## Death Sound Randomizer Changelog
# v1.1.0
- Moved sounds folder to config dir (pencil icon thing) (your folder will be automatically moved!)
- Sounds are cached and preloaded when loading a level (no more lagspikes when playing a long sound for the first time!)
- Now using std::mt19937 rng instead of rand() (aka less repeated sounds)

# v1.0.0
- Initial release