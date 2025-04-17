#include "channel.hpp"
#include "chainFX.hpp"
#include "audioHelper.hpp"
#include "resourceManager.hpp"

int main() {
    // Audio::Manager amang;
    // amang.parsePAB("AudioBank/audio_bank.pab");
    Audio::Manager audioManager;
    audioManager.initialize("AudioBank/audio_bank.pab");
    audioManager.parsePAB();

    Audio::Engine mainEngine;
    

    return 0;
}
