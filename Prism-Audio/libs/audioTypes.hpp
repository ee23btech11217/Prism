#pragma once

namespace Audio
{
    using resID_t = uint32_t;
    using channelID_t = uint32_t;
    using fxID_t = uint8_t;
    using acallback_t = std::function<void(channelID_t)>;
    using audio_pos_t = uint32_t;
    using sample_t = uint16_t;

    using volume_t = float;
    using pan_t = float;
    using speed_t = float;
    using gain_t = float;

    using time_ms = uint32_t;
    using time_t = float;
    

    constexpr uint32_t DEFAULT_SAMPLE_RATE = 22050;
    constexpr uint16_t DEFAULT_BUFFER_SIZE = 128;
    
    constexpr time_ms DEFAULT_FADE_MS = 100;
    
    constexpr size_t MAX_CHANNELS = 32;
    
    constexpr size_t AGC_WINSIZE = 256;
    constexpr float AGC_TARGET_LEVEL = 0.707f;
    constexpr float AGC_ATTACK_TIME = 0.01f;
    constexpr float AGC_RELEASE_TIME = 0.5f;
}