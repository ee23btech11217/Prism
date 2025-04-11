#pragma once

#include <cinttypes>
#include <vector>
#include <iostream>

namespace Audio
{
    struct audioParams
    {
        float volume;
        float pan;
        float playbackSpeed;
    };
    
    struct Buffer
    {
        uint32_t id;
        int32_t* data_L;
        int32_t* data_R;
        uint32_t length;
        uint32_t sampleRate;
        uint16_t idx;
        bool isLoaded;
    };

    class AudioBuffer
    {
        public:
            AudioBuffer(uint32_t bufID);
            ~AudioBuffer();
            friend class Channel;
            Buffer makeBuffer(uint32_t length, uint32_t sampleRate);

            void loadBuffer(const std::string& filePath);
            void resampleBuffer(uint32_t newSampleRate);
            Buffer audioBuffer;
    };

    struct AutomationProfile 
    {
        std::vector<uint32_t> times;
        std::vector<uint32_t> values;
        uint32_t length;
        uint32_t sampleRate;
        uint32_t startSample;
        uint32_t endSample;
    };
}
