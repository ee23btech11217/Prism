#include "channel.hpp"
#include "chainFX.hpp"
#include "audioHelper.hpp"
#include "resourceManager.hpp"
#include "libs/modifiers.hpp"

int main() {
    // Audio::Manager amang;
    // amang.parsePAB("AudioBank/audio_bank.pab");
    Audio::Manager audioManager;
    audioManager.initialize("AudioBank/audio_bank.pab");
    audioManager.parsePAB();

    Audio::AudioBuffer buffer(1);
    Audio::Modifiers modifer;
    modifer.processModifier(0, 0, buffer);
    // Audio::Engine mainEngine;
    

    return 0;
}
